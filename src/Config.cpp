/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 07:40:17 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/21 12:02:49 by dinguyen         ###   ########.fr       */
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
	long	value = std::atol(str.c_str());
	if (value < 0 || value > 65535)
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

void	ConfigParser::_parseListenDirective(const std::string &listenStr, str::string &host, int &port) {
	size_t	colonP = listenStr.find(':');
	if (colonP == str::string::npos)
		throw ConfigParserE(_formatErrorMsg("Invalid listen format. Expected 'IP:PORT', got: " + listenStr));
	host = listenStr.substr(0, colonP);
	std::string	portStr = listenStr.substr(colonP + 1);
	if (host.empty())
		throw ConfigParserE(_formatErrorMsg("Listen directive: IP address cant be empty"));
	if (portStr.empty())
		throw ConfigParserE(_formaErrorMsg("Listen directive: port cant be empty"));
	port = _stringToInt(portStr);
}
