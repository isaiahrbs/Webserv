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

/* ============================================================================
RAW REQUEST STRUCTURE
============================================================================ */

	/**
	* @struct	RawRequest
	* @brief	Structure pour stocker les données brutes parsées d'une requête HTTP
	* @details	Intermédiaire entre parsing HTTP brut et objet Request C++
	*			Utilisée pour valider AVANT de créer un objet Request
	*/
	struct	RawRequest {
		std::string							method;			// "GET", "POST", etc.
		std::string							uri;			// "/index.html", "/api?id=42"
		std::string							version;		// "HTTP/1.1", "HTTP/1.0"
		std::map<std::string, std::string>	headers;		// "Host" → "localhost:8080"
		std::string							body;			// Contenu POST/PUT
		int									headerCount;	// Nombre de headers parsés
	};

	/* ============================================================================
	HTTP PARSER CLASS
	============================================================================ */

	/**
	* @class	HTTPParser
	* @brief	Parse les données brutes HTTP en structures RawRequest
	* @details	Sépare complètement le parsing du stockage
	*			- Responsabilité UNIQUE : parser HTTP brut
	*			- NE crée pas d'objets Request, juste des RawRequest
	*			- Valide la syntaxe HTTP RFC 7230
	*
	* @note		C++98 compatible, pas de static members dans namespace
	*/

	class	HTTPParser {

		public:
			/**
			* @brief	Parse une requête HTTP brute en RawRequest
			* @param	rawData Les données brutes reçues du socket
			* @return	RawRequest avec tous les champs remplis
			* @throw 	RequestE si parsing échoue ou format invalide
			* @details	Format attendu (RFC 7230) :
			*			GET /path HTTP/1.1\r\n
			*			Host: localhost\r\n
			*			\r\n
			*			[optionnel body]
			*/
			static RawRequest	parseRequest(const std::string &rawData);

			/**
			* @brief	Vérifie qu'une requête HTTP est complète (headers + body)
			* @param	rawData Les données brutes du socket
			* @param	headers Map des headers (pour Content-Length)
			* @return	true si headers + body sont complètement reçus
			* @details	Logic :
			*			- Si pas de Content-Length → requête complète
			*			- Si Content-Length → vérifie bodySize >= Content-Length
			*/
			static bool			isRequestComplete(const std::string &rawData,
										const std::map<std::string, std::string> &headers);

			/**
			* @brief	Extrait la séparation headers/body
			* @param	rawData Les données brutes
			* @return	Position du début du body (\r\n\r\n trouvé)
			* @details	Utilisé internalement pour parser
			*/
			static size_t		findBodyStart(const std::string &rawData);

		private:
			// ===== PARSING HELPERS =====
			static std::string	_trim(const std::string &str);
			static std::string	_split(const std::string &str, char delimiter, size_t &pos);
			static void			_parseRequestLine(const std::string &line, RawRequest &req);
			static void			_parseHeaders(const std::string &headerBlock, RawRequest &req);
};

#endif
