/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 08:31:25 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 12:15:25 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <string>
# include <map>
# include <vector>
# include <unistd.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <signal.h>
# include <cstring>
# include "Config.hpp"
# include "Request.hpp"
# include "FileHandler.hpp"
# include "HTTPCommon.hpp"

/*	============================================================================
		CGI RESULT STRUCTURE
	============================================================================ */

	/**
	* @struct	CGIResult
	* @brief	Résultat d'exécution d'un script CGI
	* @details	Contient le code de sortie et la sortie capturée
	*/
	struct	CGIResult {
		int			exitCode;		// Code de sortie du process (0 = succès)
		std::string	output;			// stdout du script CGI (peut être vide)
		bool		success;		// true si processus complété correctement
	};

/*	============================================================================
		CGI HANDLER CLASS
	============================================================================ */

	/**
	* @class	CGIHandler
	* @brief	Gère complètement l'exécution de scripts CGI
	* @details	Sépare la complexité CGI de RequestHandler
	*			- Detection des fichiers CGI
	*			- Exécution fork/execve/pipe (POSIX)
	*			- Construction de l'environnement RFC 3875
	*			- Capture de stdout
	*
	* @note		RFC 3875 : Common Gateway Interface 1.1
	*/
	class	CGIHandler {

		public:
			/* ===== CGI DETECTION ===== */
			/**
			* @brief	Détecte si un fichier doit être exécuté en CGI
			* @param	filePath Chemin du fichier (ex: "/scripts/test.py")
			* @param	handlers Map des CGI handlers de la location
			*					(ex: ".py" → "/usr/bin/python3")
			* @return	true si extension match un handler
			* @details	Cherche l'extension du fichier dans la map handlers
			*/
			static bool	isCGI(const std::string &filePath,
							const std::map<std::string, std::string> &handlers);

			/**
			* @brief	Récupère l'interprète pour un fichier CGI
			* @param	filePath Chemin du fichier (ex: "test.py")
			* @param	handlers Map des CGI handlers
			* @return	Interprète (ex: "/usr/bin/python3"), "" si pas trouvé
			* @details	Utilise getFileExtension() pour trouver le bon handler
			*/
			static std::string	getCGIInterpreter(const std::string &filePath,
												const std::map<std::string, std::string> &handlers);

			/* ===== CGI EXECUTION ===== */

			/**
			* @brief	Exécute un script CGI et retourne le résultat
			* @param	scriptPath Chemin absolu du script (ex: "www/server2/scripts/test.py")
			* @param	request L'objet Request (pour méthode, headers, body)
			* @param	server Configuration du serveur (pour variables d'env)
			* @param	handlers Map des CGI handlers
			* @return	CGIResult avec exitCode et output
			*
			* @details	Processus :
			*			1. Construit les variables d'environnement RFC 3875
			*			2. Crée 2 pipes (stdout du script)
			*			3. fork() : crée process enfant
			*			4. Enfant : execve() le script avec l'interprète
			*			5. Parent : lis stdout via pipe, wait() pour fin
			*			6. Retourne CGIResult avec la sortie capturée
			*
			* @note		Timeout : 5 secondes max par défaut
			*			Si timeout : output vide, success = false
			*/
			static CGIResult	execute(const std::string &scriptPath, const Request &request,
									const ServerConfig &server, const std::map<std::string, std::string> &handlers);

	private:
		/* ===== ENVIRONMENT BUILDING (RFC 3875) ===== */
		/**
		* @brief	Construit l'environnement CGI RFC 3875
		* @param	request L'objet Request
		* @param	scriptPath Chemin du script
		* @param	server Configuration du serveur
		* @return	Map de variables d'environnement prête pour execve()
		* @details	Variables standard RFC 3875 :
		*			- REQUEST_METHOD : GET, POST, DELETE
		*			- SCRIPT_NAME : "/scripts/test.py"
		*			- QUERY_STRING : "id=42&name=test"
		*			- CONTENT_TYPE : "application/x-www-form-urlencoded"
		*			- CONTENT_LENGTH : "42"
		*			- SERVER_NAME : "localhost"
		*			- SERVER_PORT : "8081"
		*			- SERVER_PROTOCOL : "HTTP/1.1"
		*			- SERVER_SOFTWARE : "webserv/1.0"
		*			- GATEWAY_INTERFACE : "CGI/1.1"
		*			- HTTP_* : Tous les headers (HTTP_HOST, HTTP_USER_AGENT, etc.)
		*			- PATH : copié de l'env du système
		*/
		static std::map<std::string, std::string>
				_buildCGIEnvironment(const Request &request, const std::string &scriptPath,
									const ServerConfig &server);

		/**
		* @brief	Convertit map d'env en char** pour execve()
		* @param	env Map de "VAR=value"
		* @return	Vecteur de char* terminé par NULL (pour execve)
		* @details	C++98 : utilise std::vector<char*> et .c_str()
		*/
		static std::vector<char*>
				_mapToCharArray(const std::map<std::string, std::string> &env);

		static std::string	_getPathInfo(const std::string &scriptPath, const std::string &uri);
};

#endif
