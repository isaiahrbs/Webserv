/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPCommon.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 09:01:17 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 14:26:44 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HTTPCommon.hpp"
#include "RequestHandler.hpp"
#include "Config.hpp"

/*	============================================================================
		MIME TYPES MAP (static, private to this file)
	============================================================================ */

static std::map<std::string, std::string>	g_mimeTypes;
static bool									g_mimeTypesInit = false;

/*	============================================================================
		MIME TYPES INITIALIZATION
	============================================================================ */

void	httpInitMimeTypes(void) {
	if (g_mimeTypesInit)
		return ;
	g_mimeTypes[".html"] = "text/html";
	g_mimeTypes[".htm"] = "text/html";
	g_mimeTypes[".txt"] = "text/plain";
	g_mimeTypes[".css"] = "text/css";
	g_mimeTypes[".js"] = "application/javascript";
	g_mimeTypes[".json"] = "application/json";
	g_mimeTypes[".xml"] = "application/xml";

	g_mimeTypes[".png"] = "image/png";
	g_mimeTypes[".jpg"] = "image/jpeg";
	g_mimeTypes[".jpeg"] = "image/jpeg";
	g_mimeTypes[".gif"] = "image/gif";
	g_mimeTypes[".svg"] = "image/svg+xml";
	g_mimeTypes[".ico"] = "image/x-icon";
	g_mimeTypes[".webp"] = "image/webp";

	g_mimeTypes[".pdf"] = "application/pdf";
	g_mimeTypes[".doc"] = "application/msword";
	g_mimeTypes[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";

	g_mimeTypes[".zip"] = "application/zip";
	g_mimeTypes[".tar"] = "application/x-tar";
	g_mimeTypes[".gz"] = "application/gzip";

	g_mimeTypes[".mp3"] = "audio/mpeg";
	g_mimeTypes[".mp4"] = "video/mp4";
	g_mimeTypes[".wav"] = "audio/wav";
	g_mimeTypes[".webm"] = "video/webm";

	g_mimeTypes[".py"] = "text/x-python";
	g_mimeTypes[".sh"] = "application/x-sh";
	g_mimeTypes[".php"] = "application/x-php";
	g_mimeTypes[".exe"] = "application/octet-stream";

	g_mimeTypesInit = true;
}

/*	============================================================================
		HTTP METHOD CONVERSIONS
	============================================================================ */

int		httpStringToMethod(const std::string &method) {
	if (method == "GET")
		return (HTTP_METHOD_GET);
	if (method == "POST")
		return (HTTP_METHOD_POST);
	if (method == "PUT")
		return (HTTP_METHOD_PUT);
	if (method == "DELETE")
		return (HTTP_METHOD_DELETE);
	if (method == "HEAD")
		return (HTTP_METHOD_HEAD);
	if (method == "OPTIONS")
		return (HTTP_METHOD_OPTIONS);
	if (method == "PATCH")
		return (HTTP_METHOD_PATCH);
	return (HTTP_METHOD_UNKNOWN);
}

std::string		httpMethodToString(int method) {
	switch (method) {
		case HTTP_METHOD_GET:
			return ("GET");
		case HTTP_METHOD_POST:
			return ("POST");
		case HTTP_METHOD_PUT:
			return ("PUT");
		case HTTP_METHOD_DELETE:
			return ("DELETE");
		case HTTP_METHOD_HEAD:
			return ("HEAD");
		case HTTP_METHOD_OPTIONS:
			return ("OPTIONS");
		case HTTP_METHOD_PATCH:
			return ("PATCH");
		default:
			return ("UNKNOWN");
	}
}

std::string		httpIntToString(long num) {
	if (num == 0)
		return ("0");
	bool negative = (num < 0);
	if (negative)
		num = -num;
	std::string result = "";
	while (num > 0) {
		result = char('0' + (num % 10)) + result;
		num /= 10;
	}
	if (negative)
		result = "-" + result;
	return (result);
}

std::string	httpToLower(const std::string &str) {
	std::string	result = str;
	for (size_t i = 0; i < result.length(); i++) {
		if (result[i] >= 'A' && result[i] <= 'Z')
			result[i] = result[i] + 32;
	}
	return (result);
}

/*	============================================================================
		HTTP STATUS CODE MESSAGES
	============================================================================ */

std::string		httpStatusCodeToMessage(int code) {
	switch (code) {
		case HTTP_OK:
			return ("OK");
		case HTTP_CREATED:
			return ("Created");
		case HTTP_ACCEPTED:
			return ("Accepted");
		case HTTP_NO_CONTENT:
			return ("No Content");

		case HTTP_MOVED_PERMANENTLY:
			return ("Moved Permanently");
		case HTTP_FOUND:
			return ("Found");
		case HTTP_NOT_MODIFIED:
			return ("Not Modified");

		case HTTP_BAD_REQUEST:
			return ("Bad Request");
		case HTTP_UNAUTHORIZED:
			return ("Unauthorized");
		case HTTP_FORBIDDEN:
			return ("Forbidden");
		case HTTP_NOT_FOUND:
			return ("Not Found");
		case HTTP_METHOD_NOT_ALLOWED:
			return ("Method Not Allowed");
		case HTTP_CONFLICT:
			return ("Conflict");
		case HTTP_PAYLOAD_TOO_LARGE:
			return ("Payload Too Large");

		case HTTP_INTERNAL_SERVER_ERROR:
			return ("Internal Server Error");
		case HTTP_NOT_IMPLEMENTED:
			return ("Not Implemented");
		case HTTP_SERVICE_UNAVAILABLE:
			return ("Service Unavailable");

		default:
			return ("Unknown");
	}
}

/*	============================================================================
		HTTP VERSION VALIDATION
	============================================================================ */

bool		httpIsValidVersion(const std::string &version) {
	return (version == "HTTP/1.0" || version == "HTTP/1.1");
}

/*	============================================================================
		HTTP METHOD VALIDATION
	============================================================================ */

bool		httpIsValidMethod(const std::string &method) {
	return (method == "GET" || method == "POST" || method == "PUT"
		|| method == "DELETE" || method == "HEAD" || method == "OPTIONS"
		|| method == "PATCH");
}

/*	============================================================================
		MIME TYPE RETRIEVAL
	============================================================================ */

std::string	httpGetMimeType(const std::string &filename) {
	size_t			dot_pos;
	std::string		extension;

	if (!g_mimeTypesInit)
		httpInitMimeTypes();
	dot_pos = filename.find_last_of('.');
	if (dot_pos == std::string::npos)
		return ("application/octet-stream");
	extension = filename.substr(dot_pos);
	for (size_t i = 0; i < extension.length(); i++) {
		if (extension[i] >= 'A' && extension[i] <= 'Z')
			extension[i] = extension[i] + 32;
	}
	if (g_mimeTypes.find(extension) != g_mimeTypes.end())
		return (g_mimeTypes[extension]);
	return ("application/octet-stream");
}

HTTPServerEngine::HTTPServerEngine(const std::vector<ServerConfig> &servers) {
	_handler = new RequestHandler(servers);
}

HTTPServerEngine::~HTTPServerEngine() {
	delete _handler;
}

std::string	HTTPServerEngine::processRequest(const std::string &rawData, int clientPort) {
	try {
		RawRequest raw = HTTPParser::parseRequest(rawData);
		Request req;
		req.loadFromRaw(raw);
		Response resp = _handler->handleRequest(req, rawData, clientPort);
		RawResponse raw_resp = resp.toRaw();
		std::string http_response = HTTPSerializer::serializeResponse(raw_resp);
		return (http_response);
	}
	catch (const RequestE &e) {
		RawResponse error = HTTPSerializer::createErrorResponse(400, "Bad Request");
		return HTTPSerializer::serializeResponse(error);
	}
	catch (const std::exception &e) {
		RawResponse error = HTTPSerializer::createErrorResponse(500, "Internal Server Error");
		return HTTPSerializer::serializeResponse(error);
	}
}
