/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 08:23:00 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 13:04:09 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
# define HTTPPARSER_HPP

# include <string>
# include <map>
# include <cstdlib>
# include "HTTPCommon.hpp"
# include "Exceptions.hpp"

struct	RawRequest {
	std::string							method;
	std::string							uri;
	std::string							version;
	std::map<std::string, std::string>	headers;
	std::string							body;
	int									headerCount;
};

class	HTTPParser {

	public:
		static RawRequest	parseRequest(const std::string &rawData);
		static bool			isRequestComplete(const std::string &rawData,
									const std::map<std::string, std::string> &headers);
		static size_t		findBodyStart(const std::string &rawData);

	private:
		static std::string	_trim(const std::string &str);
		static std::string	_split(const std::string &str, char delimiter, size_t &pos);
		static void			_parseRequestLine(const std::string &line, RawRequest &req);
		static void			_parseHeaders(const std::string &headerBlock, RawRequest &req);
		static std::string	_unchunkBody(const std::string &chunked);
};

#endif
