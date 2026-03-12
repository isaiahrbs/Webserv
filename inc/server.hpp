#pragma once

#include <iostream>
#include <map>
#include "Config.hpp"
#include "SocketServer.hpp"
#include "SocketClient.hpp"
#include <vector>

class server
{
private:
	int _maxUsers;
	std::map<int, SocketClient*> _clients; // les clients connecté
	std::map<int, SocketServer*> _serverPorts;

public:
	// ===== Constructors =====
	//prendre en parametre les ports mis dans le vector par le parsing
	server(const std::vector<ServerConfig>& serverConfigs);
	~server();

	// ===== Functions =====
	int getServerLimit();
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
