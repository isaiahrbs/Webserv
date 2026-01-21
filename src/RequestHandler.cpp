/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 09:19:21 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/21 08:22:13 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include "../inc/ResponseBuilder.hpp"

RequestHandler::RequestHandler(const std::vector<ServerConfig> &servers) : _servers(servers) {}
RequestHandler::~RequestHandler() {}

/*	============================================================================
		CONFIGURATION ROUTING
	============================================================================ */

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

	LocationConfig*	best_match = NULL;
	size_t			best_length = 0;
	for (size_t i = 0; i < server->locations.size(); i++) {
		const std::string &loc_path = server->locations[i].path;
		if (uri.substr(0, loc_path.length()) == loc_path) {
			if (loc_path.length() > best_length) {
				best_match = &server->locations[i];
				best_length = loc_path.length();
			}
		}
	}
	return (best_match);
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


bool	RequestHandler::_isBodyComplete(const std::string &rawData, const Request &request) {
	std::string	contentLengthStr = request.getHeader("Content-Length");
	if (contentLengthStr.empty())
		return (true);
	size_t	bodyStart = rawData.find("\r\n\r\n");
	if (bodyStart == std::string::npos)
		bodyStart = rawData.find("\n\n");
	if (bodyStart == std::string::npos)
		return (false);
	bodyStart += (rawData.find("\r\n\r\n") != std::string::npos) ? 4 : 2;
	long	bodySize = rawData.length() - bodyStart;
	long	expectedLength = atol(contentLengthStr.c_str());
	return (bodySize >= expectedLength);
}

bool	RequestHandler::_isBodyTooLarge(long bodySize, ServerConfig* server) {
	if (!server || server->maxBodySize <= 0)
		return (false);
	return (bodySize > server->maxBodySize);
}


/*	============================================================================
		GET HANDLER
	============================================================================ */

Response	RequestHandler::_handleGET(const Request &request, ServerConfig* server, LocationConfig* loc) {
	ResponseBuilder	builder(server);
	std::string		filePath = loc->root;

	if (!filePath.empty() && filePath[filePath.length() - 1] != '/')
		filePath += '/';
	std::string	uriPath = request.getUri();
	if (!uriPath.empty() && uriPath[0] == '/')
		uriPath = uriPath.substr(1);
	filePath += uriPath;
	if (FileHandler::isDirectory(filePath)) {
		if (!loc->index.empty()) {
			std::string	indexPath = filePath;
			if (!indexPath.empty() && indexPath[indexPath.length() - 1] != '/')
				indexPath += '/';
			indexPath += loc->index;
			if (FileHandler::exists(indexPath)) {
				std::string	content = FileHandler::getContent(indexPath);
				std::string	mimeType = httpGetMimeType(indexPath);
				return (builder.buildSuccess(200, content, mimeType));
			}
		}
		if (loc->autoIndex) {
			std::string	autoindexHtml = FileHandler::generateDirectoryListing(filePath, request.getUri());
			return (builder.buildSuccess(200, autoindexHtml, "text/html"));
		}
		return (builder.buildError(403, "Forbidden"));
	}
	if (!FileHandler::exists(filePath))
		return (builder.buildError(404, "Not Found"));
	std::string	content = FileHandler::getContent(filePath);
	std::string	mimeType = httpGetMimeType(filePath);
	return (builder.buildSuccess(200, content, mimeType));
}


/*	============================================================================
		POST HANDLER
	============================================================================ */

Response	RequestHandler::_handlePOST(const Request &request, ServerConfig* server, LocationConfig* loc) {
	ResponseBuilder	builder(server);
	long			bodySize = request.getBody().length();

	if (_isBodyTooLarge(bodySize, server))
		return (builder.buildError(413, "Payload Too Large"));
	if (!loc->allowUpload)
		return (builder.buildError(405, "Method Not Allowed"));
	if (loc->uploadStore.empty())
		return (builder.buildError(500, "Internal Server Error"));
	std::string	uploadPath = loc->uploadStore;
	if (!uploadPath.empty() && uploadPath[uploadPath.length() - 1] != '/')
		uploadPath += '/';
	std::string	filename = FileHandler::extractFileName(request.getUri());
	uploadPath += filename;
	if (!FileHandler::writeContent(uploadPath, request.getBody()))
		return (builder.buildError(500, "Internal Server Error"));
	std::string	response = "<html><body><h1>File uploaded successfully</h1></body></html>";
	return (builder.buildSuccess(201, response, "text/html"));
}

/*	============================================================================
		DELETE HANDLER
	============================================================================ */

Response	RequestHandler::_handleDELETE(const Request &request, ServerConfig* server, LocationConfig* loc) {
	ResponseBuilder	builder(server);
	std::string		filePath = loc->root;

	if (!filePath.empty() && filePath[filePath.length() - 1] != '/')
		filePath += '/';
	std::string	uriPath = request.getUri();
	if (!uriPath.empty() && uriPath[0] == '/')
		uriPath = uriPath.substr(1);
	filePath += uriPath;
	if (!FileHandler::exists(filePath))
		return (builder.buildError(404, "Not Found"));
	if (FileHandler::isDirectory(filePath))
		return (builder.buildError(403, "Forbidden"));
	if (!FileHandler::deleteFile(filePath))
		return (builder.buildError(500, "Internal Server Error"));
	return (builder.buildSuccess(204, "", "text/plain"));
}

/*	============================================================================
		MAIN REQUEST HANDLER
	============================================================================ */

Response	RequestHandler::handleRequest(const Request &request, const std::string &rawData, int port) {
	ResponseBuilder	builder(NULL);
	if (!_isBodyComplete(rawData, request))
		return (builder.buildError(400, "Bad Request"));
	ServerConfig*	server = _findServerConfig(port);
	if (!server)
		return (builder.buildError(500, "Internal Server Error"));
	builder = ResponseBuilder(server);
	LocationConfig*	loc = _findLocation(server, request.getUri());
	if (!loc)
		return (builder.buildError(404, "Not Found"));
	std::string	method = request.getMethod();
	if (!_isMethodAllowed(loc, method))
		return (builder.buildError(405, "Method Not Allowed"));
	if (method == "GET")
		return (_handleGET(request, server, loc));
	else if (method == "POST")
		return (_handlePOST(request, server, loc));
	else if (method == "DELETE")
		return (_handleDELETE(request, server, loc));
	else
		return (builder.buildError(405, "Method Not Allowed"));
}
