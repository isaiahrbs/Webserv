#include "SocketServer.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>

SocketServer::SocketServer(int port, int maxUsers) : ASocket(port) {
	_fd = -1;
	_port = port;
	_maxUsers = maxUsers;
	memset(&_addr, 0, sizeof(_addr));
}

SocketServer::~SocketServer() {
	if (_fd != -1)
		close(_fd);
}

// creates a socket (the phone)
void	SocketServer::create() {
		_fd = socket(AF_INET, SOCK_STREAM, 0); // fd recois la clée pour modif le socket
		if (_fd < 0)
			throw socketException("Error: Socket creation");
}

// sets the type to Asynchrone I/O
void	SocketServer::setNonBlocking() {
	int opt = 1;
		if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			throw socketException("Error: setsockopt");

	int flags = fcntl(_fd, F_GETFL, 0);
		if (flags == -1 || fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
			throw socketException("Error: fcntl");
}

// sets the phone to be connected to 8080 at our ip address
void	SocketServer::bindSocket() {
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;
	_addr.sin_port = htons(_port);

	// bind
	if (bind(_fd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0)
		throw socketException("Error: bind");
}

// starts waiting for calls
void	SocketServer::listenSocket() {
	std::cout << "Starting to listen..." << std::endl;
	if (listen(_fd, _maxUsers) < 0)
		throw socketException("listen");
}

int	SocketServer::acceptClient() {
	int	new_socket;
	int addrlen = sizeof(_addr);

	new_socket = accept(_fd, (struct sockaddr *)&_addr, (socklen_t*)&addrlen);
	if (new_socket < 0)
		throw socketException("Error: accept");
	return new_socket;
}
