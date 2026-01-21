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
# include <sstream>
# include "Response.hpp"
# include "Config.hpp"
# include "FileHandler.hpp"
# include "HTTPCommon.hpp"
# include "HTTPSerializer.hpp"

/**
 * @class	ResponseBuilder
 * @brief	Construit les réponses HTTP de manière cohérente
 * @details	Sépare la construction des réponses du routage des requêtes
 *			- Gère les réponses de succès (200, 201, 204, etc.)
 *			- Gère les réponses d'erreur avec pages personnalisées
 *			- Charge les pages d'erreur depuis la configuration
 *			- Génère des pages d'erreur par défaut en fallback
 *
 * @note	Builder Pattern : chaque réponse est construite de manière cohérente
 *			Réutilisable partout (RequestHandler, autres handlers, etc.)
 */
class ResponseBuilder {

	private:
		/**
		* @brief	Configuration du serveur (pour charger les pages d'erreur)
		* @details	Peut être NULL (les pages d'erreur seront générées par défaut)
		*/
		ServerConfig*	_server;

		/**
		* @brief	Charge une page d'erreur personnalisée depuis le serveur
		* @param	code Le code HTTP d'erreur (404, 500, etc.)
		* @param	message Le message d'erreur (texte humain)
		* @return	Le contenu HTML de la page d'erreur
		* @details	Logique :
		* 			1. Cherche si le serveur a une page d'erreur personnalisée
		* 			2. Si trouvée et existante, retourne son contenu
		* 			3. Sinon, génère une page d'erreur par défaut
		* @note	Jamais d'exceptions levées, toujours une réponse valide
		*/
		std::string	_loadErrorPage(int code);

	public:
		/**
		* @brief	Constructeur
		* @param	server Pointeur vers la configuration du serveur (peut être NULL)
		* @details	Stocke la référence au serveur pour charger les pages d'erreur
		*/
		ResponseBuilder(ServerConfig* server);

		/**
		* @brief	Destructeur
		* @details	Rien à nettoyer (pas d'allocation dynamique)
		*/
		~ResponseBuilder();

		/**
		* @brief	Construit une réponse HTTP de succès
		* @param	code Le code HTTP (200, 201, 204, etc.)
		* @param	body Le corps de la réponse (contenu HTML, JSON, etc.)
		* @param	mimeType Le type MIME (text/html, application/json, etc.)
		* @return	Une Response HTTP prête à être sérialisée
		*
		* @details	Ajoute automatiquement :
		* 			- Version HTTP/1.1
		* 			- Content-Type : mimeType
		* 			- Content-Length : calculée automatiquement
		* 			- Le body fourni
		*/
		Response		buildSuccess(int code, const std::string &body, const std::string &mimeType);

		/**
		* @brief	Construit une réponse HTTP d'erreur
		* @param	code Le code HTTP d'erreur (404, 500, 413, etc.)
		* @param	message Le message d'erreur (texte humain)
		* @return	Une Response HTTP d'erreur prête à être sérialisée
		*
		* @details	Logique :
		* 			1. Charge la page d'erreur (personnalisée ou par défaut)
		* 			2. Crée une réponse avec le code et le message
		* 			3. Définit le Content-Type à text/html
		* 			4. Ajoute le Content-Length automatiquement
		*
		* @note	Toujours une réponse valide, même si la page d'erreur n'existe pas
		*/
		Response		buildError(int code, const std::string &message);

};

#endif
