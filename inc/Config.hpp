/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/07 17:55:27 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/21 13:26:57 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include "Exceptions.hpp"

	/**
	* @brief	Contient toute la configuration pour une route spécifique (un bloc "location")
	* 		Chaque fois qu'une requete arrive, le serveur trouve la configuration qui correspond
	* 		a l'URI (uniform resource identifier : page web, photo, video...) demandée pour savoir
	* 		comment la traiter ensuite.
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
	 * 			Chaque instance de cette structure represente un serveur qui ecoute sur un port specifique et possede son propre
	 * 			ensemble de règles.
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
	* @brief	Le répertoire racine du serveur (ex: www/server1/).
	* @details	Chemin de base à partir duquel le serveur servira les fichiers.
	*/
	std::string					root;
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
 * 			Elle lit le fichier de configuration, valide sa syntaxe, et remplit une liste de structures ServerConfig avec les parametres lus.
 */

class	ConfigParser {

public:
	ConfigParser();
	~ConfigParser();

	/**
	 * @brief	Parse le fichier de configuration complet et retourne une liste de configurations de serveur
	 * @details	Lit d'abord le fichier avec _readFile() et réinitialise la position de parsing.
	 * 			Puis boucle en utilisant _peekToken() pour vérifier si on atteint EOF:
	 * 			- Valide que le prochain token est "server"
	 * 			- Appelle _parseServerBlock() pour chaque bloc server trouvé
	 * 			- Ajoute chaque ServerConfig au vecteur de résultat
	 * 			Valide que au moins un bloc server a été trouvé.
	 * @param	filepath Le chemin vers le fichier de configuration à parser
	 * @return	Un std::vector<ServerConfig> contenant toutes les configurations de serveur parsées
	 * @throw	ConfigParserE si:
	 * 			- Le fichier ne peut pas être ouvert (via _readFile())
	 * 			- Aucun bloc "server" n'est trouvé dans le fichier
	 * 			- Un token autre que "server" est rencontré au niveau racine
	 * 			- Un bloc server est invalide (via _parseServerBlock())
	 * @note	Point d'entrée principal du ConfigParser
	 * 		Exemple: std::vector<ServerConfig> servers = parser.parse("config/server.conf");
	*/
	std::vector<ServerConfig>	parse(const std::string &filepath);

private:

	//		ATTRIBUTS

	/**
	 * @brief	Le contenu complet du fichier
	 */
	std::string					_fileContent;
	/**
	 * @brief	Position actuelle dans le fichier
	 */
	size_t						_position;
	/**
	 * @brief	Numero de ligne pour les msgs d'erreur
	 */
	int							_lineNumber;

	//		METHODES

