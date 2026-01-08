/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 13:06:22 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/08 19:40:42 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <string>
# include <sstream>
# define MAX_HEADERS 50

class	Response {


private:
	std::string	_version;
	std::string	_statusMessage;
	std::string	_headerKeys[MAX_HEADERS];
	std::string	_headerValues[MAX_HEADERS];
	std::string	_body;
	int			_statusCode;
	int			_headerCount;

public:
	Response();
	~Response();

	void	setVersion(const std::string &version);
	void	setStatus(int code, const std::string &message);
	void	setHeader(const std::string &key, const std::string &value);
	void	setBody(const std::string &body);

	std::string	build() const;

};
