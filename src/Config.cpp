/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 07:40:17 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/12 12:13:14 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Config.hpp"

ConfigParser::ConfigParser() : _position(0), _lineNumber(1) {}
ConfigParser::~ConfigParser() {}

void	ConfigParser::_readFile(const std::string &filepath) {
	std::ifstream	infile(filepath.c_str());
	if (!infile.is_open())
		throw ConfigParserE("Can't open configuration file: " + filepath);
	std::string		line;
	while (std::getline(infile, line))
		_fileContent += line + "\n";
	infile.close();
	if (_fileContent.empty())
		throw	ConfigParserE("Configuration file is empty");
}

void	ConfigParser::_skipSpacesAndC() {
	while (_position < _fileContent.length()) {
		char	c = _fileContent[_position];
		if (c == ' ' || c == '\t' || c == '\n') {
			if (c == '\n')
				_lineNumber++;
			_position++;
		}
		else if (c == '#') {
			while (_position < _fileContent.length() && _fileContent[_position] != '\n')
				_position++;
		}
		else
			break ;
	}
}

std::string ConfigParser::_formatErrorMsg(const std::string &msg) {
	std::stringstream ss;
	ss<<"Line "<<_lineNumber<<": "<<msg;
	return ss.str();
}

int	ConfigParser::_stringToInt(const std::string &str) {
	if (str.empty())
		throw ConfigParserE(_formatErrorMsg("Empty string cant be converted to int"));
	for (size_t i = 0; i < str.length(); i++) {
		if (!std::isdigit(str[i]))
			throw ConfigParserE(_formatErrorMsg("Invalid integer : " + str));
	}
	long	value = atol(str.c_str());
	if (value < 0 )
		throw ConfigParserE(_formatErrorMsg("Port number " + str + " out of range"));
	return ((int)value);
}

bool ConfigParser::_stringToBool(const std::string &str) {
	if (str == "on" || str == "true" || str == "yes" || str == "1")
		return true;
	if (str == "off" || str == "false" || str == "no" || str == "0")
		return false;
	throw ConfigParserE(_formatErrorMsg("Invalid boolean value: " + str + ". Use 'on'/'off', 'true'/'false', or 'yes'/'no'"));
}

std::string	ConfigParser::_readToken() {
	_skipSpacesAndC();
	if (_position >= _fileContent.length())
		return ("");
	std::string	token;
	char		c = _fileContent[_position];
	if (c == '{' || c == '}' || c == ';' || c == ':') {
		token += c;
		_position++;
		return (token);
	}
	if (c == '"') {
		_position++;
		while (_position < _fileContent.length() && _fileContent[_position] != '"') {
			token += _fileContent[_position];
			_position++;
		}
		if (_position < _fileContent.length())
			_position++;
		return (token);
	}
	while (_position < _fileContent.length()) {
		char ch = _fileContent[_position];
		if (std::isalnum(ch) || ch == '_' || ch == '.' || ch == '/' || ch == '-' || ch == ':') {
			token += ch;
			_position++;
		} else
			break ;
	}
	return (token);
}

std::string	ConfigParser::_peekToken() {
	size_t	savedPos = _position;
	int		savedLineNumber = _lineNumber;
	std::string	token = _readToken();
	_position = savedPos;
	_lineNumber = savedLineNumber;
	return (token);
}

void	ConfigParser::_parseListenDirective(const std::string &listenStr, std::string &host, int &port) {
	size_t	colonP = listenStr.find(':');
	if (colonP == std::string::npos)
		throw ConfigParserE(_formatErrorMsg("Invalid listen format. Expected 'IP:PORT', got: " + listenStr));
	host = listenStr.substr(0, colonP);
	std::string	portStr = listenStr.substr(colonP + 1);
	if (host.empty())
		throw ConfigParserE(_formatErrorMsg("Listen directive: IP address cant be empty"));
	if (portStr.empty())
		throw ConfigParserE(_formatErrorMsg("Listen directive: port cant be empty"));
	port = _stringToInt(portStr);
}

std::vector<std::string>	ConfigParser::_parseMethodsList() {
	std::vector<std::string>	methods;
	std::string					token;

	while (true) {
		token = _readToken();
		if (token == ";")
			break ;
		if (token.empty())
			throw ConfigParserE(_formatErrorMsg("Unexpected EOF while parsing methods list"));
		if (token == "{" || token == "}")
			throw ConfigParserE(_formatErrorMsg("Unexpected symbol '" + token + "' in methods list"));
		methods.push_back(token);
	}
	if (methods.empty())
		throw ConfigParserE(_formatErrorMsg("Methods list cant be empty"));
	return (methods);
}

