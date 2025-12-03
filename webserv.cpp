#include "inc/server.hpp"
#include "inc/socket.hpp"

int main(int argc, char **argv) {
	server Server(8080);// on va changer a le port donner dans le fichier

	//parse_input(argv); fais ca dimicouille
	(void)argc;
	(void)argv;

	try {
		Server.setup_socket();
		std::cout << "Passed all tests" << std::endl;
	}
	catch (std::exception& e) {

	}
}