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
#include <sstream>
#include <cstdlib>
# include "HTTPCommon.hpp"

/* ============================================================================
RAW RESPONSE STRUCTURE
============================================================================ */

	/**
	* @struct	RawResponse
	* @brief	Structure pour stocker les données d'une réponse HTTP
	* @details	Intermédiaire entre objet Response C++ et sérialisation HTTP brute
	*			Contient toutes les parties : status line, headers, body
	*/
	struct	RawResponse {
		int									statusCode;			// HTTP_OK, HTTP_NOT_FOUND, etc.
		std::string							statusMessage;		// "OK", "Not Found", etc.
		std::string							version;			// "HTTP/1.1", "HTTP/1.0"
		std::map<std::string, std::string>	headers;			// "Content-Type" → "text/html"
		std::string							body;				// Contenu de la réponse
	};

	/* ============================================================================
	HTTP SERIALIZER CLASS
	============================================================================ */

	/**
	* @class	HTTPSerializer
	* @brief	Convertit les réponses C++ en texte HTTP brut RFC 7230
	* @details	Sépare complètement la sérialisation du stockage
	*			- Responsabilité UNIQUE : sérialiser en HTTP brut
	*			- NE touche pas aux objets Response, travaille avec RawResponse
	*			- Gère la mise en forme : \r\n, Content-Length auto, etc.
	*/

	class	HTTPSerializer {

		public:
			/**
			* @brief	Convertit une RawResponse en texte HTTP RFC 7230
			* @param	response La structure RawResponse remplie
			* @return	Texte HTTP complet prêt à envoyer au socket
			* @details	Format de sortie :
			*			HTTP/1.1 200 OK\r\n
			*			Content-Type: text/html\r\n
			*			Content-Length: 42\r\n
			*			\r\n
			*			<html>...</html>
			*/
			static std::string	serializeResponse(const RawResponse &response);

			/**
			* @brief	Crée une RawResponse d'erreur simple
			* @param	code Code d'erreur (HTTP_NOT_FOUND, HTTP_INTERNAL_SERVER_ERROR, etc.)
			* @param	message Message d'erreur (optionnel, utilisé pour le body)
			* @return	RawResponse complète prête à sérialiser
			* @details	Génère du HTML simple :
			*			<html><body>
			*			<h1>404 Not Found</h1>
			*			<p>The requested resource was not found.</p>
			*			</body></html>
			*/
			static RawResponse	createErrorResponse(int code, const std::string &message);


		private:
			// ===== HELPER METHODS =====
			/**
			* @brief	Construit la status line HTTP
			* @return	"HTTP/1.1 200 OK\r\n"
			*/
			static std::string	_buildStatusLine(const RawResponse &response);

			/**
			* @brief	Construit tous les headers avec \r\n
			* @return	"Content-Type: text/html\r\nContent-Length: 42\r\n"
			*/
			static std::string	_buildHeadersBlock(const RawResponse &response);
};

#endif
