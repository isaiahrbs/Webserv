/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 09:19:21 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/13 12:44:47 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"

RequestHandler::RequestHandler(const std::vector<ServerConfig> &servers) : _servers(servers) {}
RequestHandler::~RequestHandler() {}

ServerConfig*	RequestHandler::_findServerConfig(int port) {
	for (size_t i = 0; i < _servers.size(); i++) {
		if (_servers[i].port == port)
			return (&_servers[i]);
	}
	return (NULL);
}

LocationConfig*	RequestHandler::_findLocation(ServerConfig* server, const std::string &uri) {
	if (!server)
		return (NULL);
	for (size_t i = 0; i < server->locations.size(); i++) {
		if (server->locations[i].path == uri)
			return (&server->locations[i]);
	}
	return (NULL);
}

bool	RequestHandler::_isMethodAllowed(LocationConfig* loc, const std::string &method) {
	if (!loc)
		return (false);
	for (size_t i = 0; i < loc->allowedMethods.size(); i++) {
		if (loc->allowedMethods[i] == method)
			return (true);
	}
	return (false);
}

std::string	RequestHandler::_intToString(long num) {
	std::stringstream ss;
	ss << num;
	return (ss.str());
}

std::string	RequestHandler::_buildFilePath(LocationConfig* loc, const std::string& uri) {
	std::string	filePath = loc->root;
	if (!filePath.empty() && filePath[filePath.length() - 1] != '/')
		filePath += '/';
	std::string	uriPath = uri;
	if (!uri.empty() && uriPath[0] == '/')
		uriPath = uriPath.substr(1);
	filePath += uriPath;
	return (filePath);
}

std::string	RequestHandler::_getFileContent(const std::string &filePath) {
	std::ifstream	file(filePath.c_str(), std::ios::binary);
	if (!file.is_open())
		throw ResponseE("Cant open file: " + filePath);
	std::stringstream	buffer;
	buffer<<file.rdbuf();
	file.close();
	return (buffer.str());
}

std::string	RequestHandler::_getMimeType(const std::string &filePath) {
	size_t	dotPos = filePath.rfind('.');
	if (dotPos == std::string::npos)
		return ("application/octet-stream");
	std::string	extension = filePath.substr(dotPos);
	if (extension == ".html" || extension == ".htm")
		return ("text/html");
	if (extension == ".css")
		return ("text/css");
	if (extension == ".js")
		return ("application/javascript");
	if (extension == ".json")
		return ("application/json");
	if (extension == ".txt")
		return ("text/plain");
	if (extension == ".png")
		return ("image/png");
	if (extension == ".jpg" || extension == ".jpeg")
		return ("image/jpeg");
	if (extension == ".gif")
		return ("image/gif");
	if (extension == ".ico")
		return ("image/x-icon");
	if (extension == ".pdf")
		return ("application/pdf");
	if (extension == ".xml")
		return ("application/xml");
	return ("application/octet-stream");
}

bool	RequestHandler::_fileExists(const std::string &filePath) {
	struct stat buffer;
	return (stat(filePath.c_str(), &buffer) == 0);
}

bool	RequestHandler::_isDirectory(const std::string &filePath) {
	struct stat buffer;
	if (stat(filePath.c_str(), &buffer) != 0)
		return (false);
	return (S_ISDIR(buffer.st_mode));
}

bool	RequestHandler::_deleteFile(const std::string &filePath) {
	return (remove(filePath.c_str()) == 0);
}

bool	RequestHandler::_writeFile(const std::string &filePath, const std::string &content) {
	std::ofstream	file(filePath.c_str(), std::ios::binary);
	if (!file.is_open())
		return (false);
	file.write(content.c_str(), content.length());
	file.close();
	return (true);
}

bool	RequestHandler::_isBodyComplete(const Request &request, long bodySize) {
	std::string contentLengthStr = request.getHeader("Content-Length");
	if (contentLengthStr.empty())
		return (true);
	long expectedLength = atol(contentLengthStr.c_str());
	return (bodySize >= expectedLength);
}

bool	RequestHandler::_isBodyTooLarge(long bodySize, ServerConfig* server) {
	if (!server || server->maxBodySize <= 0)
		return (false);
	return (bodySize > server->maxBodySize);
}

std::string	RequestHandler::_generateAutoindex(const std::string &dirPath, const std::string &uri) {
	DIR*			dir = opendir(dirPath.c_str());
	if (!dir)
		return ("");
	std::stringstream	html;
	html << "<html><head><title>Index of " << uri << "</title></head><body>\n";
	html << "<h1>Index of " << uri << "</h1>\n";
	html << "<table border='1'><tr><th>Name</th><th>Size</th></tr>\n";
	struct dirent	*entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string filename = entry->d_name;
		if (filename == "." || filename == "..")
			continue;
		std::string filepath = dirPath;
		if (!filepath.empty() && filepath[filepath.length() - 1] != '/')
			filepath += '/';
		filepath += filename;
		struct stat statbuf;
		if (stat(filepath.c_str(), &statbuf) == 0) {
			std::string size = (S_ISDIR(statbuf.st_mode)) ? "[DIR]" : _intToString(statbuf.st_size);
			html << "<tr><td><a href='" << uri;
			if (!uri.empty() && uri[uri.length() - 1] != '/')
				html << '/';
			html << filename << "'>" << filename << "</a></td>";
			html << "<td>" << size << "</td></tr>\n";
		}
	}
	html << "</table></body></html>";
	closedir(dir);
	return (html.str());
}

