/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 09:08:04 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 12:01:34 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

# include "Config.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Exceptions.hpp"
# include "HTTPParser.hpp"
# include "HTTPSerializer.hpp"
# include "FileHandler.hpp"
# include "CGIHandler.hpp"
# include "HTTPCommon.hpp"
# include <dirent.h>
# include <sys/stat.h>
# include <sys/types.h>

class ResponseBuilder;

class	RequestHandler {

	private:
		std::vector<ServerConfig>	_servers;

		ServerConfig*	_findServerConfig(int port, const std::string &host);
		LocationConfig*	_findLocation(ServerConfig* server, const std::string &uri);
		bool			_isMethodAllowed(LocationConfig* loc, const std::string &method);
		bool			_isBodyComplete(const std::string &rawData, const Request &request);
		bool			_isBodyTooLarge(long bodySize, ServerConfig* server);

		std::string		_buildFilePath(const std::string &uri,
		                               ServerConfig* server, LocationConfig* loc);
		Response		_buildCGIResponse(const CGIResult &result, ResponseBuilder &builder);
		Response		_handleGET(const Request &request, ServerConfig* server, LocationConfig* loc);
		Response		_handlePOST(const Request &request, ServerConfig* server, LocationConfig* loc);
		Response		_handleDELETE(const Request &request, ServerConfig* server, LocationConfig* loc);

	public:
		RequestHandler(const std::vector<ServerConfig>& servers);
		~RequestHandler();

		Response	handleRequest(const Request& request, const std::string &rawData, int port);
};

#endif
