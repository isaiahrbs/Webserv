#pragma once

#include <iostream>

class server
{
private:
	int		_port;
	int		_listen_fd;
	bool	_setup_socket();

public:
	// ===== Constructors =====
	server(int port = 8080);
	~server();

	// ===== Functions =====
	bool init();
	void run_one_connexion();
};