std::string	RequestHandler::_extractFileName(const Request &request) {
	std::string filename = request.getUri();
	size_t slashPos = filename.rfind('/');
	if (slashPos != std::string::npos)
		filename = filename.substr(slashPos + 1);
	if (filename.empty())
		filename = "uploaded_file.bin";
	return (filename);
}

Response	RequestHandler::_handleGET(const Request &request, ServerConfig* server, LocationConfig* loc) {
	std::string	filePath = _buildFilePath(loc, request.getUri());
	if (_isDirectory(filePath)) {
		if (!loc->index.empty()) {
			std::string indexPath = filePath;
			if (!indexPath.empty() && indexPath[indexPath.length() - 1] != '/')
				indexPath += '/';
			indexPath += loc->index;
			if (_fileExists(indexPath)) {
				try {
					std::string content = _getFileContent(indexPath);
					std::string mimeType = _getMimeType(indexPath);
					return (_buildResponse(200, "OK", content, mimeType));
				} catch (const ResponseE &e) {
					return (_errorResponse(404, "Not Found", server));
				}
			}
		}
		if (loc->autoIndex) {
			std::string autoindexHtml = _generateAutoindex(filePath, request.getUri());
			return (_buildResponse(200, "OK", autoindexHtml, "text/html"));
		}
		return (_errorResponse(404, "Not Found", server));
	}
	if (!_fileExists(filePath))
		return (_errorResponse(404, "Not Found", server));
	try {
		std::string content = _getFileContent(filePath);
		std::string mimeType = _getMimeType(filePath);
		return (_buildResponse(200, "OK", content, mimeType));
	} catch (const ResponseE &e) {
		return (_errorResponse(404, "Not Found", server));
	}
}

Response	RequestHandler::_handlePOST(const Request &request, ServerConfig* server, LocationConfig* loc) {
	long bodySize = request.getBody().length();
	if (_isBodyTooLarge(bodySize, server))
		return (_errorResponse(413, "Payload Too Large", server));
	if (!loc->allowUpload)
		return (_errorResponse(405, "Method Not Allowed", server));
	if (loc->uploadStore.empty())
		return (_errorResponse(500, "Internal Server Error", server));
	std::string uploadPath = loc->uploadStore;
	if (!uploadPath.empty() && uploadPath[uploadPath.length() - 1] != '/')
		uploadPath += '/';
	uploadPath += _extractFileName(request);
	if (!_writeFile(uploadPath, request.getBody()))
		return (_errorResponse(500, "Internal Server Error", server));
	std::string response = "<html><body><h1>File uploaded successfully</h1></body></html>";
	return (_buildResponse(200, "OK", response, "text/html"));
}

Response	RequestHandler::_handleDELETE(const Request &request, ServerConfig* server, LocationConfig* loc) {
	std::string	filePath = _buildFilePath(loc, request.getUri());
	if (!_fileExists(filePath))
		return (_errorResponse(404, "Not Found", server));
	if (_isDirectory(filePath))
		return (_errorResponse(403, "Forbidden", server));
	if (!_deleteFile(filePath))
		return (_errorResponse(500, "Internal Server Error", server));
	Response resp;
	resp.setVersion("HTTP/1.1");
	resp.setStatus(204, "No Content");
	resp.setHeader("Content-Length", "0");
	return (resp);
}

Response	RequestHandler::_buildResponse(int code, const std::string &message,
				const std::string &body, const std::string &mimeType) {
		Response	resp;
		resp.setVersion("HTTP/1.1");
		resp.setStatus(code, message);
		resp.setHeader("Content-Type", mimeType);
		std::stringstream	ss;
		ss<<body.length();
		resp.setHeader("Content-Length", ss.str());
		resp.setBody(body);
		return (resp);
}

std::string	RequestHandler::_loadErrorPage(int code, const std::string &message, ServerConfig* server) {
	if (server && server->errorPages.find(code) != server->errorPages.end()) {
		std::string	errorPagePath = server->errorPages[code];
		std::string	fullPath = server->root;
		if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/')
			fullPath += '/';
		fullPath += errorPagePath;
		std::ifstream file(errorPagePath.c_str());
		if (file.good()) {
			file.close();
			try {
				return (_getFileContent(errorPagePath));
			} catch (const ResponseE &e) {
			}
		}
	}
	std::stringstream	ss;
	ss<<"<html><body><h1>"<<code<<" "<<message<<"</h1></body></html>";
	return (ss.str());
}

Response	RequestHandler::_errorResponse(int code, const std::string &message, ServerConfig* server) {
	std::string errorBody = _loadErrorPage(code, message, server);
	return (_buildResponse(code, message, errorBody, "text/html"));
}

Response	RequestHandler::handleRequest(const Request &request, int port) {
	ServerConfig*	server = _findServerConfig(port);
	if (!server)
		return (_errorResponse(500, "Internal Server Error", NULL));
	LocationConfig*	loc = _findLocation(server, request.getUri());
	if (!loc)
		return (_errorResponse(404, "Not Found", server));
	std::string method = request.getMethod();
	if (!_isMethodAllowed(loc, method))
		return (_errorResponse(405, "Method Not Allowed", server));
	if (method == "GET")
		return (_handleGET(request, server, loc));
	else if (method == "POST")
		return (_handlePOST(request, server, loc));
	else if (method == "DELETE")
		return (_handleDELETE(request, server, loc));
	else
		return (_errorResponse(405, "Method Not Allowed", server));
}
