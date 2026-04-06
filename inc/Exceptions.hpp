/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 09:36:07 by dinguyen          #+#    #+#             */
/*   Updated: 2026/04/06 10:48:48 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <exception>
#include <string>
#include <sstream>

class	ConfigParserE : public std::exception {
	private:
		std::string			_message;
	public:
		ConfigParserE(const std::string &msg);
		virtual const char*	what() const throw();
		virtual ~ConfigParserE() throw();
};

class	RequestE : public std::exception {
	private:
		std::string			_message;
	public:
		RequestE(const std::string &msg);
		virtual const char*	what() const throw();
		virtual ~RequestE() throw();
};
