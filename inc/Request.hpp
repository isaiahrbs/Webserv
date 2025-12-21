/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:49:52 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/21 14:17:10 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include "Exceptions.hpp"

#define MAX_HEADERS 50

	/**
	* @class	Request
	* @brief	Classe responsable du parsing et du stockage des requêtes HTTP
	* @details	Cette classe parse une requête HTTP brute (string) et extrait :
	* 			- La méthode HTTP (GET, POST, DELETE, etc.)
	* 			- L'URI (le chemin demandé : /index.html, /api/users, etc.)
	* 			- La version HTTP (HTTP/1.0 ou HTTP/1.1)
	* 			- Les en-têtes (Host, Content-Type, Content-Length, etc.)
	* 			- Le corps de la requête (pour POST/PUT avec données)
	*
	* 		Format attendu (RFC 7230) :
	* 		GET /path HTTP/1.1\r\n
	* 		Host: localhost:8080\r\n
	* 		Content-Type: application/json\r\n
	* 		Content-Length: 42\r\n
	* 		\r\n
	* 		{"key": "value"}
	*
	* 		La classe valide que :
	* 		- La méthode est reconnue (GET, POST, DELETE, PUT, HEAD, OPTIONS)
	* 		- La version HTTP est supportée (1.0 ou 1.1)
	* 		- Le format des en-têtes est correct (clé: valeur)
	* 		- Le nombre d'en-têtes ne dépasse pas MAX_HEADERS (50)
	*
	* 		En cas d'erreur, elle lève une exception RequestE.
	 * 		Exemple d'utilisation :
	 * 		@code
	 * 		Request req;
	 * 		try {
	 * 			req.parse("POST /upload HTTP/1.1\r\nHost: example.com\r\n\r\n{...}");
	 * 			std::cout << req.getMethod();        // "POST"
	 * 			std::cout << req.getUri();           // "/upload"
	 * 			std::cout << req.getHeader("Host");  // "example.com"
	 * 		} catch (const RequestE &e) {
	 * 			std::cerr << "Parse error: " << e.what() << std::endl;
	 * 		}
	 * 		@endcode
	*/

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

	//		METHODES

	/**
	* @brief	Utilitaire pour diviser une chaine au premier délimiteur
	* @param	str La chaine à diviser
	* @param	delimiter Le caractère délimiteur
	* @param	pos Position de départ et de fin (modifiée)
	* @return	La portion de la chaine avant le délimiteur
	* @details	Cherche le délimiteur et met à jour pos pour pointer après
	* 			Retourne la portion jusqu'au délimiteur, ou le reste de la string
	* @throw	Aucune, peut retourner une string vide
	*/
	static std::string	_split(const std::string &str, char delimiter, size_t &pos);
	/**
	* @brief	Supprime les espaces au début et fin d'une chaine
	* @param	str La chaine à nettoyer
	* @return	La chaine sans espaces/tabs/newlines aux limites
	* @details	Supprime ' ', '\t', '\r', '\n' en début et fin
	* @throw	Aucune
	*/
	static std::string	_trim(const std::string &str);
	/**
	* @brief	Convertit une chaine en minuscules
	* @param	str La chaine à convertir
	* @return	La chaine en minuscules
	* @details	Utilisé pour les comparaisons case-insensitive d'en-têtes
	* @throw	Aucune
	*/
	static std::string	_toLower(std::string str);
	/**
	* @brief	Valide que la méthode HTTP est reconnue
	* @param	method La méthode à valider
	* @throw	RequestE si la méthode n'est pas reconnue
	* @details	Valide : GET, POST, DELETE, PUT, HEAD, OPTIONS
	*/
	void				_validateMethod(const std::string &method);
	/**
	* @brief	Valide que la version HTTP est supportée
	* @param	version La version à valider
	* @throw	RequestE si la version n'est pas supportée
	* @details	Accepte : HTTP/1.0, HTTP/1.1
	*/
	void				_validateVersion(const std::string &version);
	/**
	* @brief	Parse la première ligne de la requête
	* @param	requestLine La première ligne (METHOD URI VERSION)
	* @throw	RequestE si le format est invalide
	* @details	Extrait et valide method, uri, version
	*/
	void				_parseRequestLine(const std::string &requestLine);
	/**
	* @brief	Parse tous les en-têtes
	* @param	headersPortion La portion contenant les en-têtes
	* @throw	RequestE si un en-tête est mal formé
	* @details	Chaque ligne doit être au format "clé: valeur"
	* 			Arrête à la première ligne vide
	*/
	void				_parseHeaders(const std::string &headersPortion);
	/**
	* @brief	Sépare le corps du reste de la requête
	* @param	rawRequest La requête brute complète
	* @throw	Aucune
	* @details	Cherche le délimiteur \r\n\r\n ou \n\n
	* 			Le reste est stocké dans _body
	*/
	void				_extractBody(const std::string &rawRequest);
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
	* @brief	Parse une requête HTTP brute et la stocke dans l'objet
	* @details	Effectue les opérations suivantes :
	* 			1. Vérifie que la requête n'est pas vide
	* 			2. Sépare les en-têtes du corps (délimiteur \r\n\r\n ou "\n")
	* 			3. Extrait et valide la première ligne (méthode URI version)
	* 			4. Valide que la méthode est supportée (GET, POST, DELETE, etc.)
	* 			5. Valide que la version HTTP est supportée (1.0 ou 1.1)
	* 			6. Parse chaque en-tête en format "clé: valeur"
	* 			7. Nettoie les espaces des clés et valeurs
	* 			8. Stocke le corps restant
	*
	* 		Validations effectuées :
	* 		- Rejette les requêtes vides
	* 		- Rejette les méthodes inconnues
	* 		- Rejette les versions HTTP non supportées
	* 		- Rejette les en-têtes mal formés (pas de ':' ou clé/valeur vides)
	* 		- Rejette si plus de MAX_HEADERS (50) en-têtes
	* 		- Ignore les espaces et caractères de contrôle aux limites
	* 		- Gère les deux formats de fin de ligne : \r\n (Windows) et "\n" (Unix)
	*
	* @param	rawRequest La requête HTTP brute sous forme de string
	* 			Format : "METHOD URI VERSION\r\nHeader1: Value1\r\n...\r\n\r\nBODY"
	* 			Exemple : "POST /api/login HTTP/1.1\r\nHost: localhost\r\n\r\n{...}"
	*
	* @return	true si le parsing est réussi et la requête est valide
	*
	* @throw	RequestE si :
	* 			- La requête est vide
	* 			- La première ligne est invalide ou incomplète
	* 			- La méthode n'est pas reconnue
	* 			- La version HTTP n'est pas supportée
	* 			- Un en-tête est mal formé (pas de ':', clé/valeur vides)
	* 			- Le nombre d'en-têtes dépasse MAX_HEADERS (50)
	* 			- EOF atteint inopinément
	*
	* @note	Case-insensitif pour les clés d'en-têtes lors de la recherche (getHeader)
	* 		mais conserve la casse originale en stockage
	* @note	Nettoie automatiquement les sauts de ligne "\n".
	* @note	Coupe le corps au premier "\r\n" ou "\n" rencontré
	* @note	Utilise les méthodes privées _parseRequestLine, _parseHeaders, _extractBody
	*/
	bool		parse(const std::string &rawRequest);
	/**
	* @brief	Récupère la méthode HTTP de la requête
	* @return	La méthode HTTP (GET, POST, DELETE, PUT, HEAD, OPTIONS)
	* 			Vide si parse() n'a pas été appelé ou a échoué
	* @note	Case-sensitive : "GET" != "get"
	*/
	std::string	getMethod() const;
	/**
	* @brief	Récupère l'URI de la requête
	* @details	C'est le chemin complet demandé, incluant la query string s'il y en a une
	* @return	L'URI (ex: "/api/users?id=42" ou "/index.html")
	* 			Vide si parse() n'a pas été appelé ou a échoué
	* @note	L'URI est conservé tel quel, sans décodage URL
	*/
	std::string	getUri() const;
	/**
	* @brief	Récupère la version HTTP utilisée
	* @details	Format standard : "HTTP/1.1" ou "HTTP/1.0"
	* @return	La version HTTP extraite de la première ligne
	* 			Vide si parse() n'a pas été appelé ou a échoué
	*/
	std::string	getVersion() const;
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
	std::string	getHeader(const std::string &key) const;
	/**
	* @brief	Récupère le corps de la requête
	* @details	Contient les données envoyées par le client (pour POST/PUT)
	* @return	Le corps de la requête
	* 			Vide pour GET, DELETE, HEAD, OPTIONS ou si aucun corps n'a été envoyé
	* @note	Conserve le corps tel quel, sans décodage ou validation
	* 		Le serveur doit utiliser l'en-tête Content-Length pour connaître la taille
	*/
	std::string	getBody() const;
	/**
	* @brief	Récupère le nombre d'en-têtes parsés
	* @return	Le nombre d'en-têtes actuellement stockés
	* @note	Utile pour itérer sur tous les en-têtes avec getHeaderKey/Value
	*/
	int			getHeaderCount() const;
	/**
	* @brief	Récupère la clé d'en-tête à un index donné
	* @param	index L'index de l'en-tête (0 à getHeaderCount()-1)
	* @return	La clé d'en-tête (ex: "Host", "Content-Type")
	* 			Vide si l'index est hors limites
	* @note	Utile pour itérer sur tous les en-têtes
	*/
	std::string	getHeaderKey(int index) const;
	/**
	* @brief	Récupère la valeur d'en-tête à un index donné
	* @param	index L'index de l'en-tête (0 à getHeaderCount()-1)
	* @return	La valeur d'en-tête (ex: "localhost:8080", "application/json")
	* 			Vide si l'index est hors limites
	* @note	Utile pour itérer sur tous les en-têtes
	*/
	std::string	getHeaderValue(int index) const;
	/**
	* @brief	Réinitialise la requête
	* @details	Vide tous les champs et prépare pour un nouveau parse()
	* 			Utile si tu veux réutiliser le même objet Request
	*/
	void		reset();
	/**
	* @brief	Affiche le contenu de la requête (pour débogage)
	* @details	Affiche method, uri, version, tous les en-têtes et le corps
	* 			Format lisible pour vérifier le parsing
	* @note	Utile pour le débogage
	*/
	void		debug() const;
};
