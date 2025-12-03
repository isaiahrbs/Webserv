/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASocket.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irobinso <irobinso@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:20:34 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/03 15:55:12 by irobinso         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma	once

#include <iostream>
#include <string>
#include <exception>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class	ASocket {

protected:
	int					_fd;
	struct	sockaddr_in	_addr;

public:
	ASocket(int port);
	virtual	~ASocket();

	virtual void	create() = 0;
	virtual void	setNonBlocking();
	int				getFd() const;
	sockaddr_in		getSockaddr() const;
	std::string		getIp() const;

	void			closeSocket();
	bool			isOpen() const;

	class	socketException : public std::exception {
		private:
			std::string	_msg;
		public:
		
			// if no custom message, default message
			socketException();
			
			// for custom error message
			socketException(const std::string &msg);

			// returns error message from constructor
			virtual const char*	what() const throw();

			// obliger d'ajouter lui car std::exception a un destructor throw()
			virtual ~socketException() throw();
	};
};
