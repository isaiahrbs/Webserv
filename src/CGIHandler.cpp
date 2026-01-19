/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 10:48:14 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 12:55:36 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/CGIHandler.hpp"

/*	============================================================================
		CGI DETECTION
	============================================================================ */

bool	CGIHandler::isCGI(const std::string &filePath,
						const std::map<std::string, std::string> &handlers) {
	std::string	extension = FileHandler::getFileExtension(filePath);
	return (handlers.find(extension) != handlers.end());
}

std::string	CGIHandler::getCGIInterpreter(const std::string &filePath,
											const std::map<std::string, std::string> &handlers) {
	std::string	extension = FileHandler::getFileExtension(filePath);
	std::map<std::string, std::string>::const_iterator	it = handlers.find(extension);

	if (it == handlers.end())
		return ("");
	return (it->second);
}

/*	============================================================================
		ENVIRONMENT BUILDING (RFC 3875)
	============================================================================ */

std::string	CGIHandler::_getPathInfo(const std::string &scriptPath, const std::string &uri) {
	size_t	script_pos = uri.find(scriptPath);
	if (script_pos == std::string::npos)
		return ("");
	size_t	path_start = script_pos + scriptPath.length();
	if (path_start >= uri.length())
		return ("");
	return (uri.substr(path_start));
}
std::map<std::string, std::string>	CGIHandler::_buildCGIEnvironment(const Request &request,
			const std::string &scriptPath, const ServerConfig &server) {
	std::map<std::string, std::string>	env;
	int	methodCode = httpStringToMethod(request.getMethod());
	std::string normalizedMethod;
	if (methodCode != HTTP_METHOD_UNKNOWN)
		normalizedMethod = httpMethodToString(methodCode);
	else
		normalizedMethod = request.getMethod();
	env["REQUEST_METHOD"] = normalizedMethod;
	env["SCRIPT_NAME"] = scriptPath;
	env["SCRIPT_FILENAME"] = scriptPath;
	env["SERVER_NAME"] = server.serverName;
	env["SERVER_PORT"] = httpIntToString(server.port);
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_SOFTWARE"] = "WebServ/1.0";
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	std::string	uri = request.getUri();
	size_t		query_pos = uri.find('?');
	if (query_pos != std::string::npos) {
		env["QUERY_STRING"] = uri.substr(query_pos + 1);
	} else {
		env["QUERY_STRING"] = "";
	}
	std::string	content_length = request.getHeader("content-length");
	if (!content_length.empty())
		env["CONTENT_LENGTH"] = content_length;
	std::string	content_type = request.getHeader("content-type");
	if (!content_type.empty())
		env["CONTENT_TYPE"] = content_type;
	env["PATH_INFO"] = _getPathInfo(scriptPath, uri);
	for (int i = 0; i < request.getHeaderCount(); i++) {
		std::string key = request.getHeaderKey(i);
		std::string value = request.getHeaderValue(i);
		std::string env_key = "HTTP_";
		for (size_t j = 0; j < key.length(); j++) {
			if (key[j] == '-')
				env_key += '_';
			else if (key[j] >= 'a' && key[j] <= 'z')
				env_key += (key[j] - 32);
			else
				env_key += key[j];
		}
		env[env_key] = value;
	}
	return (env);
}

std::vector<char*>	CGIHandler::_mapToCharArray(const std::map<std::string, std::string> &env) {
	std::vector<char*>	result;

	for (std::map<std::string, std::string>::const_iterator it = env.begin();
		it != env.end(); ++it) {
		std::string env_str = it->first + "=" + it->second;
		char *env_var = new char[env_str.length() + 1];
		std::strcpy(env_var, env_str.c_str());
		result.push_back(env_var);
	}
	result.push_back(NULL);
	return (result);
}

/*	============================================================================
		CGI EXECUTION (fork/execve/pipe)
	============================================================================ */

CGIResult	CGIHandler::execute(const std::string &scriptPath, const Request &request,
					const ServerConfig &server, const std::map<std::string, std::string> &handlers) {

	CGIResult	result;
	result.exitCode = -1;
	result.success = false;
	result.output = "";

	if (!isCGI(scriptPath, handlers))
		return (result);
	std::string interpreter = getCGIInterpreter(scriptPath, handlers);
	if (interpreter.empty())
		return (result);
	int pipe_in[2];
	int pipe_out[2];
	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
		return (result);

	std::map<std::string, std::string> cgi_env = _buildCGIEnvironment(request, scriptPath, server);
	std::vector<char*> env_array = _mapToCharArray(cgi_env);
	pid_t pid = fork();
	if (pid == -1) {
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		for (size_t i = 0; i < env_array.size() - 1; i++)
			delete[] env_array[i];
		return (result);
	}
	if (pid == 0) {
		close(pipe_in[1]);
		close(pipe_out[0]);
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_out[1]);
		char *argv[] = {
			(char *)interpreter.c_str(),
			(char *)scriptPath.c_str(),
			NULL
		};
		execve(interpreter.c_str(), argv, env_array.data());
		exit(127);
	}
	close(pipe_in[0]);
	close(pipe_out[1]);
	std::string body = request.getBody();
	if (!body.empty()) {
		write(pipe_in[1], body.c_str(), body.length());
	}
	close(pipe_in[1]);
	char buffer[4096];
	ssize_t bytes;
	while ((bytes = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
		result.output.append(buffer, bytes);
	}
	close(pipe_out[0]);
	int status;
	waitpid(pid, &status, 0);
	if (WIFEXITED(status)) {
		result.exitCode = WEXITSTATUS(status);
		result.success = (result.exitCode == 0);
	}
	for (size_t i = 0; i < env_array.size() - 1; i++)
		delete[] env_array[i];
	return (result);
}
