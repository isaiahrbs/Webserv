/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 09:08:04 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/18 10:27:51 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

# include "Config.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Exceptions.hpp"
# include <dirent.h>
# include <sys/stat.h>
#include <sys/types.h>

/**
 * @class	RequestHandler
 * @brief	Traite les requêtes HTTP et génère les réponses correspondantes
 * @details	Cette classe est responsable de :
 * 			- Router les requêtes vers la bonne configuration de serveur
 * 			- Appliquer les règles de la location correspondante
 * 			- Exécuter l'action appropriée (GET, POST, DELETE)
 * 			- Gérer les erreurs HTTP (404, 405, 413, 500, etc.)
 * 			- Générer les réponses HTTP valides
 *
 * 		Flux de traitement d'une requête :
 * 		1. handleRequest() reçoit la requête et le port du serveur
 * 		2. _findServerConfig() cherche la configuration du serveur
 * 		3. _findLocation() cherche la location correspondant à l'URI
 * 		4. _isMethodAllowed() vérifie que la méthode est autorisée
 * 		5. _handleGET/POST/DELETE() exécute l'action
 * 		6. Une Response est retournée au client
 *
 * 		Gestion des erreurs :
 * 		- Les erreurs HTTP sont converties en réponses avec pages d'erreur personnalisées
 * 		- Fallback sur une page d'erreur générée si le fichier personnalisé n'existe pas
 */
class	RequestHandler {

	private:

		/**
		* @brief	Liste de toutes les configurations de serveurs
		* @details	Vecteur contenant les ServerConfig parsées depuis le fichier de configuration
		* 			Chaque élément représente un serveur virtuel indépendant
		*/
		std::vector<ServerConfig>	_servers;


		// ====== ROUTING ======

		/**
		* @brief	Trouve la configuration du serveur pour un port donné
		* @param	port Le numéro de port à chercher (ex: 8080)
		* @return	Pointeur vers ServerConfig si trouvé, NULL sinon
		* @details	Itère sur _servers pour trouver un match de port
		*/
		ServerConfig*	_findServerConfig(int port);

		/**
		* @brief	Trouve la configuration de location pour un URI donné
		* @param	server Pointeur vers la ServerConfig
		* @param	uri L'URI demandé (ex: "/upload", "/assets")
		* @return	Pointeur vers LocationConfig si trouvé, NULL sinon
		* @details	Cherche un exact match entre URI et location.path
		*/
		LocationConfig*	_findLocation(ServerConfig* server, const std::string &uri);

		/**
		* @brief	Vérifie si une méthode HTTP est autorisée pour une location
		* @param	loc Pointeur vers la LocationConfig
		* @param	method La méthode à vérifier (GET, POST, DELETE, etc.)
		* @return	true si autorisée, false sinon
		* @details	Vérifie que method est dans loc->allowedMethods
		*/
		bool			_isMethodAllowed(LocationConfig* loc, const std::string &method);



		// ====== FILE OPERATIONS ======

		/**
		* @brief	Convertit un nombre long en string (décimal)
		* @param	num Le nombre à convertir
		* @return	Représentation décimale en string
		* @details	Utilisé principalement pour Content-Length et affichage de tailles
		*/
		std::string		_intToString(long num);

		/**
		* @brief	Construit le chemin d'accès complet au fichier sur disque
		* @param	loc Pointeur vers LocationConfig contenant le root
		* @param	uri L'URI demandé par le client
		* @return	Chemin absolu vers le fichier (ex: "www/server1/upload")
		* @details	Combine loc->root avec l'URI, gère les slashes
		*/
		std::string		_buildFilePath(LocationConfig* loc, const std::string &uri);

		/**
		* @brief	Lit le contenu complet d'un fichier
		* @param	filePath Chemin absolu du fichier
		* @return	Contenu du fichier en binary string
		* @throw	ResponseE si le fichier ne peut pas être ouvert
		* @details	Lit le fichier en mode binaire (préserve les bytes exacts)
		*/
		std::string		_getFileContent(const std::string &filePath);

