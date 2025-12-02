#pragma once

#include <iostream>
#include "socket.hpp"
class server
{
private:
	int _port;
	int _maxUsers;
	Socket* _listeningSocket;

public:
	// ===== Constructors =====
	server(int port);
	~server();

	// ===== Functions =====
	int	getPort();
	
	void setup_socket();
};
