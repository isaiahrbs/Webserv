/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 09:41:58 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 13:04:39 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPParser.hpp"

/*	============================================================================
		STRING HELPERS (private)
	============================================================================ */

std::string	HTTPParser::_trim(const std::string &str) {
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

std::string	HTTPParser::_split(const std::string &str, char delimiter, size_t &pos) {
	size_t	start = pos;
	size_t	end = str.find(delimiter, start);
	if (end == std::string::npos) {
		pos = str.length();
		return (str.substr(start));
	}
	pos = end + 1;
	return (str.substr(start, end - start));
}

/*	============================================================================
		REQUEST LINE PARSING (GET /path HTTP/1.1)
	============================================================================ */

void	HTTPParser::_parseRequestLine(const std::string &line, RawRequest &req) {
	size_t	pos = 0;
	std::string	trimmed = _trim(line);
	if (trimmed.empty())
		throw RequestE("Request line is empty");
	req.method = _split(trimmed, ' ', pos);
	int	methodCode = httpStringToMethod(req.method);
	if (methodCode == HTTP_METHOD_UNKNOWN)
		throw RequestE("Invalid HTTP method: " + req.method);
	req.uri = _split(trimmed, ' ', pos);
	if (req.uri.empty())
		throw RequestE("URI is missing");
	req.version = _split(trimmed, ' ', pos);
	if (!httpIsValidVersion(req.version))
		throw RequestE("Invalid HTTP version: " + req.version);
}

/*	============================================================================
		HEADERS PARSING (Host: localhost\r\n...)
	============================================================================ */

void	HTTPParser::_parseHeaders(const std::string &headerBlock, RawRequest &req) {
	size_t	pos = 0;
	std::string	line;
	std::string	key;
	std::string	value;
	size_t	colon_pos;
	req.headerCount = 0;
	while (pos < headerBlock.length()) {
		size_t	line_end = headerBlock.find("\r\n", pos);
		if (line_end == std::string::npos)
			line_end = headerBlock.length();
		line = headerBlock.substr(pos, line_end - pos);
		pos = line_end + 2;
		if (line.empty())
			break ;
		colon_pos = line.find(':');
		if (colon_pos == std::string::npos)
			throw RequestE("Invalid header format (missing colon): " + line);
		key = _trim(line.substr(0, colon_pos));
		value = _trim(line.substr(colon_pos + 1));
		if (key.empty())
			throw RequestE("Empty header key");
		key = httpToLower(key);
		req.headers[key] = value;
		req.headerCount++;
	}
}

/*	============================================================================
		PUBLIC API: FIND BODY START (\r\n\r\n separator)
	============================================================================ */

size_t	HTTPParser::findBodyStart(const std::string &rawData) {
	size_t	pos = rawData.find("\r\n\r\n");
	if (pos == std::string::npos)
		return (std::string::npos);
	return (pos + 4);
}

/*	============================================================================
		PUBLIC API: PARSE REQUEST (main entry point)
	============================================================================ */

RawRequest	HTTPParser::parseRequest(const std::string &rawData) {
	RawRequest	req;
	size_t		body_start;
	size_t		first_line_end;
	std::string	request_line;
	std::string	headers_block;
	if (rawData.empty())
		throw RequestE("Raw request data is empty");
	first_line_end = rawData.find("\r\n");
	if (first_line_end == std::string::npos)
		throw RequestE("Request line not properly terminated");
	request_line = rawData.substr(0, first_line_end);
	_parseRequestLine(request_line, req);
	body_start = findBodyStart(rawData);
	if (body_start == std::string::npos) {
		headers_block = rawData.substr(first_line_end + 2);
		req.body = "";
	} else {
		headers_block = rawData.substr(first_line_end + 2, body_start - first_line_end - 6);
		req.body = rawData.substr(body_start);
	}
	_parseHeaders(headers_block, req);
	return (req);
}

/*	============================================================================
		PUBLIC API: CHECK REQUEST COMPLETE (headers + body)
	============================================================================ */

bool	HTTPParser::isRequestComplete(const std::string &rawData,
										const std::map<std::string, std::string> &headers) {
	size_t	body_start;
	long	content_length;
	std::map<std::string, std::string>::const_iterator	it;
	body_start = findBodyStart(rawData);
	if (body_start == std::string::npos)
		return (false);
	it = headers.find("content-length");
	if (it == headers.end())
		return (true);
	content_length = atol(it->second.c_str());
	if (content_length < 0)
		return (false);
	size_t	body_size = rawData.length() - body_start;
	return (body_size >= (size_t)content_length);
}
