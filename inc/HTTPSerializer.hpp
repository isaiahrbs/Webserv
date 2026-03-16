/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPSerializer.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 08:27:20 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 10:42:24 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERIALIZER_HPP
# define HTTPSERIALIZER_HPP

# include <string>
# include <map>
# include <cstdlib>
# include "HTTPCommon.hpp"

struct	RawResponse {
	int									statusCode;
	std::string							statusMessage;
	std::string							version;
	std::map<std::string, std::string>	headers;
	std::string							body;
};

class	HTTPSerializer {

	public:
		static std::string	serializeResponse(const RawResponse &response);
		static RawResponse	createErrorResponse(int code, const std::string &message);

	private:
		static std::string	_buildStatusLine(const RawResponse &response);
		static std::string	_buildHeadersBlock(const RawResponse &response);
};

#endif
