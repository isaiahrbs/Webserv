/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 10:31:14 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/19 10:40:11 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/FileHandler.hpp"

/*	============================================================================
		FILE EXISTENCE & TYPE CHECKING
	============================================================================ */

bool	FileHandler::exists(const std::string &path) {
	struct stat	statbuf;
	return (stat(path.c_str(), &statbuf) == 0);
}

bool	FileHandler::isDirectory(const std::string &path) {
	struct stat	statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return (false);
	return (S_ISDIR(statbuf.st_mode));
}

bool	FileHandler::isFile(const std::string &path) {
	struct stat	statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return (false);
	return (S_ISREG(statbuf.st_mode));
}

/*	============================================================================
		FILE CONTENT OPERATIONS
	============================================================================ */

std::string	FileHandler::getContent(const std::string &path) {
	std::ifstream		file(path.c_str(), std::ios::binary);
	std::stringstream	buffer;

	if (!file.is_open())
		return ("");
	buffer << file.rdbuf();
	file.close();
	return (buffer.str());
}

bool	FileHandler::writeContent(const std::string &path, const std::string &content) {
	std::ofstream	file(path.c_str(), std::ios::binary);

	if (!file.is_open())
		return (false);
	file.write(content.c_str(), content.length());
	file.close();
	return (true);
}

bool	FileHandler::deleteFile(const std::string &path) {
	if (!isFile(path))
		return (false);
	return (remove(path.c_str()) == 0);
}

/*	============================================================================
		FILE INFORMATION
	============================================================================ */

long	FileHandler::getFileSize(const std::string &path) {
	struct stat	statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return (-1);
	return (statbuf.st_size);
}

std::string	FileHandler::getFileExtension(const std::string &path) {
	size_t	dot_pos = path.find_last_of(".");

	if (dot_pos == std::string::npos || dot_pos == 0)
		return ("");
	return (path.substr(dot_pos));
}

/*	============================================================================
		DIRECTORY OPERATIONS
	============================================================================ */

std::string	FileHandler::generateDirectoryListing(const std::string &dirPath,
												const std::string &uri) {
	DIR				*dir;
	struct dirent	*entry;
	struct stat		statbuf;
	std::string		html;
	std::string		full_path;
	std::string		size_str;

	dir = opendir(dirPath.c_str());
	if (!dir)
		return ("");
	html = "<html>\r\n<head><title>Directory Listing</title></head>\r\n";
	html += "<body><h1>Directory: ";
	html += uri;
	html += "</h1>\r\n<table border=\"1\">\r\n";
	html += "<tr><th>Name</th><th>Type</th><th>Size</th></tr>\r\n";
	while ((entry = readdir(dir)) != NULL) {
		if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
			continue ;
		full_path = dirPath + "/" + entry->d_name;
		if (stat(full_path.c_str(), &statbuf) != 0)
			continue ;
		html += "<tr><td><a href=\"";
		html += uri;
		if (uri[uri.length() - 1] != '/')
			html += "/";
		html += entry->d_name;
		if (S_ISDIR(statbuf.st_mode))
			html += "/";
		html += "\">";
		html += entry->d_name;
		html += "</a></td>";
		html += "<td>";
		if (S_ISDIR(statbuf.st_mode))
			html += "[DIR]";
		else
			html += "FILE";
		html += "</td>";
		html += "<td>";
		if (S_ISREG(statbuf.st_mode))
			html += httpIntToString(statbuf.st_size);
		else
			html += "-";
		html += "</td></tr>\r\n";
	}
	closedir(dir);
	html += "</table>\r\n</body>\r\n</html>\r\n";
	return (html);
}

std::string	FileHandler::findIndexFile(const std::string &dirPath,
										const std::vector<std::string> &indexFiles) {
	std::string	full_path;

	for (size_t i = 0; i < indexFiles.size(); i++) {
		full_path = dirPath + "/" + indexFiles[i];
		if (isFile(full_path))
			return (full_path);
	}
	return ("");
}

/*	============================================================================
		PATH OPERATIONS
	============================================================================ */

std::string	FileHandler::buildFilePath(const std::string &root, const std::string &uri) {
	std::string	path = root;

	if (!root.empty() && root[root.length() - 1] != '/')
		path += "/";
	if (!uri.empty() && uri[0] == '/')
		path += uri.substr(1);
	else
		path += uri;
	return (path);
}

std::string	FileHandler::extractFileName(const std::string &uri) {
	size_t	last_slash = uri.find_last_of("/");

	if (last_slash == std::string::npos)
		return (uri);
	if (last_slash == uri.length() - 1)
		return ("");
	return (uri.substr(last_slash + 1));
}

std::string	FileHandler::normalizePath(const std::string &path) {
	std::string	normalized = path;

	if (normalized.find("/../") != std::string::npos)
		return ("");
	if (normalized.substr(0, 3) == "/..")
		return ("");
	size_t pos = 0;
	while ((pos = normalized.find("//", pos)) != std::string::npos) {
		normalized.erase(pos, 1);
	}
	return (normalized);
}
