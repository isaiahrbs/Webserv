#pragma once

#include <iostream>
#include <string>
#include <exception>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Socket
{
private:
	int _fd;
	int _port;
	int _max;
	struct sockaddr_in addr;

public:
	// ===== Constructors =====
	Socket(int port);
	~Socket();

	// ===== Functions =====
	void	create();
	void	set_non_blocking();
	void	bindSocket();
	void	listenSocket(int _max);

	void	accept_clients();


	int		get_fd() const;
	sockaddr_in	get_sockaddr() const;

	class socketException : public std::exception {
		private:
			std::string _msg;
		public:

			// if no custom message, default message
			socketException();

			// for custom error message
			socketException(const std::string& msg);

			// returns error message from constructor
			virtual const char* what() const throw();

			// obliger d'ajouter lui car std::exception a un destructor throw()
			virtual ~socketException() throw();
	};

};
