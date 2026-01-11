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

		// vide la liste des FDs
		FD_ZERO(&read_fds);

		/*		
			FD_SET permet d'ajouter un FD dans une liste de fd (fd_set)
			on ajoute le FD du serveur dedans read_fds qui va nous être utile just après
		*/
		int listening_fd = this->_listeningSocket->getFd();
		FD_SET(listening_fd, &read_fds);
		/*
			on met le max_fd = a listening_fd car listening_fd est le fd le plus grand
			pour l'instant.
			
			(default fd = fd mis par le kernel automatiquement)
			fds:
			0• stdin			| default fd
			1• stdout			| default fd
			2• stderr			| default fd
			-> 3• listening_fd

			on ajoute le listening_fd apres les autres donc c'est le plus grand
			en ce moment.
			plus tard quand les autres fds vont venir on va ajouter +1 a max_fds
			et -1 quand ils partent, max_fd c'est savoir combien ya de fd dans le server

		*/
		max_fd = listening_fd;

		std::cout << "Looking for activity..." << std::endl;

		/*
			on ajoute tous les fds dans la liste fd (read_fds) avec FD_SET
			rappelle: FD_SET permet d'ajouter des fds dans une liste d'FDs

			int client_fd = it_first c'est normal,
			le std::map clients, chaque clients a 2 choses:
				• int fd;
				• SocketClient (l'objet)
					- std::map<int, SocketClient*> clients
			on peut savoir quel objet appartient a qui, le fd est avec l'objet SocketClient
		*/
		std::map<int, SocketClient*>::iterator it;
		for (it = clients.begin(); it != clients.end(); ++it) {
			int client_fd = it->first;
			FD_SET(client_fd, &read_fds);
			
			/*
				si il y a un client avec un fd plus grand que max_fd
				ca veut dire que ya un nouveau client et donc on augemente le max_fd

				0• stdin			| default fd
				1• stdout			| default fd
				2• stderr			| default fd
				3• listening_fd
				-> 4• client_fd
			*/
			if (client_fd > max_fd) {
				max_fd = client_fd;// augemente la limite si ya des fds
			}
		}

		/*
			avec tous les fds ajouter dedans la liste read_fds, on passe cette liste a select().
			select() va checker et regarder ceux qui ont besoin de traitement, il garde
			que ceux qui ont besoin de traitement, le reste c'est enlever
		*/
		int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

		if (activity < 0) {
			throw serverException("Activity < 0");
			break;
		}

		/*
			FD_ISSET() est une fonction qui retourne un true or false si le fd qu'on
			check en premier argument est present dans la liste de fd en deuxieme argument

			listening_fd est le fd du server, si il a besoin d'être traiter c'est que quelqu'un
			veut se connecter a lui et donc il faut accepter le nouveau client
		*/
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

				// Buffer temporaire de 4KB
				char buf[4096];

				/*
					recv() = Lit les données du client depuis un fd

					recv() sans flags c'est égal a read(). C'est vraiment la meme chose
					et dans ce cas la ya pas de flags donc c'est du coup un read()

				*/
				ssize_t bytes_read = recv(client_fd, buf, sizeof(buf), 0);
				
				//CAS 1: DECONNEXION OU ERREUR
				/*
					recv va retourner un chiffre à bytes_read
					c'est obliger qu'il retourne quelquechose au dessus de 0
					car select nous donne que les fd qui ont besoin de traitement
					si elle nous retourne 0 ou <, ca veut dire que le user a deconnecter
					ou qu'il y a erreur
				*/
				if (bytes_read <= 0) {
					/*
						il peut y arriver des cas ultra rare ou il donne des false negatifs.
						si on appelle recv() alors qu'il n'y a aucune information a lire,
						on dit de continuer et de pas interompre la connexion avec.
						le prochain loop il va reussir a le gerer lerreur

						Signification : Les deux signifient exactement la même chose : "L'opération ne peut pas être faite maintenant, veuillez réessayer plus tard."
       						EAGAIN = "Try again" (Réessaie).
       						EWOULDBLOCK = "The operation would block" (L'opération devrait bloquer).
					*/
					if (bytes_read < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
						std::cout << "!!!False error!!!" << std::endl;
						it++;
						continue;
					}
					
					// on ferme le fd du client, supprime son objet et on le supprime de std::map
					std::cout << "Client " << client_fd << " disconnected or error." << std::endl;
					close(client_fd);
					delete client_ptr;
					clients.erase(it++);
					continue;
				}
 
				//CAS 2: DONNEES RECU, ON LES AJOUTE AU BUFFER
				/*
					chaque client a son buffer personnel.
					J'ajoute les bytes lu dans le buffer du client pour ensuite regarder plus tard
					si j'ai eu la requete en entier.
					je l'append dans un std::string car std::string grandit tout seul
				*/
				client_ptr->getBuffer().append(buf, bytes_read);

				// je check ici si la requete est finis en cherchant "\r\n\r\n"
				if (client_ptr->getBuffer().find("\r\n\r\n") != std::string::npos) { // je check si la request est pas finis

					std::cout << "FULL REQUEST: " << client_ptr->getBuffer() << std::endl << std::endl;

					// !! ici c'est pour toi Dim, tu peux faire tes envoie de requetes
					std::string payload(buf, buf + bytes_read);
					std::cout << "Received from fd " << client_fd << ": " << payload << std::endl;

					// -- Minimal static file serving: always reply with .html --
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

					// on vide le buffer du client
					client_ptr->getBuffer().clear();
				}
				else { // REQUETE INCOMPLETE
					std::cout << "user: " << client_fd << " has slow connexion, comming back to him later." << std::endl;
				}
			}
			++it;
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
