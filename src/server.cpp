#include "../inc/server.hpp"
#include "../inc/SocketServer.hpp"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <sstream>

server::server(int port) : _port(port), _maxUsers(1024), _listeningSocket(0) {
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



/*
	• Activity nous dis si il y a de l'activité mais pour savoir exactement ce qu'il se passe
		on utilise la fonction FD_ISSET() pour savoir exactement si jappelle acceptClient ou recvData()
*/
void	server::run() {
	fd_set	read_fds;
	int		max_fd;

	while (true) {

		// met la liste a 0
		FD_ZERO(&read_fds);

		// commence a surveiller si des connection veulent communiquer a ce fd
		int listening_fd = this->_listeningSocket->getFd();
		FD_SET(listening_fd, &read_fds);
		max_fd = listening_fd;
		// loop through all to add them in list

		std::cout << "Looking for activity..." << std::endl;

		// j'ajoute tout les fds dans le read_fds list
		std::map<int, SocketClient*>::iterator it;
		for (it = clients.begin(); it != clients.end(); ++it) {
			int client_fd = it->first;
			FD_SET(client_fd, &read_fds);
			if (client_fd > max_fd) {
				max_fd = client_fd;
			}
		}

		int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

		if (activity < 0) {
			throw serverException("Activity < 0");
			break;
		}

		// accepte les nouvelles connecxions quand FD_ISSET est vrai
		if (FD_ISSET(listening_fd, &read_fds)) {
			SocketClient* newClient = _listeningSocket->acceptClient();
			if (newClient) {
				clients[newClient->getFd()] = newClient;
				std::cout << "New client connected with FD: " << newClient->getFd() << std::endl;
			}
		}

		for (it = clients.begin(); it != clients.end(); /* increment handled inside */) {
			int 			client_fd = it->first;
			SocketClient*	client_ptr = it->second;

			if (FD_ISSET(client_fd, &read_fds)) {
				char buf[4096];
				ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
				if (n == 0) {
					close(client_fd);
					delete client_ptr;
					std::map<int, SocketClient*>::iterator next = it;
					++next;
					clients.erase(it);
					it = next;
					continue;
				}
				if (n < 0) {
					if (errno == EAGAIN || errno == EWOULDBLOCK) {
						++it;
						continue;
					}
					close(client_fd);
					delete client_ptr;
					std::map<int, SocketClient*>::iterator next = it;
					++next;
					clients.erase(it);
					it = next;
					continue;
				}
				std::string payload(buf, buf + n);
				std::cout << "Received from fd " << client_fd << ": " << payload << std::endl;

				// -- Minimal static file serving: always reply with index.html --
				std::string body;
				std::ifstream file("/home/isaiah/Downloads/random.html");
				if (file) {
					std::ostringstream ss;
					ss << file.rdbuf();
					body = ss.str();
				}
				std::ostringstream response;
				if (body.empty()) {
					body = "<html><body><h1>404 Not Found</h1></body></html>";
					response << "HTTP/1.1 404 Not Found\r\n";
				} else {
					response << "HTTP/1.1 200 OK\r\n";
				}
				response << "Content-Type: text/html; charset=UTF-8\r\n";
				response << "Content-Length: " << body.size() << "\r\n";
				response << "Connection: close\r\n\r\n";
				response << body;

				std::string raw = response.str();
				send(client_fd, raw.c_str(), raw.size(), 0);
				close(client_fd);
				delete client_ptr;
				std::map<int, SocketClient*>::iterator next = it;
				++next;
				clients.erase(it);
				it = next;
				continue;
				// end
			}
			++it;
		}

		
		
		/* if (activity > 0) {
			if (FD_ISSET(listening_fd, &read_fds)) {
				std::cout << "New connection waiting!" << std::endl;
				if ((int)clients.size() < _maxUsers) {
				//CREATE NEW SOCKET CLIENT ON HEAP
				//ADD OBJECT TO UNORDERED MAP (clients) IN SERVER
				}
				else {
					std::cout << "Server is already full" << std::endl;
				}
			}
			std::map<int, SocketClient*>::iterator it;
			for (it = clients.begin(); it != this->clients.end(); ++it) {
				int client_fd = it->first;// donne la clé (le int)
				SocketClient* client_ptr = it->second;// donne la valeur (socketclient)
				(void) client_ptr;
				if (FD_ISSET(client_fd, &read_fds)) {
					std::cout << "Client : " << client_fd << " sent a message." << std::endl;
				}
			}
		}	 */
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
