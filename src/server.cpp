#include "server.hpp"

server::server(int port) : _port(port), _maxUsers(16), _listeningSocket(0) {
	this->_listeningSocket = new Socket(this->_port);
}

server::~server() {
	delete this->_listeningSocket;
	this->_listeningSocket = 0;
}

int	server::getPort() {
	return _port;
}

void	server::setup_socket() {
	_listeningSocket->create();
	_listeningSocket->set_non_blocking();
	_listeningSocket->bindSocket();
	_listeningSocket->listenSocket(_maxUsers);
}