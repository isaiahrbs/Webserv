/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/07 17:55:27 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/21 14:28:27 by dinguyen         ###   ########.fr       */
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

struct	LocationConfig {
	std::string							path;
	std::string							root;
	std::vector<std::string>			allowedMethods;
	std::string							index;
	bool								autoIndex;
	std::string							redirectUrl;
	bool								allowUpload;
	std::string							uploadStore;
	std::map<std::string, std::string>	cgiHandlers;
};

struct	ServerConfig {
	int							port;
	std::string					host;
	std::string					root;
	long						maxBodySize;
	std::map<int, std::string>	errorPages;
	std::vector<LocationConfig>	locations;
	std::vector<std::string>	serverNames;
};

class	ConfigParser {

public:
	ConfigParser();
	~ConfigParser();

	std::vector<ServerConfig>	parse(const std::string &filepath);

private:
	std::string					_fileContent;
	size_t						_position;
	int							_lineNumber;

	void						_readFile(const std::string &filepath);
	void						_skipSpacesAndC();
	std::string					_formatErrorMsg(const std::string &msg);
	int							_stringToInt(const std::string &str);
	bool						_stringToBool(const std::string &str);
	std::string					_readToken();
	std::string					_peekToken();
	void						_parseListenDirective(const std::string &listenStr, std::string &host, int &port);
	std::vector<std::string>	_parseMethodsList();
	void						_parseServerDirective(const std::string &key, ServerConfig &config);
	void						_parseLocationDirective(const std::string &key, LocationConfig &location);
	LocationConfig				_parseLocationBlock();
	ServerConfig				_parseServerBlock();
};
