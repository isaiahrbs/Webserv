#include "SocketServer.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>

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

SocketClient*	SocketServer::acceptClient() {
	struct sockaddr_in  client_addr;
	socklen_t           addrlen = sizeof(client_addr);
	
	int new_socket_fd = accept(_fd, (struct sockaddr *)&client_addr, &addrlen);
	if (new_socket_fd < 0) {
			// En mode non-bloquant, accept peut retourner -1 si aucune connexion n'est en attente.
			// On ne lance pas d'exception ici, on retourne simplement nullptr.
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return (NULL);
		throw socketException("Error: accept failed");
	}

	std::cout << "New connection accepted on fd " << new_socket_fd << std::endl;
	return (new SocketClient(new_socket_fd, client_addr));
}
