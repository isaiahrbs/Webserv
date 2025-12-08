#pragma once

#include <iostream>
#include <map>
#include "SocketServer.hpp"
#include "SocketClient.hpp"

class server
{
private:
	int _port;
	int _maxUsers;
	SocketServer* _listeningSocket;
	std::map<int, SocketClient*> clients; // les clients connecté

public:
	// ===== Constructors =====
	server(int port);
	~server();

	// ===== Functions =====
	int	getPort();
	int getServerSize();
	int getServerLimit();
	
	void setup_socket();
	void run();

	class	serverException : public std::exception {
		private:
			std::string	_msg;
		public:
		
			// if no custom message, default message
			serverException();
			
			// for custom error message
			serverException(const std::string &msg);

			// returns error message from constructor
			virtual const char*	what() const throw();

			// obliger d'ajouter lui car std::exception a un destructor throw()
			virtual ~serverException() throw();
	};
};
