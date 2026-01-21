#include "inc/server.hpp"
#include "inc/SocketServer.hpp"
#include <iostream>
#include "inc/Config.hpp"

/*
int main(int argc, char **argv) {
	std::string configPath = "config/server.conf";
	if (argc > 1)
		configPath = argv[1];

	try {
		ConfigParser parser;
		std::vector<ServerConfig> servers = parser.parse(configPath);
		if (servers.empty()) {
			std::cerr<<"Error: No servers configured in "<<configPath<<std::endl;
			return (1);
		}
		std::cout<<"✓ Loaded "<<servers.size()<<" server(s) from "<<configPath<<std::endl;
		server webServer(servers[0].port, servers);
		webServer.setup_socket();
		std::cout << "✓ Socket setup successful on port " << servers[0].port << std::endl;
		webServer.run();
	}
	catch (const ConfigParserE &e) {
		std::cerr << "✗ Config parsing error: " << e.what() << std::endl;
		return (1);
	}
	catch (const std::exception &e) {
		std::cerr << "✗ Error: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}
*/


int main(int argc, char **argv) {
    std::string configPath = "config/server.conf";
    if (argc > 1)
        configPath = argv[1];

    try {
        ConfigParser parser;
        std::vector<ServerConfig> servers = parser.parse(configPath);
        if (servers.empty()) {
            std::cerr << "✗ Error: No servers configured in " << configPath << std::endl;
            return (1);
        }

        std::cout << "✓ Loaded " << servers.size() << " server(s) from " << configPath << std::endl;

        // Test avec le premier serveur (port 8080)
        ServerConfig testServer = servers[0];
        std::cout << "✓ Testing server on port " << testServer.port << std::endl;
        std::cout << "  Root directory: " << testServer.root << std::endl;
        std::cout << "  Max body size: " << testServer.maxBodySize << " bytes" << std::endl;

        // Créer le serveur avec un seul paramètre (port)
        server webServer(testServer.port);
        webServer.setup_socket();
        std::cout << "✓ Socket setup successful" << std::endl;
        webServer.run();
    }
    catch (const ConfigParserE &e) {
        std::cerr << "✗ Config parsing error: " << e.what() << std::endl;
        return (1);
    }
    catch (const std::exception &e) {
        std::cerr << "✗ Error: " << e.what() << std::endl;
        return (1);
    }

    return (0);
}
