#include "../inc/ASocket.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

ASocket::ASocket(int port) : _fd(-1) {
	std::memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;
	_addr.sin_port = htons(port);
}

ASocket::~ASocket() {
	closeSocket();
}

void ASocket::setNonBlocking() {
	int flags = fcntl(_fd, F_GETFL, 0);
	if (flags == -1) {
		throw socketException("fcntl F_GETFL failed");
	}
	if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		throw socketException("fcntl F_SETFL O_NONBLOCK failed");
	}
}

int ASocket::getFd() const {
	return _fd;
}

sockaddr_in ASocket::getSockaddr() const {
	return _addr;
}

std::string ASocket::getIp() const {
	return std::string(inet_ntoa(_addr.sin_addr));
}

void ASocket::closeSocket() {
	if (_fd >= 0) {
		close(_fd);
		_fd = -1;
	}
}

bool ASocket::isOpen() const {
	return _fd >= 0;
}

ASocket::socketException::socketException() {
	_msg = "Socket Exception";
}

ASocket::socketException::socketException(const std::string& msg) {
	_msg = msg;
}

const char* ASocket::socketException::what() const throw() {
	return _msg.c_str();
}

ASocket::socketException::~socketException() throw() {
	std::cout << "Socket destructor called" << std::endl;
}

