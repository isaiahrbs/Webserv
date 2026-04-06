#pragma once

#include <iostream>
#include <map>
#include <vector>
#include "Config.hpp"
#include "SocketServer.hpp"
#include "SocketClient.hpp"
#include "HTTPCommon.hpp"
#include "HTTPParser.hpp"

class server
{
private:
	int _maxUsers;
	std::map<int, SocketClient*> _clients;
	std::map<int, SocketServer*> _serverPorts;
	std::map<int, int>           _clientPorts;
	HTTPServerEngine*            _engine;

public:
	server(const std::vector<ServerConfig>& serverConfigs);
	~server();
	int getServerLimit();
	void run();

	class	serverException : public std::exception {
		private:
			std::string	_msg;
		public:
			serverException();
			serverException(const std::string &msg);
			virtual const char*	what() const throw();
			virtual ~serverException() throw();
	};
};
