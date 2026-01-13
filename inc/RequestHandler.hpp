/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 09:08:04 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/13 12:45:12 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

# include "Config.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Exceptions.hpp"
# include <dirent.h>
# include <sys/stat.h>
#include <sys/types.h>

class	RequestHandler {

	private:
		std::vector<ServerConfig>	_servers;

		ServerConfig*	_findServerConfig(int port);
		LocationConfig*	_findLocation(ServerConfig* server, const std::string &uri);
		bool			_isMethodAllowed(LocationConfig* loc, const std::string &method);

		std::string		_intToString(long num);
		std::string		_buildFilePath(LocationConfig* loc, const std::string &uri);
		std::string		_getFileContent(const std::string &filePath);
		std::string		_getMimeType(const std::string &filePath);
		bool			_fileExists(const std::string &filePath);
		bool			_isDirectory(const std::string &filePath);
		bool			_deleteFile(const std::string &filePath);
		bool			_writeFile(const std::string &filePath, const std::string &content);

		bool			_isBodyComplete(const std::string &rawData, const Request &request);
		bool			_isBodyTooLarge(long bodySize, ServerConfig* server);

		std::string		_generateAutoindex(const std::string &dirPath, const std::string &uri);
		std::string		_extractFileName(const Request &request);

		Response		_handleGET(const Request &request, ServerConfig* server, LocationConfig* loc);
		Response		_handlePOST(const Request &request, ServerConfig* server, LocationConfig* loc);
		Response		_handleDELETE(const Request &request, ServerConfig* server, LocationConfig* loc);

		Response		_buildResponse(int code, const std::string &message,
										const std::string &body, const std::string &mimeType);
		std::string		_loadErrorPage(int code, const std::string &message, ServerConfig* server);
		Response		_errorResponse(int code, const std::string &message, ServerConfig* server);

	public:
		RequestHandler(const std::vector<ServerConfig>& servers);
		~RequestHandler();

		Response	handleRequest(const Request& request, const std::string &rawData, int port);

};

#endif
