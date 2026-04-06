/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPCommon.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 08:14:10 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/21 08:18:54 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPCOMMON_HPP
# define HTTPCOMMON_HPP

# include <string>
# include <map>
# include <cstdlib>
# include <vector>
class RequestHandler;
# include "Config.hpp"

/*	============================================================================
	HTTP STATUS CODES
	============================================================================ */

# define HTTP_OK					200
# define HTTP_CREATED				201
# define HTTP_ACCEPTED				202
# define HTTP_NO_CONTENT			204
# define HTTP_MOVED_PERMANENTLY		301
# define HTTP_FOUND					302
# define HTTP_NOT_MODIFIED			304
# define HTTP_BAD_REQUEST			400
# define HTTP_UNAUTHORIZED			401
# define HTTP_FORBIDDEN				403
# define HTTP_NOT_FOUND				404
# define HTTP_METHOD_NOT_ALLOWED	405
# define HTTP_CONFLICT				409
# define HTTP_PAYLOAD_TOO_LARGE		413
# define HTTP_INTERNAL_SERVER_ERROR	500
# define HTTP_NOT_IMPLEMENTED		501
# define HTTP_SERVICE_UNAVAILABLE	503

/*	============================================================================
	HTTP METHODS
	============================================================================ */

# define HTTP_METHOD_GET		0
# define HTTP_METHOD_POST		1
# define HTTP_METHOD_PUT		2
# define HTTP_METHOD_DELETE		3
# define HTTP_METHOD_HEAD		4
# define HTTP_METHOD_OPTIONS	5
# define HTTP_METHOD_PATCH		6
# define HTTP_METHOD_UNKNOWN	-1

/*	============================================================================
	HTTP HELPER FUNCTIONS
	============================================================================ */

	int				httpStringToMethod(const std::string &method);
	std::string		httpMethodToString(int method);
	std::string		httpIntToString(long num);
	std::string		httpToLower(const std::string &str);
	std::string		httpStatusCodeToMessage(int code);
	bool			httpIsValidVersion(const std::string &version);
	bool			httpIsValidMethod(const std::string &method);
	std::string		httpGetMimeType(const std::string &filename);
	void			httpInitMimeTypes(void);

/*	============================================================================
	HTTP SERVER ENGINE
	============================================================================ */

	class HTTPServerEngine {

		private:
			RequestHandler*	_handler;

		public:
			HTTPServerEngine(const std::vector<ServerConfig> &servers);
			~HTTPServerEngine();
			std::string processRequest(const std::string &rawData, int clientPort);
	};

#endif
