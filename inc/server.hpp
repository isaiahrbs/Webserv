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
	
	// ce n'est pas la version definitif qu'on va utiliser de run celui la 
	void run_one_connexion();

	// run asynchrone I/O (la bonne version)
	void run();
};
