#include "inc/server.hpp"
#include "inc/SocketServer.hpp"

int main(int argc, char **argv) {
	server Server(8080);// on va changer a le port donner dans le fichier

	//parse_input(argv); fais ca dimicouille
	(void)argc;
	(void)argv;

	try {
		Server.setup_socket();
		std::cout << "Socket setup pass" << std::endl;
		Server.run();
	}
	catch (std::exception& e) {

	}
}