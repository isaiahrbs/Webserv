#pragma once

#include <iostream>
#include "SocketServer.hpp"

class server
{
private:
	int _port;
	int _maxUsers;
	SocketServer* _listeningSocket;

public:
	// ===== Constructors =====
	server(int port);
	~server();

	// ===== Functions =====
	int	getPort();
	
	void setup_socket();
};
