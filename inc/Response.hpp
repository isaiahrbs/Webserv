/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 13:06:22 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 12:41:22 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <string>
# include <sstream>
# include "HTTPSerializer.hpp"
# define MAX_HEADERS 50

class	Response {

	private:
		//		ATTRIBUTS

		/**
		* @brief	Version du protocole HTTP
		* @details	Format : "HTTP/1.1" ou "HTTP/1.0"
		* 			Défaut : "HTTP/1.1"
		* 			Envoyé dans la première ligne de réponse
		* 			Exemple : "HTTP/1.1"
		*/
		std::string	_version;

		/**
		* @brief	Message de statut associé au code
		* @details	Exemples : "OK", "Not Found", "Internal Server Error"
		* 			N'a pas d'impact technique, c'est un message pour l'humain
		* 			Envoyé après le code dans la première ligne
		* 			Format complet : "HTTP/1.1 200 OK"
		*/
		std::string	_statusMessage;

		/**
		* @brief	Tableau des clés d'en-têtes
		* @details	Parallèle à _headerValues
		* 			Exemples : "Content-Type", "Content-Length", "Location", "Server"
		* 			Peut contenir jusqu'à MAX_HEADERS (50) clés
		* 			Ordre d'ajout préservé
		*/
		std::string	_headerKeys[MAX_HEADERS];

		/**
		* @brief	Tableau des valeurs d'en-têtes
		* @details	Parallèle à _headerKeys
		* 			Exemples : "text/html", "42", "http://example.com", "WebServ/1.0"
		* 			Peut contenir jusqu'à MAX_HEADERS (50) valeurs
		* 			Ordre d'ajout préservé
		*/
		std::string	_headerValues[MAX_HEADERS];

		/**
		* @brief	Corps de la réponse HTTP
		* @details	Contient : fichiers HTML, JSON, XML, images binaires, etc.
		* 			Peut être vide (ex: réponses HEAD, 204 No Content, 304 Not Modified)
		* 			Doit avoir une taille correspondant à l'en-tête Content-Length si défini
		* 			Exemple : "<html><body>Hello!</body></html>"
		*/
		std::string	_body;

		/**
		* @brief	Code de statut HTTP
		* @details	Entier de 100 à 599
		* 			Catégories:
		* 			- 1xx (100-199) : Information
		* 			- 2xx (200-299) : Succès
		* 			- 3xx (300-399) : Redirection
		* 			- 4xx (400-499) : Erreur client
		* 			- 5xx (500-599) : Erreur serveur
		* 			Défaut : 0 (invalide jusqu'à setStatus())
		* 			Exemple : 200, 404, 500
		*/
		int			_statusCode;

		/**
		* @brief	Nombre d'en-têtes actuellement stockés
		* @details	Incrémenté par setHeader()
		* 			Vérifié pour ne pas dépasser MAX_HEADERS (50)
		* 			Initialisation : 0
		* 			Utilisé pour limiter l'ajout de nouveaux headers
		*/
		int			_headerCount;

	public:

		/**
		* @brief	Constructeur par défaut
		* @details	Initialise une réponse HTTP vide :
		* 			- _version = "HTTP/1.1"
		* 			- _statusCode = 0 (invalide)
		* 			- _statusMessage = ""
		* 			- _headerCount = 0
		* 			- _body = ""
		*
		* 			Avant d'utiliser build(), tu dois appeler :
		* 			- setStatus(code, message) obligatoire
		* 			- setVersion() optionnel (par défaut HTTP/1.1)
		* 			- setHeader(key, value) optionnel, peut être appelé plusieurs fois
		* 			- setBody() optionnel
		*
		* 		Exemple :
		* 		@code
		* 		Response resp;  // Constructeur appelé ici
		* 		resp.setStatus(200, "OK");
		* 		@endcode
		*/
		Response();
		/**
		* @brief	Destructeur
		* @details	Libère la mémoire utilisée par les champs
		* 			Les std::string sont automatiquement nettoyées
		*/
		~Response();

		/**
		* @brief	Définit la version HTTP de la réponse
		* @details	Généralement "HTTP/1.1" ou "HTTP/1.0"
		* 			Sera inséré dans la première ligne de la réponse
		* 			Format complet : "<version> <statusCode> <statusMessage>"
		* 			Exemple : "HTTP/1.1"
		*
		* @param	version La version HTTP (ex: "HTTP/1.1", "HTTP/1.0")
		*
		* @note	Valeur par défaut si non appelé : "HTTP/1.1"
		* @note	Peut être changé à tout moment avant build()
		* @note	Standard actuel : toujours utiliser "HTTP/1.1"
		*
		* 	Exemple :
		* 	@code
		* 	Response resp;
		* 	resp.setVersion("HTTP/1.1");  // Standard moderne
		* 	resp.setVersion("HTTP/1.0");  // Ancien
		* 	@endcode
		*/
		void			setVersion(const std::string &version);

