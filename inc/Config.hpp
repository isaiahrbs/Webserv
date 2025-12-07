/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/07 17:55:27 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/07 19:00:59 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>

/**
 * @brief	Contient toute la configuration pour une route spécifique (un bloc "location")
 * ---> Chaque fois qu'une requete arrive, le serveur trouve la configuration qui correspond
 * a l'URI (uniform resource identifier : page web, photo, video...) demandée pour savoir
 * comment la traiter ensuite.
 * */
struct	LocationConfig {
	/**
	 * @brief	Le chemin de l'URL de la configuration concernée
	 * @details	Exemple: "/images", "/api/users", "/doc"
	 */
	std::string							path;
	/**
	 *  @brief	Le dossier racine sur le disque ou le serveur va chercher les fichiers pour la route.
	 * @details	Ex: si root est ./www et que l'URI est "/images/logo.png", le serveur cherchera
	 * 				./www/images/logo.png
	 */
	std::string							root;
	/**
	 * @brief	la liste des methodes HTTP autorisées pour cette route.
	 * @details	Si une requete arrive avec une methode non listée, on renvoit une erreur 405 "Method not allowed"
	 * 			Ex: GET POST DELETE UNKNOWNMETHOD
	 */
	std::vector<std::string>			allowedMethods;
	/**
	 * @brief	Le fichier a servir par defaut si l'URI demandée est un dossier.
	 * @details	Ex: "index.html", "home.php", "/doc"
	 */
	std::string							index;
	/**
	 * @brief	active ou desactive(true/false) le listing automatique du contenu d'un dossier
	 * @details	si true et aucun fichier trouvé pour "index", serveur genere une page HTML avec la liste des fichiers
	 */
	bool								autoIndex;
	/**
	 * @brief	l'URL vers laquelle rediriger le client.
	 * @details	Si ce champ n'est pas vide, le sereur enverra une rponse de redirection(301. "moved permanently")
	 */
	std::string							redirectUrl;
	/**
	 * @brief	Active ou desactive la possibilité d'upload des fichiers sur cette route.
	 */
	bool								allowUpload;
	/**
	 * @brief	Le dossier sur le disque ou les fichiers uploadé doivent etre sauvegardés.
	 */
	std::string							uploadStore;
	/**
	 * @brief	Associe une extension de fichiers (ex: ".py") au chemin de l'executable qui doit l'interpreter.
	 * @details	Bonus CGI : la map permet de gérer plusieurs CGI (python/PHP/etc).
	 * 			Ex:	{".py": "usr/bin/python3", ".php": "/usr/bin/php-cgi"}
	 */
	std::map<std::string, std::string>	cgiHandlers;
};

/**
 * @brief	Contient la config complete pour un serveur virtuel (un bloc "server").
 * 	Chaque instance de cette structure represente un serveur qui ecoute sur un port specifique et possede son propre
 * 	ensemble de règles.
 */
struct	ServerConfig {
	/**
	 * @brief	le port sur lequel ce serveur ecoute les connexions entrantes.
	 */
	int							port;
	/**
	 * @brief	l'adresse IP sur laquelle le serveur ecoute.
	 * @details	Souvent 127.0.0.1 (localhost) ou 0.0.0.0 (toutes les interfaces)
	 */
	std::string					host;
	/**
	 * @brief	La taille maximale autorisée pour le corps d'une requete client, en octets.
	 * @details	Si une requete depasse cette taille, le serveur doit renvoyer une erreur 413 Payload Too large.
	 */
	long						maxBodySize;
	/**
	 * @brief	Associe un code d'erreur HTTP (ex 404) au chemin d'un fichier HTML personnalisé
	 * @details	Ex: {404:} "./errors/404.html " etc...
	 */
	std::map<int, std::string>	errorPages;
	/**
	 * @brief	Une liste de toutes les configurations de routes (location) pour ce serveur.
	 */
	std::vector<LocationConfig>	locations;
	/**
	 *	@brief	Bonus : liste de noms de domaines pour ce serveur.
	 *	@details Permet de faire du virtual hosting: plusieurs sites sur le meme port, différencié par le header host
	 	de la requete.
	 */
	std::vector<std::string>	serverNames;
};

/**
 * @brief	Classe responsable du parsing du fichier de configuration.
 * 	Elle lit le fichier de configuration, valide sa syntaxe, et remplit une liste de structures ServerConfig avec les parametres lus.
 */

class	ConfigParser {

public:
	ConfigParser();
	~ConfigParser();

	/**
	 * @brief	Parse le fichier de configuration et retourne une liste de configurations de serveur.
	 * @param	filepath Le chemin vers le fichier de configuration.
	 * @return	Un vectur de structures ServerConfig
	 * @throw	Lance une exception en cas d'erreur de syntaxe ou de fichier non trouvé.
	 */
	std::vector<ServerConfig>	parse(const std::string &filepath);

private:

	/** @brief			LOADING....  */
};
