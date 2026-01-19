/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 11:56:03 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 12:57:52 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ResponseBuilder.hpp"

/*	============================================================================
		CONSTRUCTEUR / DESTRUCTEUR
	============================================================================ */

ResponseBuilder::ResponseBuilder(ServerConfig* server) : _server(server) {}
ResponseBuilder::~ResponseBuilder() {}

/*	============================================================================
		HELPER: Charger page d'erreur personnalisée
	============================================================================ */

std::string	ResponseBuilder::_loadErrorPage(int code, const std::string &message) {
	if (!_server)
		return ("");
	if (_server->errorPages.find(code) != _server->errorPages.end()) {
		std::string	errorPagePath = _server->errorPages[code];
		std::string	fullPath = _server->root;
		if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/')
			fullPath += '/';
		fullPath += errorPagePath;
		if (FileHandler::exists(fullPath)) {
			try {
				return (FileHandler::getContent(errorPagePath));
			} catch (...) {
				// Fallback si erreur de lecture
				return ("");
			}
		}
	}
	return ("");
}

/*	============================================================================
		PUBLIC API: Construire réponse de succès
	============================================================================ */

Response	ResponseBuilder::buildSuccess(int code, const std::string &body,
										const std::string &mimeType) {
	Response	resp;
	resp.setVersion("HTTP/1.1");
	resp.setStatus(code, httpStatusCodeToMessage(code));
	resp.setHeader("Content-Type", mimeType);
	resp.setHeader("Content-Length", httpIntToString(body.length()));
	resp.setBody(body);
	return (resp);
}

/*	============================================================================
		PUBLIC API: Construire réponse d'erreur
	============================================================================ */

Response	ResponseBuilder::buildError(int code, const std::string &message) {
	std::string	customBody = _loadErrorPage(code, message);
	if (!customBody.empty())
		return (buildSuccess(code, customBody, "text/html"));
	RawResponse	raw = HTTPSerializer::createErrorResponse(code, message);
	Response	resp;
	resp.setVersion(raw.version);
	resp.setStatus(raw.statusCode, raw.statusMessage);
	for (std::map<std::string, std::string>::const_iterator it = raw.headers.begin();
		it != raw.headers.end(); ++it) {
		resp.setHeader(it->first, it->second);
	}
	resp.setBody(raw.body);
	return (resp);
}
