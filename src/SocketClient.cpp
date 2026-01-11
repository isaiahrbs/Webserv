#include "../inc/SocketClient.hpp"
#include <unistd.h>

SocketClient::SocketClient(int fd, struct sockaddr_in addr) : ASocket(0) {
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
    // Peut être implémenté si nécessaire
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

std::string& SocketClient::getBuffer() {
	return buffer;
}
