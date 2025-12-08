#include "../inc/ASocket.hpp"

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

