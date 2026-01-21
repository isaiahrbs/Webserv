/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 08:31:11 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/21 08:24:12 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEHANDLER_HPP
# define FILEHANDLER_HPP

# include <fstream>
# include <sstream>
# include <string>
# include <vector>
# include <cstdlib>
# include <sys/types.h>
# include <sys/stat.h>
# include <dirent.h>
# include "HTTPCommon.hpp"

/*	============================================================================
		FILE HANDLER CLASS
	============================================================================ */

	/**
	* @class	FileHandler
	* @brief	Gère TOUTES les opérations sur fichiers et répertoires
	* @details	Sépare complètement "logique métier fichiers" de "logique HTTP"
	*			RequestHandler n'a plus besoin de connaître stat(), opendir(), etc.
	*			C'est le SEUL endroit où on fait du I/O disque
	*
	* @note		C++98 compatible, pas de exceptions levées (retourne bool/string vide)
	*/
	class	FileHandler {

		public:
			/* ===== FILE EXISTENCE & TYPE ===== */

			/**
			* @brief	Vérifie qu'un fichier/dossier existe
			* @param	path Chemin absolu à vérifier
			* @return	true si existe, false sinon
			*/
			static bool			exists(const std::string &path);

			/**
			* @brief	Vérifie si c'est un dossier
			* @param	path Chemin absolu
			* @return	true si c'est un répertoire, false sinon
			*/
			static bool			isDirectory(const std::string &path);

			/**
			* @brief	Vérifie si c'est un fichier régulier
			* @param	path Chemin absolu
			* @return	true si c'est un fichier, false sinon
			*/
			static bool			isFile(const std::string &path);

			/* ===== FILE CONTENT ===== */

			/**
			* @brief	Lit le contenu complet d'un fichier
			* @param	path Chemin absolu du fichier
			* @return	Contenu binaire du fichier, "" si erreur
			* @details	Lit en mode binaire (préserve les bytes exacts)
			*			Retourne "" en cas d'erreur (pas de exception)
			*/
			static std::string	getContent(const std::string &path);

			/**
			* @brief	Écrit du contenu dans un fichier
			* @param	path Chemin absolu du fichier
			* @param	content Contenu à écrire
			* @return	true si succès, false sinon
			* @details	Crée le fichier ou l'overwrite si existe
			*/
			static bool			writeContent(const std::string &path, const std::string &content);

			/**
			* @brief	Supprime un fichier du disque
			* @param	path Chemin absolu du fichier
			* @return	true si suppression réussie, false sinon
			* @details	N'accepte que les fichiers, pas les dossiers
			*/
			static bool			deleteFile(const std::string &path);

			/**
			* @brief	Récupère la taille en octets d'un fichier
			* @param	path Chemin absolu
			* @return	Taille du fichier, -1 si erreur
			*/
			static long			getFileSize(const std::string &path);

			/**
			* @brief	Extrait l'extension d'un fichier
			* @param	path "image.png" → ".png"
			* @return	Extension avec le point, "" si pas d'extension
			*/
			static std::string	getFileExtension(const std::string &path);

			/* ===== DIRECTORY OPERATIONS ===== */

			/**
			* @brief	Génère du HTML pour lister un répertoire (autoindex)
			* @param	dirPath Chemin absolu du dossier
			* @param	uri URI originale d'accès (pour les liens)
			* @return	HTML table avec fichiers et dossiers, "" si erreur
			* @details	Utilise opendir/readdir/closedir POSIX
			*			Affiche : tailles pour fichiers, "[DIR]" pour dossiers
			*/
			static std::string	generateDirectoryListing(const std::string &dirPath,
														const std::string &uri);

			/**
			* @brief	Cherche un fichier index dans un répertoire
			* @param	dirPath Chemin absolu du dossier
			* @param	indexFiles Noms à chercher ("index.html", "index.htm", etc.)
			* @return	Chemin complet du fichier trouvé, "" si aucun
			* @details	Cherche dans l'ordre de indexFiles, retourne le premier match
			*/
			static std::string	findIndexFile(const std::string &dirPath,
											const std::vector<std::string> &indexFiles);

			/* ===== PATH OPERATIONS ===== */

			/**
			* @brief	Construit le chemin absolu complet d'une ressource
			* @param	root Root de la location (ex: "www/server1")
			* @param	uri URI demandée (ex: "/upload/image.png")
			* @return	Chemin absolu complet (ex: "www/server1/upload/image.png")
			* @details	Gère les slashes, normalise le chemin
			*/
			static std::string	buildFilePath(const std::string &root, const std::string &uri);

			/**
			* @brief	Extrait le nom de fichier d'une URI
			* @param	uri "/path/to/file.txt"
			* @return	"file.txt"
			*/
			static std::string	extractFileName(const std::string &uri);

			/**
			* @brief	Normalise un chemin (prévient directory traversal)
			* @param	path Le chemin à sécuriser
			* @return	Chemin normalisé sans "..", "."
			* @details	SÉCURITÉ : Prévient les attaques "../../../etc/passwd"
			*			Utilise realpath() pour résoudre les liens symboliques
			*/
			static std::string	normalizePath(const std::string &path);
};

#endif
