/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 11:51:30 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/21 08:34:12 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSEBUILDER_HPP
# define RESPONSEBUILDER_HPP

# include <string>
# include "Response.hpp"
# include "Config.hpp"
# include "FileHandler.hpp"
# include "HTTPCommon.hpp"
# include "HTTPSerializer.hpp"

class ResponseBuilder {

	private:
		ServerConfig*	_server;
		std::string		_loadErrorPage(int code);

	public:
		ResponseBuilder(ServerConfig* server);
		~ResponseBuilder();

		Response	buildSuccess(int code, const std::string &body, const std::string &mimeType);
		Response	buildError(int code, const std::string &message);
};

#endif
