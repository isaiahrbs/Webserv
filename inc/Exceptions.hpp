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

/**
 * @brief	Exception levee lors du parsing de la configuration
 * @details	Cette exception est levée quand il y a une erreur dans le fichier de
 * 		configuration, comme une syntaxe invalide, un fichier manquant, ou une directive inconnue
*/
class	ConfigParserE : public std::exception {
	private:
		std::string			_message;
	public:
		ConfigParserE(const std::string &msg);
		virtual const char*	what() const throw();
		virtual ~ConfigParserE() throw();
};

/**
 * @brief	Exception levée lors du parsing d'une requête HTTP
 * @details	Cette exception est levée quand une requête HTTP n'est pas valide,
 * 		par exemple si la première ligne n'est pas au bon format.
*/
class	RequestE : public std::exception {
	private:
		std::string			_message;
	public:
		RequestE(const std::string &msg);
		virtual const char*	what() const throw();
		virtual ~RequestE() throw();
};

/**
 * @brief	Exception levée lors de la construction d'une réponse HTTP
 * @details	Cette exception est levée quand il y a un problème lors de la
 * 		construction ou de la sérialisation d'une réponse HTTP.
*/
class	ResponseE : public std::exception {
	private:
		std::string		_message;
	public:
		ResponseE(const std::string &msg);
		virtual const char*	what() const throw();
		virtual ~ResponseE() throw();
};
