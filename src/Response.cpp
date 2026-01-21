/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:10:09 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/21 08:21:00 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() : _version("HTTP/1.1"), _statusCode(0), _headerCount(0) {}
Response::~Response() {}

void	Response::setVersion(const std::string &version) {
	_version = version;
}

void	Response::setStatus(int code, const std::string &message) {
	_statusCode = code;
	_statusMessage = message;
}

void	Response::setHeader(const std::string &key, const std::string &value) {
	if (_headerCount >= MAX_HEADERS)
		return ;
	_headerKeys[_headerCount] = key;
	_headerValues[_headerCount] = value;
	_headerCount++;
}

void	Response::setBody(const std::string &body) {
	_body = body;
}

// ============ CONVERT TO RAW RESPONSE ============
RawResponse	Response::toRaw() const {
	RawResponse	raw;
	raw.version = _version;
	raw.statusCode = _statusCode;
	raw.statusMessage = _statusMessage;
	raw.body = _body;

	// Copy headers from arrays to map
	for (int i = 0; i < _headerCount; i++) {
		raw.headers[_headerKeys[i]] = _headerValues[i];
	}

	return (raw);
}

int	Response::getStatusCode() const {
	return (_statusCode);
}

std::string	Response::getBody() const {
	return (_body);
}
