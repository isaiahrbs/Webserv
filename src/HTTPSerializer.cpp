/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPSerializer.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 09:55:35 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 10:14:15 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPSerializer.hpp"

/*	============================================================================
		HELPER: Build status line (HTTP/1.1 200 OK\r\n)
	============================================================================ */

std::string	HTTPSerializer::_buildStatusLine(const RawResponse &response) {
	std::string	status_line;
	status_line = response.version;
	status_line += " ";
	status_line += httpIntToString(response.statusCode);
	status_line += " ";
	status_line += response.statusMessage;
	status_line += "\r\n";
	return (status_line);
}

/*	============================================================================
		HELPER: Build headers block (Key: Value\r\n...)
	============================================================================ */

std::string	HTTPSerializer::_buildHeadersBlock(const RawResponse &response) {
	std::string	headers_block;
	std::map<std::string, std::string>::const_iterator	it;

	for (it = response.headers.begin(); it != response.headers.end(); ++it) {
		headers_block += it->first;
		headers_block += ": ";
		headers_block += it->second;
		headers_block += "\r\n";
	}
	if (!response.body.empty()) {
		headers_block += "Content-Length: ";
		headers_block += httpIntToString(response.body.length());
		headers_block += "\r\n";
	}
	return (headers_block);
}

/*	============================================================================
		PUBLIC API: Serialize response to HTTP text
	============================================================================ */

std::string	HTTPSerializer::serializeResponse(const RawResponse &response) {
	std::string	http_response;
	http_response += _buildStatusLine(response);
	http_response += _buildHeadersBlock(response);
	http_response += "\r\n";
	http_response += response.body;
	return (http_response);
}

/*	============================================================================
		PUBLIC API: Create error response with HTML body
	============================================================================ */

RawResponse	HTTPSerializer::createErrorResponse(int code, const std::string &message) {
	RawResponse	response;
	std::string	html_body;
	response.statusCode = code;
	response.statusMessage = httpStatusCodeToMessage(code);
	response.version = "HTTP/1.1";
	html_body = "<html>\r\n";
	html_body += "<head>\r\n";
	html_body += "<title>";
	html_body += httpIntToString(code);
	html_body += " ";
	html_body += response.statusMessage;
	html_body += "</title>\r\n";
	html_body += "</head>\r\n";
	html_body += "<body>\r\n";
	html_body += "<h1>";
	html_body += httpIntToString(code);
	html_body += " ";
	html_body += response.statusMessage;
	html_body += "</h1>\r\n";
	html_body += "<p>";
	html_body += message;
	html_body += "</p>\r\n";
	html_body += "</body>\r\n";
	html_body += "</html>\r\n";
	response.body = html_body;
	response.headers["Content-Type"] = "text/html";
	response.headers["Connection"] = "close";
	return (response);
}
