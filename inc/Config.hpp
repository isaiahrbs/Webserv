/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/07 17:55:27 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/07 18:36:42 by dinguyen         ###   ########.fr       */
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
