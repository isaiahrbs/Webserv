/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 10:11:35 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/21 13:01:37 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Exceptions.hpp"

ConfigParserE::ConfigParserE(const std::string &msg) : _message(msg) {}
const char*	ConfigParserE::what() const throw() {
	return (_message.c_str());
}
ConfigParserE::~ConfigParserE() throw() {}

RequestE::RequestE(const std::string &msg) : _message(msg) {}
const char*	RequestE::what() const throw() {
	return (_message.c_str());
}
RequestE::~RequestE() throw() {}

ResponseE::ResponseE(const std::string &msg) : _message(msg) {}
const char*	ResponseE::what() const throw() {
	return (_message.c_str());
}
ResponseE::~ResponseE() throw() {}