void	ConfigParser::_parseServerDirective(const std::string &key, ServerConfig &config) {
	std::string		token;
	if (key == "listen") {
		token = _readToken();
		_parseListenDirective(token, config.host, config.port);
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after listen directive, got: " + token));
	} else if (key == "server_name") {
		while (true) {
			token = _readToken();
			if (token == ";")
				break ;
			if (token.empty())
				throw ConfigParserE(_formatErrorMsg("Unexpected EOF in server_name directive"));
			config.serverNames.push_back(token);
		}
	} else if (key == "max_body_size") {
		token = _readToken();
		config.maxBodySize = _stringToInt(token);
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after max_body_size, got: " + token));
	} else if (key == "root") {
		token = _readToken();
		if (token.empty() || token == ";")
			throw ConfigParserE(_formatErrorMsg("Root directive requires a path"));
		config.root = token;
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after root, got: " + token));
	} else if (key == "error_page") {
		int	error_code = _stringToInt(_readToken());
		std::string	errorPath = _readToken();
		if (errorPath.empty() || errorPath == ";")
			throw ConfigParserE(_formatErrorMsg("Error page directive requires a path"));
		config.errorPages[error_code] = errorPath;
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after error_page, got: " + token));
	} else
		throw ConfigParserE(_formatErrorMsg("Unknown server directive: " + key));
}


void	ConfigParser::_parseLocationDirective(const std::string &key, LocationConfig &location) {
	std::string		token;
	if (key == "allowed_methods") {
		location.allowedMethods = _parseMethodsList();
	} else if (key == "root") {
		token = _readToken();
		if (token.empty() || token == ";")
			throw ConfigParserE(_formatErrorMsg("Root directive requires a path"));
		location.root = token;
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after root, got: " + token));
	} else if (key == "index") {
		token = _readToken();
		if (token.empty() || token == ";")
			throw ConfigParserE(_formatErrorMsg("Index directive requires a filename"));
		location.index = token;
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after index, got: " + token));
	} else if (key == "autoindex") {
		token = _readToken();
		location.autoIndex = _stringToBool(token);
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after autoindex, got: " + token));
	} else if (key == "redirect_url") {
		token = _readToken();
		if (token.empty() || token == ";")
			throw ConfigParserE(_formatErrorMsg("Redirect URL cannot be empty"));
		location.redirectUrl = token;
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after redirect_url, got: " + token));
	} else if (key == "allow_upload") {
		token = _readToken();
		location.allowUpload = _stringToBool(token);
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after allow_upload, got: " + token));
	} else if (key == "upload_store") {
		token = _readToken();
		if (token.empty() || token == ";")
			throw ConfigParserE(_formatErrorMsg("Upload store path cannot be empty"));
		location.uploadStore = token;
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after upload_store, got: " + token));
	} else if (key == "cgi_extension") {
		std::string	extension = _readToken();
		std::string	path = _readToken();
		if (extension.empty() || extension == ";")
			throw ConfigParserE(_formatErrorMsg("CGI extension cannot be empty"));
		if (path.empty() || path == ";")
			throw ConfigParserE(_formatErrorMsg("CGI path cannot be empty"));
		location.cgiHandlers[extension] = path;
		token = _readToken();
		if (token != ";")
			throw ConfigParserE(_formatErrorMsg("Expected ';' after cgi_extension, got: " + token));
	} else
		throw ConfigParserE(_formatErrorMsg("Unknown location directive: " + key));
}

LocationConfig	ConfigParser::_parseLocationBlock() {
	LocationConfig	location;
	std::string		token;
	location.autoIndex = false;
	location.allowUpload = false;
	token = _readToken();
	if (token.empty() || token == "{")
		throw ConfigParserE(_formatErrorMsg("Location requires a path"));
	location.path = token;
	token = _readToken();
	if (token != "{")
		throw ConfigParserE(_formatErrorMsg("Expected '{' after location path, got: " + token));
	while (true) {
		token = _peekToken();
		if (token == "}")
			break ;
		if (token.empty())
			throw ConfigParserE(_formatErrorMsg("Unexpected EOF in location block"));
		token = _readToken();
		_parseLocationDirective(token, location);
	}
	token = _readToken();
	if (token != "}")
		throw ConfigParserE(_formatErrorMsg("Expected '}' to close location block, got: " + token));
	return (location);
}

ServerConfig	ConfigParser::_parseServerBlock() {
	ServerConfig	config;
	std::string		token;

	config.port = 0;
	config.root = "";
	config.maxBodySize = 0;
	token = _readToken();
	if (token != "{")
		throw ConfigParserE(_formatErrorMsg("Expected '{' after 'server', got: " + token));
	while (true) {
		token = _peekToken();
		if (token == "}")
			break ;
		if (token.empty())
			throw ConfigParserE(_formatErrorMsg("Unexpected EOF in server block"));

		token = _readToken();
		if (token == "location") {
			LocationConfig	location = _parseLocationBlock();
			config.locations.push_back(location);
		}
		else {
			_parseServerDirective(token, config);
		}
	}
	token = _readToken();
	if (token != "}")
		throw ConfigParserE(_formatErrorMsg("Expected '}' to close server block, got: " + token));
	return (config);
}

std::vector<ServerConfig>	ConfigParser::parse(const std::string &filepath) {
	std::vector<ServerConfig>	servers;
	std::string					token;

	_readFile(filepath);
	_position = 0;
	_lineNumber = 1;
	while (true) {
		token = _peekToken();
		if (token.empty())
			break ;
		if (token != "server")
			throw ConfigParserE(_formatErrorMsg("Expected 'server' keyword, got: " + token));
		token = _readToken();
		ServerConfig	config = _parseServerBlock();
		servers.push_back(config);
	}
	if (servers.empty())
		throw ConfigParserE(_formatErrorMsg("No server blocks found in configuration"));
	return (servers);
}