		/**
		* @brief	Détermine le type MIME d'un fichier selon son extension
		* @param	filePath Chemin du fichier
		* @return	Type MIME (ex: "text/html", "image/png")
		* @details	Supporte 18 types MIME courants, défaut: "application/octet-stream"
		*/
		std::string		_getMimeType(const std::string &filePath);

		/**
		* @brief	Vérifie l'existence d'un fichier ou dossier
		* @param	filePath Chemin à vérifier
		* @return	true si existe, false sinon
		* @details	Utilise stat() pour la vérification
		*/
		bool			_fileExists(const std::string &filePath);

		/**
		* @brief	Vérifie si un chemin correspond à un dossier
		* @param	filePath Chemin à vérifier
		* @return	true si c'est un dossier, false sinon
		* @details	Utilise S_ISDIR() sur struct stat
		*/
		bool			_isDirectory(const std::string &filePath);

		/**
		* @brief	Supprime un fichier du disque
		* @param	filePath Chemin absolu du fichier à supprimer
		* @return	true si la suppression réussit, false sinon
		* @details	Utilise la fonction C standard remove() de <cstdio>
		* 			Retourne true si remove() == 0 (succès)
		*/
		bool			_deleteFile(const std::string &filePath);

		/**
		* @brief	Écrit du contenu dans un fichier
		* @param	filePath Chemin du fichier à créer/overwrite
		* @param	content Contenu à écrire
		* @return	true si succès, false sinon
		* @details	Crée le fichier en mode binaire, overwrite si existe
		*/
		bool			_writeFile(const std::string &filePath, const std::string &content);

		// ====== BODY HANDLING ======

		/**
		* @brief	Vérifie si une requête HTTP est complète (headers + body)
		* @param	rawData Les données brutes reçues du socket
		* @param	request Objet Request parsé (pour Content-Length)
		* @return	true si la requête est complète, false sinon
		* @details	Sans Content-Length = requête complète (GET, HEAD, etc.)
		* 			Avec Content-Length = vérifie que bodySize >= Content-Length
		*/
		bool			_isBodyComplete(const std::string &rawData, const Request &request);

		/**
		* @brief	Vérifie si la taille du body dépasse la limite du serveur
		* @param	bodySize Taille du body en octets
		* @param	server Pointeur vers ServerConfig (contient maxBodySize)
		* @return	true si body trop volumineux, false sinon
		* @details	Retourne false si maxBodySize <= 0 (pas de limite)
		*/
		bool			_isBodyTooLarge(long bodySize, ServerConfig* server);

		// ====== DIRECTORY HANDLING ======

		/**
		* @brief	Génère une page HTML listant le contenu d'un dossier (autoindex)
		* @param	dirPath Chemin absolu du dossier
		* @param	uri L'URI d'origine pour les liens relatifs
		* @return	HTML table contenant les fichiers et dossiers
		* @details	Utilise opendir/readdir/closedir POSIX
		* 			Affiche taille pour fichiers, "[DIR]" pour dossiers
		*/
		std::string		_generateAutoindex(const std::string &dirPath, const std::string &uri);

		/**
		* @brief	Extrait le nom de fichier de l'URI pour upload
		* @param	request L'objet Request contenant l'URI
		* @return	Nom du fichier (ex: "image.png" de "/upload/image.png")
		* @details	Fallback à "uploaded_file.bin" si URI sans nom
		*/
		std::string		_extractFileName(const Request &request);

		// ====== HTTP METHODS HANDLERS ======

		/**
		* @brief	Traite une requête GET (lecture de fichiers)
		* @param	request La requête HTTP parsée
		* @param	server Configuration du serveur
		* @param	loc Configuration de la location
		* @return	Response HTTP (200, 404, 403, etc.)
		* @details	Logique :
		* 			1. Si dossier : cherche index file, sinon autoindex/404
		* 			2. Si fichier existe : retourne le contenu
		* 			3. Sinon : retourne 404
		*/
		Response		_handleGET(const Request &request, ServerConfig* server, LocationConfig* loc);

