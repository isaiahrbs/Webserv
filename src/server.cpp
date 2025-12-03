#include "server.hpp"
#include "SocketServer.hpp"

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

void	server::setup_socket() {
	_listeningSocket->create();
	_listeningSocket->setNonBlocking();
	_listeningSocket->bindSocket();
	_listeningSocket->listenSocket();
}