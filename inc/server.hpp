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
	std::map<SocketClient, int> Clients;

public:
	// ===== Constructors =====
	server(int port);
	~server();

	// ===== Functions =====
	int	getPort();
	
	void setup_socket();
	void run();
};
