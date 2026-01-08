/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:10:09 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/08 19:56:00 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() : _version("HTTP/1.1"), _statusCode(0), _headercount(0) {}
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

std::string	Response::build() const {
	std::stringstream	response;
	response<<_version<<" "<<_statusCode<<" "<<_statusMessage<<"\r\n";
	for (int i = 0; i < _headerCount; i++) {
		response << _headerKeys[i] << ": " << _headerValues[i] << "\r\n";
	}
	response<<"\r\n";
	response<<_body;
	return (response.str());
}
