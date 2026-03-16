/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 09:36:07 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/21 13:06:49 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <exception>
#include <string>
#include <sstream>

// Exception levée lors du parsing du fichier de configuration
class	ConfigParserE : public std::exception {
	private:
		std::string			_message;
	public:
		ConfigParserE(const std::string &msg);
		virtual const char*	what() const throw();
		virtual ~ConfigParserE() throw();
};

// Exception levée lors du parsing d'une requête HTTP invalide
class	RequestE : public std::exception {
	private:
		std::string			_message;
	public:
		RequestE(const std::string &msg);
		virtual const char*	what() const throw();
		virtual ~RequestE() throw();
};
