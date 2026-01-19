/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:49:52 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 12:10:50 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <cctype>
#include <cstdlib>
#include "Exceptions.hpp"
#include "HTTPParser.hpp"

#define MAX_HEADERS 50

class	Request {
private:

	//		ATTRIBUTS

	/**
	* @brief	La méthode HTTP de la requête
	* @details	Acceptées : GET, POST, DELETE, PUT, HEAD, OPTIONS
	* 			Exemple : "POST"
	*/
	std::string			_method;
	/**
	* @brief	L'URI (Uniform Resource Identifier) demandé
	* @details	Le chemin de la ressource, potentiellement avec une query string
	* 			Exemple : "/api/users?id=42" ou "/index.html"
	*/
	std::string			_uri;
	/**
	* @brief	La version du protocole HTTP utilisée
	* @details	Format : "HTTP/1.1" ou "HTTP/1.0"
	* 			Extrait de la première ligne de la requête
	*/
	std::string			_version;
	/**
	* @brief	Tableau des clés d'en-têtes (pairs avec _headerValues)
	* @details	Contient jusqu'à MAX_HEADERS clés, dans l'ordre d'apparition
	* 			Exemples : "Host", "Content-Type", "Content-Length", "Connection"
	*/
	std::string			_headersKeys[MAX_HEADERS];
	/**
	* @brief	Tableau des valeurs d'en-têtes (pairs avec _headersKeys)
	* @details	Contient jusqu'à MAX_HEADERS valeurs, dans l'ordre d'apparition
	* 			Exemples : "localhost:8080", "application/json", "42", "keep-alive"
	*/
	std::string			_headerValues[MAX_HEADERS];
	/**
	* @brief	Nombre d'en-têtes actuellement stockés
	* @details	Compté pendant le parsing, vérifié pour ne pas dépasser MAX_HEADERS
	* 			Permet d'itérer facilement sur les en-têtes présents
	*/
	int					_headerCount;
	/**
	* @brief	Le corps de la requête
	* @details	Contient les données POST/PUT, généralement en JSON ou form-encoded
	* 			Vide pour GET, DELETE, HEAD, OPTIONS
	* 			Exemple : {"username": "john", "password": "secret"}
	*/
	std::string			_body;

public:
	/**
	* @brief	Constructeur par défaut
	* @details	Initialise _headerCount à 0 et laisse les autres champs vides
	* 			Un objet Request nouvellement créé n'est pas valide
	* 			Il faut appeler parse() pour le remplir
	*/
	Request();
	/**
	* @brief	Destructeur
	* @details	Libère la mémoire utilisée par les champs de la requête
	*/
	~Request();

	/**
	* @brief	Récupère la méthode HTTP de la requête
	* @return	La méthode HTTP (GET, POST, DELETE, PUT, HEAD, OPTIONS)
	* 			Vide si parse() n'a pas été appelé ou a échoué
	* @note	Case-sensitive : "GET" != "get"
	*/
	std::string		getMethod() const;
	/**
	* @brief	Récupère l'URI de la requête
	* @details	C'est le chemin complet demandé, incluant la query string s'il y en a une
	* @return	L'URI (ex: "/api/users?id=42" ou "/index.html")
	* 			Vide si parse() n'a pas été appelé ou a échoué
	* @note	L'URI est conservé tel quel, sans décodage URL
	*/
	std::string		getUri() const;
	/**
	* @brief	Récupère la version HTTP utilisée
	* @details	Format standard : "HTTP/1.1" ou "HTTP/1.0"
	* @return	La version HTTP extraite de la première ligne
	* 			Vide si parse() n'a pas été appelé ou a échoué
	*/
	std::string		getVersion() const;
	/**
	* @brief	Récupère la valeur d'un en-tête spécifique
	* @details	Recherche l'en-tête par sa clé de manière case-insensitive
	* 			(c'est-à-dire que "Host", "host", "HOST" donnent le même résultat)
	*
	* @param	key La clé d'en-tête à chercher (ex: "Host", "Content-Type")
	* @return	La valeur de l'en-tête si trouvé
	* 			Une chaîne vide si l'en-tête n'existe pas
	*
	* @note	La recherche est case-insensitive mais retourne la valeur exacte telle que stockée
	* @note	Utilise _toLower() pour la comparaison case-insensitive
	*
	* Exemples :
	* @code
	* Request req;
	* req.parse("GET / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 42\r\n\r\n");
	*
	* std::string host = req.getHeader("Host");           // "example.com"
	* std::string ct = req.getHeader("content-type");     // "" (not present)
	* std::string cl = req.getHeader("CONTENT-LENGTH");   // "42" (case-insensitive)
	* @endcode
	*/
	std::string		getHeader(const std::string &key) const;
	/**
	* @brief	Récupère le corps de la requête
	* @details	Contient les données envoyées par le client (pour POST/PUT)
	* @return	Le corps de la requête
	* 			Vide pour GET, DELETE, HEAD, OPTIONS ou si aucun corps n'a été envoyé
	* @note	Conserve le corps tel quel, sans décodage ou validation
	* 		Le serveur doit utiliser l'en-tête Content-Length pour connaître la taille
	*/
	std::string		getBody() const;
	/**
	* @brief	Récupère le nombre d'en-têtes parsés
	* @return	Le nombre d'en-têtes actuellement stockés
	* @note	Utile pour itérer sur tous les en-têtes avec getHeaderKey/Value
	*/
	int				getHeaderCount() const;
	/**
	* @brief	Récupère la clé d'en-tête à un index donné
	* @param	index L'index de l'en-tête (0 à getHeaderCount()-1)
	* @return	La clé d'en-tête (ex: "Host", "Content-Type")
	* 			Vide si l'index est hors limites
	* @note	Utile pour itérer sur tous les en-têtes
	*/
	std::string		getHeaderKey(int index) const;
	/**
	* @brief	Récupère la valeur d'en-tête à un index donné
	* @param	index L'index de l'en-tête (0 à getHeaderCount()-1)
	* @return	La valeur d'en-tête (ex: "localhost:8080", "application/json")
	* 			Vide si l'index est hors limites
	* @note	Utile pour itérer sur tous les en-têtes
	*/
	std::string		getHeaderValue(int index) const;

	void			loadFromRaw(const RawRequest &raw);

};
