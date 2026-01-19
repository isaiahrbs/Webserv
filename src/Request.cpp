/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 16:18:25 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 13:03:36 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

// ============ CONSTRUCTEUR/DESTRUCTEUR ============
Request::Request() : _headerCount(0) {}
Request::~Request() {}

// ============ GETTERS ============
std::string	Request::getMethod() const { return (_method); }
std::string	Request::getUri() const { return (_uri); }
std::string	Request::getVersion() const { return (_version); }
std::string	Request::getBody() const { return (_body); }
int			Request::getHeaderCount() const { return (_headerCount); }

std::string Request::getHeader(const std::string &key) const {
	std::string lowerKey = httpToLower(key);
	for (int i = 0; i < _headerCount; i++) {
		if (httpToLower(_headersKeys[i]) == lowerKey)
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

// ============ LOAD FROM RAW REQUEST ============
void	Request::loadFromRaw(const RawRequest &raw) {
	_method = raw.method;
	_uri = raw.uri;
	_version = raw.version;
	_body = raw.body;
	_headerCount = 0;
	for (std::map<std::string, std::string>::const_iterator it = raw.headers.begin();
		it != raw.headers.end() && _headerCount < MAX_HEADERS; ++it) {
		_headersKeys[_headerCount] = it->first;
		_headerValues[_headerCount] = it->second;
		_headerCount++;
	}
}
