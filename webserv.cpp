#include <iostream>
#include <cstdlib>
#include "server.hpp"

int main(int argc, char** argv)
{
	int port = 8080;

	if (argc != 2)
	{
		std::cout << "How to use: " << argv[0] << " <filename>" << std::endl;
		return -1;
	}
	port = std::atoi(argv[1]);
	server srv(port);
	if (!srv.init())
	{
		std::cerr << "Failed to initialize server on port " << port << "\n";
        return 1;
	}
	srv.run_one_connexion(); // accepte une connexion, envoie Hello World, ferme
    return 0;
}