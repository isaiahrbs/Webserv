/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 07:40:17 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/21 11:00:24 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Config.hpp"

ConfigParser::ConfigParser() : _position(0), _lineNumber(1) {}
ConfigParser::~ConfigParser() {}

void	ConfigParser::_readFile(const std::string &filepath) {
	std::ifstream	infile(filepath.c_str());
	if (!infile.is_open())
		throw ConfigParserE("Can't open configuration file: " + filepath);
	std::string		line;
	while (std::getline(infile, line))
		_fileContent += line + "\n";
	infile.close();
	if (_fileContent.empty())
		throw	ConfigParserE("Configuration file is empty");
}

void	ConfigParser::_skipSpacesAndC() {
	while (_position < _fileContent.length()) {
		char	c = _fileContent[_position];
		if (c == ' ' || c == '\t' || c == '\n') {
			if (c == '\n')
				_lineNumber++;
			_position++;
		}
		else if (c == '#') {
			while (_position < _fileContent.length() && _fileContent[_position] != '\n')
				_position++;
		}
		else
			break ;
	}
}
