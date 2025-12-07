#include "server.hpp"
#include "SocketServer.hpp"
#include <iostream>
#include <unistd.h>

server::server(int port) : _port(port), _maxUsers(16), _listeningSocket(0) {
	this->_listeningSocket = new SocketServer(_port, _maxUsers);
}

server::~server() {
	delete this->_listeningSocket;
	this->_listeningSocket = 0;
}

int	server::getPort() {
	return _port;
}

int server::getServerSize() {
	return clients.size();
}

int server::getServerLimit() {
	return _maxUsers;
}

void	server::setup_socket() {
	_listeningSocket->create();
	_listeningSocket->setNonBlocking();
	_listeningSocket->bindSocket();
	_listeningSocket->listenSocket();
}

void	server::run() {
	fd_set	read_fds;
	int		max_fd;

	while (true) {

		// met la liste a 0
		FD_ZERO(&read_fds);

		int listening_fd = this->_listeningSocket->getFd();
		FD_SET(listening_fd, &read_fds);
		max_fd = listening_fd;
		// loop through all to add them in list

		std::cout << "Looking for activity..." << std::endl;

		int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

		if (activity < 0) {
			throw serverException("Activity < 0");
			break;
		}
		if (FD_ISSET(listening_fd, &read_fds))
			std::cout << "New connection waiting!" << std::endl;
		if (clients.size() < _maxUsers) {
			//CREATE NEW SOCKET CLIENT ON HEAP
			//ADD OBJECT TO UNORDERED MAP (clients) IN SERVER

		}
		}
}

/* for (auto const& [client_fd, client_ptr]
      this->clients) {
                FD_SET(client_fd, &read_fds);
                if (client_fd > max_fd) {
                    max_fd = client_fd;
                }
            } */



server::serverException::serverException() {
	_msg = "Server Exception";
}

server::serverException::serverException(const std::string& msg) {
	_msg = msg;
}

const char* server::serverException::what() const throw() {
	return _msg.c_str();
}

server::serverException::~serverException() throw() {
	std::cout << "Server destructor called" << std::endl;
}