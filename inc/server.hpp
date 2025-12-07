#pragma once

#include <iostream>
#include <unordered_map>
#include "SocketServer.hpp"
#include "SocketClient.hpp"

class server
{
private:
	int _port;
	int _maxUsers;
	SocketServer* _listeningSocket;
	std::unordered_map<int, SocketClient*> clients; // les clients connecté

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
};
