/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 16:18:25 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/21 18:49:08 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

// ============ CONSTRUCTEUR/DESTRUCTEUR ============
Request::Request() : _headerCount(0) {}
Request::~Request() {}

// ============ HELPERS STATIQUES ============
std::string	Request::_split(const std::string &str, char delimiter, size_t &pos) {
	size_t	start = pos;
	size_t	end = str.find(delimiter, start);
	if (end == std::string::npos) {
		pos = str.length();
		return (str.substr(start));
	}
	pos = end + 1;
	return (str.substr(start, end - start));
}

std::string	Request::_trim(const std::string &str) {
	size_t	start = 0;
	size_t	end = str.length();
	while (start < end && (str[start] == ' ' || str[start] == '\t'
		|| str[start] == '\r' || str[start] == '\n'))
		start++;
	while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t'
		|| str[end - 1] == '\r' || str[end - 1] == '\n'))
		end--;
	return (str.substr(start, end - start));
}

std::string	Request::_toLower(std::string str) {
	for (size_t i = 0; i < str.length(); i++)
		str[i] = std::tolower(str[i]);
	return (str);
}

// ============ VALIDATIONS ============
void	Request::_validateMethod(const std::string &method) {
	const std::string	methods[] = {"GET", "POST", "DELETE", "PUT", "HEAD", "OPTIONS"};
	for (int i = 0; i < 6; i++) {
		if (method == methods[i])
			return ;
	}
	throw RequestE("Invalid HTTP method: " + method);
}

void	Request::_validateVersion(const std::string &version) {
	if (version != "HTTP/1.0" && version != "HTTP/1.1")
		throw RequestE("Unsupported HTTP version: " + version);
}

// ============ GETTERS ============
std::string	Request::getMethod() const { return (_method); }
std::string	Request::getUri() const { return (_uri); }
std::string	Request::getVersion() const { return (_version); }
std::string	Request::getBody() const { return (_body); }
int			Request::getHeaderCount() const { return (_headerCount); }

std::string Request::getHeader(const std::string &key) const {
	std::string lowerKey = _toLower(key);
	for (int i = 0; i < _headerCount; i++) {
		if (_toLower(_headersKeys[i]) == lowerKey)
			return (_headerValues[i]);
	}
	return ("");
}

std::string Request::getHeaderKey(int index) const {
	if (index < 0 || index >= _headerCount)
		return ("");
	return (_headersKeys[index]);
}

std::string Request::getHeaderValue(int index) const {
	if (index < 0 || index >= _headerCount)
		return ("");
	return (_headerValues[index]);
}

// ============ UTILITAIRES ============
void	Request::reset() {
	_method = "";
	_uri = "";
	_version = "";
	_headerCount = 0;
	_body = "";
	for (int i = 0; i < MAX_HEADERS; i++) {
		_headersKeys[i] = "";
		_headerValues[i] = "";
	}
}

void	Request::debug() const {
	std::cout<<"=== REQUEST DEBUG ==="<<std::endl;
	std::cout<<"Method: "<<_method<<std::endl;
	std::cout<<"URI: "<<_uri<<std::endl;
	std::cout<<"Version: "<<_version<<std::endl;
	std::cout<<"\nHeaders ("<<_headerCount<<"):"<<std::endl;
	for (int i = 0; i < _headerCount; i++) {
		std::cout<<"  "<<_headersKeys[i]<<": "<<_headerValues[i]<<std::endl;
	}
	std::cout<<"\nBody:"<<std::endl;
	std::cout<<_body<<std::endl;
	std::cout<<"==================="<<std::endl;
}

// ============ PARSERS ============

void	Request::_parseRequestLine(const std::string &line) {
	std::string	method;
	std::string	uri;
	std::string	version;
	size_t		pos = 0;
	method = _trim(_split(line, ' ', pos));
	uri = _trim(_split(line, ' ', pos));
	version = _trim(_split(line, ' ', pos));
	if (method.empty() || uri.empty() || version.empty())
		throw RequestE("Invalid request line format");
	_validateMethod(method);
	_validateVersion(version);
	_method = method;
	_uri = uri;
	_version = version;
}

void	Request::_parseHeaders(const std::string &headersPortion) {
	size_t	pos = 0;
	std::string	line;
	while (pos < headersPortion.length()) {
		line = _split(headersPortion, '\n', pos);
		line = _trim(line);
		if (line.empty())
			break;
		size_t	colonPos = line.find(':');
		if (colonPos == std::string::npos)
			throw RequestE("Invalid header format: missing colon");
		std::string	key = _trim(line.substr(0, colonPos));
		std::string	value = _trim(line.substr(colonPos + 1));
		if (key.empty())
			throw RequestE("Invalid header format: empty key");
		if (_headerCount >= MAX_HEADERS)
			throw RequestE("Too many headers (max 50)");
		_headersKeys[_headerCount] = key;
		_headerValues[_headerCount] = value;
		_headerCount++;
	}
}

void	Request::_extractBody(const std::string &rawRequest) {
	size_t	delimPos = rawRequest.find("\r\n\r\n");
	if (delimPos == std::string::npos)
		delimPos = rawRequest.find("\n\n");

	if (delimPos != std::string::npos) {
		if (rawRequest.find("\r\n\r\n") != std::string::npos)
			_body = rawRequest.substr(delimPos + 4);
		else
			_body = rawRequest.substr(delimPos + 2);
	}
}

bool	Request::parse(const std::string &rawRequest) {
	if (rawRequest.empty())
		throw RequestE("Empty request");
	reset();
	size_t	headersEndPos = rawRequest.find("\r\n\r\n");
	size_t	headersEndPosUnix = rawRequest.find("\n\n");
	if (headersEndPos == std::string::npos && headersEndPosUnix == std::string::npos)
		throw RequestE("Invalid request: missing header/body delimiter");
	size_t	endPos = (headersEndPos != std::string::npos) ? headersEndPos : headersEndPosUnix;
	std::string	headersPortion = rawRequest.substr(0, endPos);
	size_t	firstNewline = headersPortion.find('\n');
	if (firstNewline == std::string::npos)
		throw RequestE("Invalid request: missing request line");
	std::string	requestLine = _trim(headersPortion.substr(0, firstNewline));
	std::string	headersOnly = headersPortion.substr(firstNewline + 1);
	_parseRequestLine(requestLine);
	_parseHeaders(headersOnly);
	_extractBody(rawRequest);
	return (true);
}
