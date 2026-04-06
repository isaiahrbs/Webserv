#include "SocketServer.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>

SocketServer::SocketServer(int port, const std::string& host, int maxUsers) : ASocket(port, host) {
	_fd = -1;
	_port = port;
	_maxUsers = maxUsers;
}

SocketServer::~SocketServer() {
	// ASocket::~ASocket() appelle closeSocket() qui gère la fermeture
}

void	SocketServer::create() {
		_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (_fd < 0)
			throw socketException("Error: Socket creation");
}

void	SocketServer::setNonBlocking() {
	int opt = 1;
		if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			throw socketException("Error: setsockopt");

	int flags = fcntl(_fd, F_GETFL, 0);
		if (flags == -1 || fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
			throw socketException("Error: fcntl");
}

void	SocketServer::bindSocket() {
	if (bind(_fd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0)
		throw socketException("Error: bind");
}

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
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return (NULL);
		throw socketException("Error: accept failed");
	}

	std::cout << "New connection accepted on fd " << new_socket_fd << std::endl;
	return (new SocketClient(new_socket_fd, client_addr));
}
