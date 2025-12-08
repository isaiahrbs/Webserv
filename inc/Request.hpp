/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*    Request.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:49:52 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/03 13:11:16 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

#define MAX_HEADERS 50

class	Request {
private:
	std::string	_method;
	std::string	_uri;
	std::string	_version;
	std::string	_headersKeys[MAX_HEADERS];
	std::string	_headerValues[MAX_HEADERS];
	int			_headerCount;
	std::string	_body;

public:
	Request();
	~Request();

	bool	parse(const std::string &rawRequest);
	std::string	getMethod() const;
	std::string	getUri() const;
	std::string	getVersion() const;
	std::string	getHeader(const std::string &key) const;
	std::string	getBody() const;
};
