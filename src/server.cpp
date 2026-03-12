#include "../inc/server.hpp"
#include "../inc/SocketServer.hpp"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <sstream>

server::server(const std::vector<ServerConfig>& serverConfigs) : _maxUsers(1024) {
	for (size_t i = 0; i < serverConfigs.size(); ++i) {
		const ServerConfig& config = serverConfigs[i];
		try {
			/*
				il retourne le dernier objet de la std::map
				si il n'existe pas dans la liste
			*/
			if (_serverPorts.find(config.port) == _serverPorts.end()) {
				SocketServer* newServer = new SocketServer(config.port, config.host, _maxUsers);
				newServer->create();
				newServer->setNonBlocking();
				newServer->bindSocket();
				newServer->listenSocket();
				_serverPorts[config.port] = newServer;
				std::cout << "Server listening on " << config.host << ":" << config.port << std::endl;
			} else {
				std::cout << "Port " << config.port << " is already being listened to. Skipping duplicate." << std::endl;
			}
		} catch (const ASocket::socketException& e) {
			std::cerr << "Error setting up socket on port " << config.port << ": " << e.what() << std::endl;
			throw serverException(std::string("Failed to set up all listening sockets: ") + e.what());
		}
	}
}

server::~server() {
	for (std::map<int, SocketServer*>::iterator it = _serverPorts.begin(); it != _serverPorts.end(); ++it) {
		std::cout << "Closing socket on port " << it->first << std::endl;
		delete it->second;
	}
	_serverPorts.clear();

	for (std::map<int, SocketClient*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		std::cout << "Closing client with FD " << it->first << std::endl;
		close(it->first);
		delete it->second;
	}
	_clients.clear();
}

int server::getServerLimit() {
	return _maxUsers;
}



/*
	• Activity nous dis si il y a de l'activité mais pour savoir exactement ce qu'il se passe
		on utilise la fonction FD_ISSET() pour savoir exactement si jappelle acceptClient ou recvData()
*/
void	server::run() {
	fd_set	read_fds;
	int		max_fd;

	while (true) {

		// vide la liste des FDs
		FD_ZERO(&read_fds);
		max_fd = 0;

		/*
			Ajout de tous les FDs des sockets d'écoute dans read_fds et détermination du max_fd
		*/
		std::map<int, SocketServer*>::iterator it_servers;
		for (it_servers = _serverPorts.begin(); it_servers != _serverPorts.end(); ++it_servers) {
			int listening_fd = it_servers->second->getFd();
			FD_SET(listening_fd, &read_fds);
			if (listening_fd > max_fd) {
				max_fd = listening_fd;
			}
		}

		std::cout << "Looking for activity..." << std::endl;

		/*
			on ajoute tous les fds des clients dans la liste fd (read_fds) avec FD_SET
		*/
		std::map<int, SocketClient*>::iterator it_clients;
		for (it_clients = _clients.begin(); it_clients != _clients.end(); ++it_clients) {
			int client_fd = it_clients->first;
			FD_SET(client_fd, &read_fds);

			/*
				si il y a un client avec un fd plus grand que max_fd
				ca veut dire que ya un nouveau client et donc on augemente le max_fd
			*/
			if (client_fd > max_fd) {
				max_fd = client_fd;
			}
		}

		/*
			avec tous les fds ajouter dedans la liste read_fds, on passe cette liste a select().
			select() va checker et regarder ceux qui ont besoin de traitement, il garde
			que ceux qui ont besoin de traitement, le reste c'est enlever
		*/
		int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

		if (activity < 0)
			throw serverException("Activity < 0");

		/*
			FD_ISSET() est une fonction qui retourne un true or false si le fd qu'on
			check en premier argument est present dans la liste de fd en deuxieme argument

			Vérifie chaque socket d'écoute pour de nouvelles connexions
		*/
		for (it_servers = _serverPorts.begin(); it_servers != _serverPorts.end(); ++it_servers) {
			int listening_fd = it_servers->second->getFd();
			if (FD_ISSET(listening_fd, &read_fds)) {
				SocketServer* listeningSocket = it_servers->second;
				int port = it_servers->first;
				SocketClient* newClient = listeningSocket->acceptClient();
				if (newClient) {
					_clients[newClient->getFd()] = newClient;
					newClient->setNonBlocking(); // Rendre le socket client non-bloquant
					std::cout << "New client connected with FD: " << newClient->getFd() << " on port " << port << std::endl;
				}
			}
		}

		for (it_clients = _clients.begin(); it_clients != _clients.end(); /* increment handled inside */) {
			int 			client_fd = it_clients->first;
			SocketClient*	client_ptr = it_clients->second;

			if (FD_ISSET(client_fd, &read_fds)) {

				// Buffer temporaire de 4KB
				char buf[4096];
				ssize_t bytes_read = recv(client_fd, buf, sizeof(buf), 0);

				//CAS 1: DECONNEXION OU ERREUR
				if (bytes_read <= 0) {
					if (bytes_read < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
						std::cout << "!!!False error!!!" << std::endl;
						it_clients++;
						continue;
					}

					// on ferme le fd du client, supprime son objet et on le supprime de std::map
					std::cout << "Client " << client_fd << " disconnected or finished sending." << std::endl;
					close(client_fd);
					delete client_ptr;
					_clients.erase(it_clients++);
					continue;
				}

				//CAS 2: DONNEES RECU, ON LES AJOUTE AU BUFFER
				client_ptr->getRequestBuffer().append(buf, bytes_read);

				// je check ici si la requete est finis en cherchant "\r\n\r\n"
				size_t find_end = client_ptr->getRequestBuffer().find("\r\n\r\n");
				if (find_end != std::string::npos) { // je check si la request est pas finis

					std::cout << "FULL REQUEST: " << client_ptr->getRequestBuffer() << std::endl << std::endl;

					// !! ici c'est pour toi Dim, tu peux faire tes envoie de requetes
					std::string payload(buf, buf + bytes_read);
					std::cout << "Received from fd " << client_fd << ": " << payload << std::endl;

					// -- Minimal static file serving: always reply with .html --
					std::string body;
					std::ifstream file("www/website/website.html");
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
					response << "Connection: keep-alive\r\n\r\n";
					response << body;

					std::string raw = response.str();
					send(client_fd, raw.c_str(), raw.size(), 0);

					// on vide le buffer du client
					client_ptr->getRequestBuffer().clear();
				}
				else { // REQUETE INCOMPLETE
					std::cout << "user: " << client_fd << " has slow connexion, comming back to him later." << std::endl;
				}
			}
			++it_clients;
		}
	}
}

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