		/**
		* @brief	Définit le code de statut et le message associé
		* @details	Compose la première ligne de la réponse :
		* 			"HTTP/1.1 CODE MESSAGE"
		* 			Exemple : "HTTP/1.1 200 OK"
		*
		* 			Codes de statut principaux :
		* 			- 200 OK : Succès (fichier trouvé, opération valide)
		* 			- 201 Created : Ressource créée
		* 			- 204 No Content : Succès mais pas de contenu
		* 			- 301 Moved Permanently : Redirection permanente
		* 			- 302 Found : Redirection temporaire
		* 			- 304 Not Modified : Ressource non modifiée (cache)
		* 			- 400 Bad Request : Requête malformée
		* 			- 403 Forbidden : Accès refusé
		* 			- 404 Not Found : Ressource inexistante
		* 			- 405 Method Not Allowed : Méthode non autorisée (POST sur fichier statique)
		* 			- 413 Payload Too Large : Body trop volumineux
		* 			- 500 Internal Server Error : Erreur interne du serveur
		* 			- 503 Service Unavailable : Service indisponible
		*
		* @param	code Le code de statut HTTP (100-599)
		* @param	message Le message associé (ex: "OK", "Not Found", "Internal Server Error")
		*
		* @note	OBLIGATOIRE : doit être appelé avant build()
		* @note	Peut être appelé plusieurs fois (remplace la valeur précédente)
		* @note	Le code et le message doivent correspondre (ex: 200 → "OK", pas "Error")
		*
		* 	Exemple :
		* 	@code
		* 	Response resp;
		* 	resp.setStatus(200, "OK");
		* 	resp.setStatus(404, "Not Found");
		* 	resp.setStatus(413, "Payload Too Large");
		* 	resp.setStatus(500, "Internal Server Error");
		* 	@endcode
		*/
		void			setStatus(int code, const std::string &message);

		/**
		* @brief	Ajoute un en-tête à la réponse
		* @details	Les en-têtes doivent être au format "Key: Value"
		* 			Seront envoyés avant le body dans la réponse HTTP
		*
		* 			En-têtes courants :
		* 			- Content-Type : Type MIME (text/html, application/json, image/png, etc.)
		* 			- Content-Length : Taille du body en octets (TRÈS IMPORTANT)
		* 			- Location : URL de redirection (pour 301, 302, 303)
		* 			- Server : Nom du serveur (ex: "WebServ/1.0")
		* 			- Date : Date/heure actuelle (RFC 1123) (optionnel)
		* 			- Cache-Control : Directives de cache (public, private, max-age=3600)
		* 			- Connection : "keep-alive" ou "close"
		* 			- Last-Modified : Date de dernière modification (pour cache)
		* 			- ETag : Identifiant unique de la ressource (pour cache)
		*
		* @param	key La clé de l'en-tête (ex: "Content-Type")
		* @param	value La valeur de l'en-tête (ex: "text/html")
		*
		* @note	Peut être appelé plusieurs fois pour ajouter plusieurs en-têtes
		* 			(jusqu'à MAX_HEADERS = 50)
		* @note	Les en-têtes sont conservés dans l'ordre d'ajout
		* @note	Si on atteint MAX_HEADERS, les nouveaux appels sont ignorés silencieusement
		* @note	Les espaces dans key et value sont conservés
		*
		* 	Exemple :
		* 	@code
		* 	Response resp;
		* 	resp.setHeader("Content-Type", "text/html");
		* 	resp.setHeader("Content-Length", "42");
		* 	resp.setHeader("Server", "WebServ/1.0");
		* 	resp.setHeader("Cache-Control", "public, max-age=3600");
		* 	@endcode
		*/
		void			setHeader(const std::string &key, const std::string &value);

		/**
		* @brief	Définit le corps de la réponse
		* @details	Contient les données à envoyer au client :
		* 			- Fichiers HTML/CSS/JS
		* 			- Réponses JSON/XML
		* 			- Fichiers binaires (images, PDF, etc.)
		* 			- Peut être vide
		*
		* 			Important : La taille du body DOIT correspondre à
		* 			l'en-tête Content-Length si celui-ci est défini.
		*
		* 			Exemple : body.length() == stoi(resp.getHeader("Content-Length"))
		*
		* @param	body Le contenu du body (ex: "<html>...</html>", "{...}", ou "")
		*
		* @note	Peut être appelé une seule fois (remplace le body précédent)
		* @note	Peut être vide ("") pour certains codes (204, 304, 404, 500, etc.)
		* @note	Les caractères binaires sont supportés
		* @note	Pas de limite de taille imposée (à gérer via maxBodySize du serveur)
		*
		* 	Exemple :
		* 	@code
		* 	Response resp;
		* 	resp.setBody("<html><body>Hello!</body></html>");
		* 	resp.setBody("{\"status\": \"success\"}");  // Remplace le précédent
		* 	resp.setBody("");  // Body vide
		* 	@endcode
		*/
		void			setBody(const std::string &body);

		RawResponse		toRaw() const;
		int				getStatusCode() const;
		std::string		getBody() const;

};
