#include "socket.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>

Socket::Socket(int port) {
	_fd = -1;
	_port = port;
	_max = 16;
	memset(&addr, 0, sizeof(addr));
}

Socket::~Socket() {
	if (_fd != -1)
		close(_fd);
}

// creates a socket (the phone)
void	Socket::create() {
		_fd = socket(AF_INET, SOCK_STREAM, 0); // fd recois la clée pour modif le socket
		if (_fd < 0)
			throw socketException("Error: Socket creation");
}

// sets the type to Asynchrone I/O
void	Socket::set_non_blocking() {
	int opt = 1;
		if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			throw socketException("Error: setsockopt");

	int flags = fcntl(_fd, F_GETFL, 0);
		if (flags == -1 || fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
			throw socketException("Error: fcntl");
}

// sets the phone to be connected to 8080 at our ip address
void	Socket::bindSocket() {
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	// bind
	if (bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw socketException("Error: bind");
}

// starts waiting for calls
void	Socket::listenSocket(int max) {
	std::cout << "Starting to listen..." << std::endl;
	if (listen(_fd, max) < 0)
		throw socketException("listen");
}

void	Socket::accept_clients() {
	int	new_socket;
	int addrlen = sizeof(addr);

	new_socket = accept(_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
	if (new_socket < 0)
        throw socketException("Error: accept");
}

int Socket::get_fd() const {
	return _fd;
}

Socket::socketException::socketException() {
	_msg = "Socket Exception";
}

Socket::socketException::socketException(const std::string& msg) {
	_msg = msg;
}

const char* Socket::socketException::what() const throw() {
	return _msg.c_str();
}

Socket::socketException::~socketException() throw() {
	std::cout << "Socket destructor called" << std::endl;
}