		/**
		* @brief	Traite une requête POST (upload/création de données)
		* @param	request La requête HTTP parsée (contient le body)
		* @param	server Configuration du serveur
		* @param	loc Configuration de la location
		* @return	Response HTTP (200, 405, 413, 500, etc.)
		* @details	Logique :
		* 			1. Vérifie body <= maxBodySize (sinon 413)
		* 			2. Vérifie allowUpload est true (sinon 405)
		* 			3. Écrit le body dans uploadStore/filename
		* 			4. Retourne 200 ou 500 si erreur d'écriture
		*/
		Response		_handlePOST(const Request &request, ServerConfig* server, LocationConfig* loc);

		/**
		* @brief	Traite une requête DELETE (suppression de fichiers)
		* @param	request La requête HTTP parsée
		* @param	server Configuration du serveur
		* @param	loc Configuration de la location
		* @return	Response HTTP (204, 404, 403, 500)
		* @details	Logique :
		* 			1. Vérifie existence fichier (sinon 404)
		* 			2. Empêche suppression dossiers (sinon 403)
		* 			3. Supprime le fichier
		* 			4. Retourne 204 No Content ou 500 si erreur
		*/
		Response		_handleDELETE(const Request &request, ServerConfig* server, LocationConfig* loc);

		// ====== RESPONSE BUILDING ======

		/**
		* @brief	Construit une Response HTTP valide avec statut, headers et body
		* @param	code Code de statut HTTP (200, 404, 500, etc.)
		* @param	message Message du statut (OK, Not Found, Internal Server Error)
		* @param	body Contenu du body (HTML, JSON, fichier, etc.)
		* @param	mimeType Type MIME du contenu (text/html, image/png, etc.)
		* @return	Response objet complètement construit et prêt à envoyer
		* @details	Ajoute automatiquement Content-Length et Content-Type
		*/
		Response		_buildResponse(int code, const std::string &message,
										const std::string &body, const std::string &mimeType);

		/**
		* @brief	Charge une page d'erreur personnalisée depuis la config
		* @param	code Code HTTP d'erreur (404, 500, etc.)
		* @param	message Message d'erreur
		* @param	server Pointeur vers ServerConfig (errorPages map)
		* @return	Contenu HTML de la page d'erreur, ou fallback généré
		* @details	Cherche dans server->errorPages[code]
		* 			Si fichier n'existe pas ou erreur, génère du HTML simple
		*/
		std::string		_loadErrorPage(int code, const std::string &message, ServerConfig* server);

		/**
		* @brief	Construit une Response d'erreur HTTP complète
		* @param	code Code HTTP (400, 404, 405, 413, 500, etc.)
		* @param	message Message d'erreur
		* @param	server Configuration du serveur (pour pages personnalisées)
		* @return	Response avec page d'erreur
		* @details	Combine _loadErrorPage et _buildResponse
		*/
		Response		_errorResponse(int code, const std::string &message, ServerConfig* server);

	public:

		/**
		* @brief	Constructeur
		* @param	servers Référence constante au vecteur de ServerConfig
		* @details	Stocke les configurations pour utilisation ultérieure
		*/
		RequestHandler(const std::vector<ServerConfig>& servers);

		/**
		* @brief	Destructeur
		* @details	Nettoie les ressources (virtuelles en C++98)
		*/
		~RequestHandler();

		/**
		* @brief	Point d'entrée principal : traite une requête HTTP complète
		* @param	request L'objet Request parsé depuis rawData
		* @param	rawData Les données brutes HTTP (pour vérifications additionelles)
		* @param	port Le port du serveur qui a reçu la requête
		* @return	Response HTTP complète prête à envoyer
		*
		* @details	Effectue le routing complet :
		* 			1. Trouve ServerConfig selon port
		* 			2. Trouve LocationConfig selon URI
		* 			3. Vérifie autorisations (method, body size)
		* 			4. Appelle _handleGET/POST/DELETE
		* 			5. Retourne Response ou erreur
		*
		* @note	Ne lève pas d'exceptions, retourne toujours une Response
		* 		(même en cas d'erreur : réponse 500, 404, etc.)
		*/
		Response	handleRequest(const Request& request, const std::string &rawData, int port);
};

#endif




