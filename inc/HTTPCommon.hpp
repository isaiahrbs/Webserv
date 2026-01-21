/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPCommon.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 08:14:10 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/21 08:18:54 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPCOMMON_HPP
# define HTTPCOMMON_HPP

# include <string>
# include <map>
# include <cstdlib>
# include <vector>
class RequestHandler;
class ServerConfig;

/*	============================================================================
		HTTP STATUS CODES
	============================================================================ */

# define HTTP_OK					200
# define HTTP_CREATED				201
# define HTTP_ACCEPTED				202
# define HTTP_NO_CONTENT			204
# define HTTP_MOVED_PERMANENTLY		301
# define HTTP_FOUND					302
# define HTTP_NOT_MODIFIED			304
# define HTTP_BAD_REQUEST			400
# define HTTP_UNAUTHORIZED			401
# define HTTP_FORBIDDEN				403
# define HTTP_NOT_FOUND				404
# define HTTP_METHOD_NOT_ALLOWED	405
# define HTTP_CONFLICT				409
# define HTTP_PAYLOAD_TOO_LARGE		413
# define HTTP_INTERNAL_SERVER_ERROR	500
# define HTTP_NOT_IMPLEMENTED		501
# define HTTP_SERVICE_UNAVAILABLE	503

/*	============================================================================
		HTTP METHODS
	============================================================================ */

# define HTTP_METHOD_GET			0
# define HTTP_METHOD_POST			1
# define HTTP_METHOD_PUT			2
# define HTTP_METHOD_DELETE			3
# define HTTP_METHOD_HEAD			4
# define HTTP_METHOD_OPTIONS		5
# define HTTP_METHOD_PATCH			6
# define HTTP_METHOD_UNKNOWN		-1

/*	============================================================================
		HTTP HELPER FUNCTIONS
	============================================================================ */

	/**
	* @brief	Convertit string de méthode HTTP en constante
	* @param	method "GET", "POST", "DELETE", etc.
	* @return	HTTP_METHOD_GET, HTTP_METHOD_POST, etc. ou HTTP_METHOD_UNKNOWN
	*/
	int				httpStringToMethod(const std::string &method);

	/**
	* @brief	Convertit constante méthode en string HTTP
	* @param	method HTTP_METHOD_GET, HTTP_METHOD_POST, etc.
	* @return	"GET", "POST", "DELETE", etc.
	*/
	std::string		httpMethodToString(int method);

	/**
	* @brief	Convertit un entier long en string
	* @param	num Le nombre à convertir
	* @return	Représentation string du nombre
	*/
	std::string		httpIntToString(long num);

	/**
	* @brief	Convertit une string en minuscules
	* @param	str La string à convertir
	* @return	La string convertie en minuscules
	* @note		C++98 compatible, gère A-Z uniquement
	*/
	std::string		httpToLower(const std::string &str);

	/**
	* @brief	Retourne le message de statut HTTP
	* @param	code HTTP_OK, HTTP_NOT_FOUND, etc.
	* @return	"OK", "Not Found", "Internal Server Error", etc.
	*/
	std::string		httpStatusCodeToMessage(int code);

	/**
	* @brief	Valide que le numéro de version HTTP est supporté
	* @param	version "HTTP/1.0" ou "HTTP/1.1"
	* @return	true si valide, false sinon
	*/
	bool			httpIsValidVersion(const std::string &version);

	/**
	* @brief	Valide qu'une chaîne est une méthode HTTP connue
	* @param	method "GET", "POST", "DELETE", etc.
	* @return	true si valide, false sinon
	*/
	bool			httpIsValidMethod(const std::string &method);

	/**
	* @brief	Retourne le type MIME pour une extension de fichier
	* @param	filename "image.png", "script.html", etc.
	* @return	"image/png", "text/html", "application/octet-stream" (default)
	*/
	std::string		httpGetMimeType(const std::string &filename);

	/**
	* @brief	Initialise la map MIME_TYPES (appeler une fois au démarrage)
	*/
	void			httpInitMimeTypes(void);

/*	============================================================================
		HTTP SERVER ENGINE - FACADE POUR SIMPLIFIER L'INTÉGRATION
	============================================================================ */

	/**
	* @class	HTTPServerEngine
	* @brief	Facade qui regroupe TOUT le parsing/routing/serialisation en UNE fonction
	* @details	Permet à la couche serveur (server::run()) d'appeler une seule fonction
	*			au lieu de 5-6 fonctions différentes. Gère aussi les erreurs HTTP.
	*
	* Utilisation:
	*	1. Créer UNE SEULE fois dans le constructeur du serveur:
	*		_engine = new HTTPServerEngine(servers_config);
	*
	*	2. Pour chaque requête HTTP complète:
	*		std::string response = _engine->processRequest(rawData, port);
	*		client->getResponseBuffer() = response;
	*		client->getRequestBuffer().clear();
	*/
	class HTTPServerEngine {

		private:
			RequestHandler*	_handler;

		public:
		/**
		* @brief Constructor - initialiser une seule fois au démarrage
		* @param servers Configuration de tous les serveurs virtuels
		*/
		HTTPServerEngine(const std::vector<ServerConfig> &servers);
		/**
		* @brief Destructor
		*/
		~HTTPServerEngine();

		/**
		* @brief	UNE SEULE FONCTION pour traiter une requête complète
		* @param	rawData Les bytes bruts reçus du client
		*			Exemple: "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
		* @param	clientPort Le port du serveur auquel le client s'est connecté
		*			(8080, 8081, etc) - utilisé pour router vers la bonne config
		* @return	std::string = réponse HTTP COMPLÈTE, prête à envoyer au client
		*			Inclut: status line, headers, et body
		*
		* Gestion d'erreurs automatique:
		* - RequestE (parse error) → 400 Bad Request
		* - Autres exceptions → 500 Internal Server Error
		*/
		std::string processRequest(const std::string &rawData, int clientPort);
};

#endif
