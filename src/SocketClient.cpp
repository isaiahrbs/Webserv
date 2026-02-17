#include "../inc/SocketClient.hpp"
#include <unistd.h>
#include <fcntl.h>

SocketClient::SocketClient(int fd, struct sockaddr_in addr) : ASocket(0, ""), _state(IDLE) { // Initialiser _state ici
	this->_fd = fd;
	this->_addr = addr;
}

SocketClient::~SocketClient() {
	// Le destructeur de ASocket s'occupera de fermer le fd s'il est ouvert.
}

// La création est gérée par accept() dans SocketServer,
// donc cette fonction peut être vide ou lancer une exception si elle est appelée.
void SocketClient::create() {
	// Ne fait rien, le socket est déjà créé par accept()
}

// Implémentation vide pour satisfaire l'héritage,
// la non-blocking est généralement défini après accept()
void SocketClient::setNonBlocking() {
	if (this->_fd != -1) {
		int flags = fcntl(this->_fd, F_GETFL, 0);
		if (flags == -1) {
			// Gérer l'erreur, par exemple, logger ou lancer une exception
			// Pour le moment, on peut simplement retourner
			return;
		}
		fcntl(this->_fd, F_SETFL, flags | O_NONBLOCK);
	}
}

ssize_t SocketClient::sendData(const void* buf, size_t len) {
	return send(_fd, buf, len, 0);
}

ssize_t SocketClient::recvData(void* buf, size_t len) {
	return recv(_fd, buf, len, 0);
}

bool SocketClient::isConnected() const {
	return this->isOpen();
}

std::string& SocketClient::getRequestBuffer() {
	return _requestBuffer;
}

std::string& SocketClient::getResponseBuffer() {
	return _responseBuffer;
}

// Implémentation des nouvelles méthodes
ClientState SocketClient::getState() const {
	return _state;
}

void SocketClient::setState(ClientState state) {
	_state = state;
}