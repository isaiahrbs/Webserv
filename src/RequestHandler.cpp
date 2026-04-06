/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 09:19:21 by dinguyen          #+#    #+#             */
/*   Updated: 2026/04/06 10:48:03 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include "../inc/ResponseBuilder.hpp"

RequestHandler::RequestHandler(const std::vector<ServerConfig> &servers) : _servers(servers) {}
RequestHandler::~RequestHandler() {}

/*	============================================================================
	CONFIGURATION ROUTING
	============================================================================ */

ServerConfig*	RequestHandler::_findServerConfig(int port, const std::string &host) {
	ServerConfig*	fallback = NULL;

	for (size_t i = 0; i < _servers.size(); i++) {
		if (_servers[i].port != port)
			continue;
		if (fallback == NULL)
			fallback = &_servers[i];
		for (size_t j = 0; j < _servers[i].serverNames.size(); j++) {
			if (_servers[i].serverNames[j] == host)
				return (&_servers[i]);
		}
	}
	return (fallback);
}

LocationConfig*	RequestHandler::_findLocation(ServerConfig* server, const std::string &uri) {
	if (!server)
		return (NULL);
	std::string uriPath = uri;
	size_t qPos = uriPath.find('?');
	if (qPos != std::string::npos)
		uriPath = uriPath.substr(0, qPos);
	LocationConfig*	best_match = NULL;
	size_t			best_length = 0;
	for (size_t i = 0; i < server->locations.size(); i++) {
		const std::string &loc_path = server->locations[i].path;
		if (uriPath.substr(0, loc_path.length()) == loc_path) {
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
		return (false);
	bodyStart += 4;
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
	HELPER: Construit le chemin fichier (alias-style comme décrit dans le sujet)
	Exemple sujet : URL /kapouet rooté à /tmp/www → /kapouet/foo → /tmp/www/foo
	============================================================================ */

std::string	RequestHandler::_buildFilePath(const std::string &uri,
                                            ServerConfig* server, LocationConfig* loc) {
	std::string root = (loc->root.empty() && server) ? server->root : loc->root;
	std::string path = uri;
	size_t qPos = path.find('?');
	if (qPos != std::string::npos)
		path = path.substr(0, qPos);
	if (!loc->path.empty() && path.length() >= loc->path.length()
	    && path.substr(0, loc->path.length()) == loc->path) {
		path = path.substr(loc->path.length());
	}
	if (!path.empty() && path[0] == '/')
		path = path.substr(1);
	std::string filePath = root;
	if (!filePath.empty() && filePath[filePath.length() - 1] != '/')
		filePath += '/';
	filePath += path;
	std::string normalized = FileHandler::normalizePath(filePath);
	if (normalized.empty())
		return ("");
	return (normalized);
}

/*	============================================================================
	HELPER: Construit une Response à partir de la sortie CGI
	Parse les éventuels headers CGI (Status:, Content-Type:) dans stdout
	============================================================================ */

Response	RequestHandler::_buildCGIResponse(const CGIResult &result,
                                               ResponseBuilder &builder) {
	if (!result.success && result.exitCode != 0)
		return (builder.buildError(502, "Bad Gateway"));
	std::string output = result.output;
	size_t headerEnd = output.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		headerEnd = output.find("\n\n");
	if (headerEnd == std::string::npos)
		return (builder.buildSuccess(200, output, "text/html"));
	std::string cgiHeaders = output.substr(0, headerEnd);
	size_t      bodyStart  = headerEnd + ((output[headerEnd] == '\r') ? 4 : 2);
	std::string body       = output.substr(bodyStart);
	int         statusCode  = 200;
	std::string contentType = "text/html";
	size_t pos = 0;
	while (pos < cgiHeaders.size()) {
		size_t lineEnd = cgiHeaders.find('\n', pos);
		if (lineEnd == std::string::npos)
			lineEnd = cgiHeaders.size();
		std::string line = cgiHeaders.substr(pos, lineEnd - pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line = line.substr(0, line.size() - 1);
		pos = lineEnd + 1;
		if (line.empty())
			continue;
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
			continue;
		std::string key   = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		size_t valStart = value.find_first_not_of(" \t");
		if (valStart != std::string::npos)
			value = value.substr(valStart);
		for (size_t i = 0; i < key.size(); i++) {
			if (key[i] >= 'A' && key[i] <= 'Z')
				key[i] += 32;
		}
		if (key == "status")
			statusCode = atoi(value.c_str());
		else if (key == "content-type")
			contentType = value;
	}
	return (builder.buildSuccess(statusCode, body, contentType));
}

/*	============================================================================
	GET HANDLER — fichiers statiques + CGI
	============================================================================ */

Response	RequestHandler::_handleGET(const Request &request, ServerConfig* server,
                                       LocationConfig* loc) {
	ResponseBuilder	builder(server);
	std::string     filePath = _buildFilePath(request.getUri(), server, loc);
	if (filePath.empty())
		return (builder.buildError(403, "Forbidden"));
	if (FileHandler::isDirectory(filePath)) {
		if (!loc->index.empty()) {
			std::string indexPath = filePath;
			if (!indexPath.empty() && indexPath[indexPath.length() - 1] != '/')
				indexPath += '/';
			indexPath += loc->index;
			if (FileHandler::exists(indexPath)) {
				if (!loc->cgiHandlers.empty()
				    && CGIHandler::isCGI(indexPath, loc->cgiHandlers)) {
					CGIResult result = CGIHandler::execute(
						indexPath, request, *server, loc->cgiHandlers);
					return (_buildCGIResponse(result, builder));
				}
				std::string content  = FileHandler::getContent(indexPath);
				std::string mimeType = httpGetMimeType(indexPath);
				return (builder.buildSuccess(200, content, mimeType));
			}
		}
		if (loc->autoIndex) {
			std::string listing = FileHandler::generateDirectoryListing(
				filePath, request.getUri());
			return (builder.buildSuccess(200, listing, "text/html"));
		}
		return (builder.buildError(403, "Forbidden"));
	}
	if (!FileHandler::exists(filePath))
		return (builder.buildError(404, "Not Found"));
	if (!loc->cgiHandlers.empty() && CGIHandler::isCGI(filePath, loc->cgiHandlers)) {
		CGIResult result = CGIHandler::execute(
			filePath, request, *server, loc->cgiHandlers);
		return (_buildCGIResponse(result, builder));
	}
	std::string content  = FileHandler::getContent(filePath);
	std::string mimeType = httpGetMimeType(filePath);
	return (builder.buildSuccess(200, content, mimeType));
}

/*	============================================================================
	POST HANDLER — CGI d'abord, puis upload de fichier
	============================================================================ */

Response	RequestHandler::_handlePOST(const Request &request, ServerConfig* server,
                                        LocationConfig* loc) {
	ResponseBuilder	builder(server);
	long            bodySize = (long)request.getBody().length();
	if (_isBodyTooLarge(bodySize, server))
		return (builder.buildError(413, "Payload Too Large"));
	if (!loc->cgiHandlers.empty()) {
		std::string filePath = _buildFilePath(request.getUri(), server, loc);
		if (CGIHandler::isCGI(filePath, loc->cgiHandlers)
		    && FileHandler::exists(filePath)) {
			CGIResult result = CGIHandler::execute(
				filePath, request, *server, loc->cgiHandlers);
			return (_buildCGIResponse(result, builder));
		}
	}
	if (!loc->allowUpload)
		return (builder.buildError(405, "Method Not Allowed"));
	if (loc->uploadStore.empty())
		return (builder.buildError(500, "Internal Server Error"));
	std::string uploadPath = loc->uploadStore;
	if (!uploadPath.empty() && uploadPath[uploadPath.length() - 1] != '/')
		uploadPath += '/';
	std::string filename = FileHandler::extractFileName(request.getUri());
	if (filename.empty())
		filename = "upload";
	uploadPath += filename;
	if (!FileHandler::writeContent(uploadPath, request.getBody()))
		return (builder.buildError(500, "Internal Server Error"));
	std::string resp = "<html><body><h1>File uploaded successfully</h1></body></html>";
	return (builder.buildSuccess(201, resp, "text/html"));
}

/*	============================================================================
	DELETE HANDLER
	============================================================================ */

Response	RequestHandler::_handleDELETE(const Request &request, ServerConfig* server,
                                          LocationConfig* loc) {
	ResponseBuilder	builder(server);
	std::string     filePath = _buildFilePath(request.getUri(), server, loc);
	if (filePath.empty())
		return (builder.buildError(403, "Forbidden"));
	if (!FileHandler::exists(filePath))
		return (builder.buildError(404, "Not Found"));
	if (FileHandler::isDirectory(filePath))
		return (builder.buildError(403, "Forbidden"));
	if (!FileHandler::deleteFile(filePath))
		return (builder.buildError(500, "Internal Server Error"));
	return (builder.buildSuccess(204, "", "text/plain"));
}

/*	============================================================================
	POINT D'ENTRÉE PRINCIPAL
	============================================================================ */

Response	RequestHandler::handleRequest(const Request &request,
                                          const std::string &rawData, int port) {
	ResponseBuilder	builder(NULL);
	if (!_isBodyComplete(rawData, request))
		return (builder.buildError(400, "Bad Request"));
	std::string hostHeader = request.getHeader("host");
	size_t colonPos = hostHeader.find(':');
	if (colonPos != std::string::npos)
		hostHeader = hostHeader.substr(0, colonPos);
	ServerConfig* server = _findServerConfig(port, hostHeader);
	if (!server)
		return (builder.buildError(500, "Internal Server Error"));
	builder = ResponseBuilder(server);
	LocationConfig* loc = _findLocation(server, request.getUri());
	if (!loc)
		return (builder.buildError(404, "Not Found"));
	if (!loc->redirectUrl.empty()) {
		Response resp;
		resp.setVersion("HTTP/1.1");
		resp.setStatus(301, "Moved Permanently");
		resp.setHeader("Location", loc->redirectUrl);
		resp.setHeader("Content-Type", "text/html");
		resp.setHeader("Content-Length", "0");
		resp.setBody("");
		return (resp);
	}
	std::string method = request.getMethod();
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
