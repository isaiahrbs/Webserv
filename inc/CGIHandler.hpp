/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 08:31:25 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 12:15:25 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <string>
# include <map>
# include <vector>
# include <unistd.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <signal.h>
# include <cstring>
# include "Config.hpp"
# include "Request.hpp"
# include "FileHandler.hpp"
# include "HTTPCommon.hpp"

struct	CGIResult {
	int			exitCode;
	std::string	output;
	bool		success;
};

class	CGIHandler {

	public:
		static bool			isCGI(const std::string &filePath,
								const std::map<std::string, std::string> &handlers);
		static std::string	getCGIInterpreter(const std::string &filePath,
											const std::map<std::string, std::string> &handlers);
		static CGIResult	execute(const std::string &scriptPath, const Request &request,
								const ServerConfig &server,
								const std::map<std::string, std::string> &handlers);

	private:
		static std::map<std::string, std::string>
				_buildCGIEnvironment(const Request &request, const std::string &scriptPath,
									const ServerConfig &server);
		static std::vector<char*>
				_mapToCharArray(const std::map<std::string, std::string> &env);
		static std::string	_getPathInfo(const std::string &scriptPath, const std::string &uri);
};

#endif
