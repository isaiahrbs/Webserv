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

class	FileHandler {

	public:
		static bool			exists(const std::string &path);
		static bool			isDirectory(const std::string &path);
		static bool			isFile(const std::string &path);

		static std::string	getContent(const std::string &path);
		static bool			writeContent(const std::string &path, const std::string &content);
		static bool			deleteFile(const std::string &path);
		static long			getFileSize(const std::string &path);
		static std::string	getFileExtension(const std::string &path);

		static std::string	generateDirectoryListing(const std::string &dirPath,
													const std::string &uri);
		static std::string	findIndexFile(const std::string &dirPath,
										const std::vector<std::string> &indexFiles);

		static std::string	buildFilePath(const std::string &root, const std::string &uri);
		static std::string	extractFileName(const std::string &uri);
		static std::string	normalizePath(const std::string &path);
};

#endif