	/**
	 * @brief	Lit le contenu du fichier dans _fileContent;
	 * @param	filepath Le chemin vers le fichier
	 * @details	Ouvre le fichier ligne par ligne et le concatène dans _fileContent.
	 * 			Chaque ligne est suivie d'un newline pour préserver la structure.
	 * 			Incrémente _lineNumber pour chaque newline rencontré.
	 * @throw	Lance une exception si le fichier n'est pas ouvert
	 */
	void						_readFile(const std::string &filepath);
	/**
	 * @brief	Saute lesespaces, tabulations et commentaires #
	 * @details Incrémente _lineNumber pour chaque newline rencontré
	*/
	void						_skipSpacesAndC();
	/**
	 * @brief	Genere un message d'erreur avec le numero de la ligne
	 * @param	msg Le message d'erreur
	 * @details	Formate le message en ajoutant le numéro de la ligne actuelle (_lineNumber)
	 * 			au début du message. Utile pour aider l'utilisateur à localiser les erreurs.
	 * @return	Un message formaté avec le numero de la ligne
	*/
	std::string					_formatErrorMsg(const std::string &msg);
	/**
	 * @brief	Convertit une chaine en entier
	 * @param	str La chaine a convertir
	 * @return	La valeur entiere
	 * @details	Valide que tous les caractères sont des chiffres.
	 * 			Puis convertit la chaîne en entier long.
	 * 			Vérifie que la valeur est entre 0 et 65535 (plage valide des ports).
	 * @throw	Lance une exception si la conversion echoue
	*/
	int							_stringToInt(const std::string &str);
	/**
	 * @brief	Convertit une chaine en booléen(on/off, true/false, yes/no)
	 * @param	str	La chaine a convertir
	 * @details	Accepte les formats suivants :
	 * 			- true : "on", "true", "yes", "1"
	 * 			- false : "off", "false", "no", "0"
	 * @return	La valeur booléenne
	*/
	bool						_stringToBool(const std::string &str);
	/**
	 * @brief	Lit le prochain token(valeur, keyword, symbole)
	 * @details	Saute d'abord les espaces, tabulations, commentaires et newlines.
	 * 			Puis extrait le prochain token : un mot (lettres/chiffres/underscores),
	 * 			un symbole spécial ({, }, ;, :), ou une chaîne entre guillemets.
	 * 			Incrémente _position pour pointer après le token lu.
	 * @return	Le token lu, ou une chaine vide si EOF
	*/
	std::string					_readToken();
	/**
	 * @brief	Regarde le prochain token sans le consommer
	 * @details	Sauvegarde la position actuelle (_position), appelle _readToken()
	 * 			pour lire le token suivant, puis restaure la position initiale.
	 * 			Permet de vérifier ce qui vient sans modifier l'état du parseur.
	 * @return	Le token suivant
	*/
	std::string					_peekToken();
	/**
	 * @brief	Parse une directive "listen" au format "IP:PORT"
	 * @details	Cette fonction extrait l'adresse IP et le port d'une chaîne formatée comme "127.0.0.1:8080".
	 * 			Elle utilise std::string::find() pour localiser le séparateur ':',
	 * 			puis std::string::substr() pour découper la chaîne en deux parties.
	 * 			Valide que ni l'IP ni le port ne sont vides, puis utilise _stringToInt()
	 * 			pour convertir et valider le port (plage 0-65535).
	 * @param	listenStr La chaine a parser
	 * @param	host Référence vers une std::string pour stocker l'adresse IP extraite
	 * @param	port Référence vers un int pour stocker le port converti et validé
	 * @throw	ConfigParserE si le format est invalide:
	 * 			- Si ':' n'est pas trouvé (format incorrect)
	 * 			- Si l'IP est vide (avant ':')
	 * 			- Si le port est vide (après ':')
	 * 			- Si le port n'est pas numérique ou hors plage (via _stringToInt())
	 * @note	Cette fonction est appelée par _parseServerDirective() lors du traitement
	 * 			de la directive "listen"
	*/
	void						_parseListenDirective(const std::string &listenStr, std::string &host, int &port);
	/**
	 * @brief	Parse une liste de methodes HTTP(ex "GET POST DELETE")
	 * @details	Lit les tokens jusqu'à trouver un point-virgule (';').
	 * 			Chaque token entre les directives est considéré comme une méthode HTTP.
	 * 			Valide que:
	 * 			- Aucun symbole spécial ({, }) n'apparaît dans la liste
	 * 			- La liste n'est pas vide (au moins une méthode requise)
	 * 			- Le fichier ne se termine pas avant la fin de la liste (EOF inattendu)
	 * @return	Un std::vector<std::string> contenant toutes les méthodes listées
	 * @throw	ConfigParserE si:
	 * 			- La liste est vide
	 * 			- Un symbole invalide est rencontré
	 * 			- EOF atteint avant de trouver ';'
	 * @note	Exemple de config: "allowed_methods GET POST DELETE;"
	 * 			Retourne: ["GET", "POST", "DELETE"]
	*/
	std::vector<std::string>	_parseMethodsList();
	/**
	 * @brief	Parse une directive au niveau serveur (bloc "server {...}")
	 * @details	Traite les directives suivantes selon la clé reçue :
	 * 			- "listen": Appelle _parseListenDirective() pour extraire IP et port
	 * 			- "server_name": Lit tous les domaines jusqu'à ';' dans serverNames vector
	 * 			- "max_body_size": Convertit la taille avec _stringToInt() en octets
	 * 			- "root": Extrait le chemin racine du serveur
	 * 			- "error_page": Lit le code erreur, puis le chemin du fichier d'erreur personnalisé
	 * 			Chaque directive doit se terminer par ';'. Cette fonction est appelée depuis
	 * 			_parseServerBlock() lors du traitement du contenu d'un bloc server.
	 * @param	key La clé de la directive à traiter (ex: "listen", "root", "max_body_size")
	 * @param	config Référence vers la structure ServerConfig à remplir avec les valeurs parsées
	 * @throw	ConfigParserE si:
	 * 			- La directive est inconnue
	 * 			- Un argument obligatoire manque ou est vide
	 * 			- Le ';' de fin manque
	 * 			- Une valeur numérique est invalide (via _stringToInt())
	 * 			- EOF atteint inopinément
	 * @note	Ordre de lecture: token (clé) → arguments → ';'
	 * 		Exemple: "listen 127.0.0.1:8080;"
	 * 		où "listen" est passé comme key
	*/
	void						_parseServerDirective(const std::string &key, ServerConfig &config);
	/**
	 * @brief	Parse une directive au niveau location (bloc "location /path {...}")
	 * @details	Traite les directives suivantes selon la clé reçue :
	 * 			- "allowed_methods": Appelle _parseMethodsList() pour extraire les méthodes HTTP
	 * 			- "root": Extrait le chemin racine spécifique à cette location
	 * 			- "index": Lit le fichier par défaut pour les répertoires
	 * 			- "autoindex": Convertit avec _stringToBool() pour activer/désactiver le listing
	 * 			- "redirect_url": Extrait l'URL cible pour les redirections (301)
	 * 			- "allow_upload": Convertit avec _stringToBool() pour autoriser/interdire uploads
	 * 			- "upload_store": Extrait le dossier de destination des uploads
	 * 			- "cgi_extension": Lit l'extension (.py, .php) et le chemin de l'exécutable CGI
	 * 			Chaque directive doit se terminer par ';'. Cette fonction est appelée depuis
	 * 			_parseLocationBlock() lors du traitement du contenu d'un bloc location.
	 * @param	key La clé de la directive à traiter (ex: "allowed_methods", "root", "cgi_extension")
	 * @param	location Référence vers la structure LocationConfig à remplir avec les valeurs parsées
	 * @throw	ConfigParserE si:
	 * 			- La directive est inconnue
	 * 			- Un argument obligatoire manque ou est vide
	 * 			- Le ';' de fin manque
	 * 			- Une valeur booléenne est invalide (via _stringToBool())
	 * 			- EOF atteint inopinément
	 * @note	Ordre de lecture: token (clé) → arguments → ';'
	 * 		Exemple: "cgi_extension .py /usr/bin/python3;"
	 * 		où "cgi_extension" est passé comme key
	*/
	void						_parseLocationDirective(const std::string &key, LocationConfig &location);
	/**
	 * @brief	Parse un bloc "location /path {...}" complet
	 * @details	Lit d'abord le chemin de la location (ex: "/api", "/uploads").
	 * 			Valide la présence du '{' d'ouverture.
	 * 			Puis boucle sur les tokens en utilisant _peekToken() pour vérifier le '}' sans le consommer.
	 * 			Chaque token de directive est passé à _parseLocationDirective().
	 * 			Consomme finalement le '}' de fermeture.
	 * @return	Une structure LocationConfig avec tous les champs parsés
	 * @throw	ConfigParserE si:
	 * 			- Le chemin est manquant ou vide
	 * 			- Le '{' d'ouverture manque
	 * 			- EOF atteint avant de trouver '}'
	 * 			- Une directive invalide est rencontrée (via _parseLocationDirective())
	 * @note	Cette fonction est appelée depuis _parseServerBlock()
	 * 		pour chaque bloc location imbriqué dans un bloc server
	*/
	LocationConfig				_parseLocationBlock();
	/**
	 * @brief	Parse un bloc "server {...}" complet
	 * @details	Valide la présence du '{' d'ouverture.
	 * 			Initialise les valeurs par défaut (port=0, maxBodySize=0).
	 * 			Puis boucle en utilisant _peekToken() pour vérifier le '}' sans le consommer:
	 * 			- Si le token est "location": appelle _parseLocationBlock() et ajoute à locations vector
	 * 			- Sinon: appelle _parseServerDirective() pour traiter les directives serveur
	 * 			Consomme finalement le '}' de fermeture.
	 * @return	Une structure ServerConfig avec tous les champs parsés et locations imbriquées
	 * @throw	ConfigParserE si:
	 * 			- Le '{' d'ouverture manque
	 * 			- EOF atteint avant de trouver '}'
	 * 			- Une directive invalide est rencontrée (via _parseServerDirective())
	 * 			- Un bloc location invalide est rencontré (via _parseLocationBlock())
	 * @note	Cette fonction est appelée depuis parse() pour chaque bloc server
	 * 		trouvé dans le fichier de configuration
	*/
	ServerConfig				_parseServerBlock();
};
