#include "inc/server.hpp"
#include "inc/SocketServer.hpp"
#include <iostream>
#include <csignal>
#include "inc/Config.hpp"

int main(int argc, char **argv) {
	signal(SIGPIPE, SIG_IGN);
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return (1);
	}
	std::string configPath = argv[1];

	try {
		ConfigParser parser;
		std::vector<ServerConfig> servers = parser.parse(configPath);
		if (servers.empty()) {
			std::cerr << "✗ Error: No servers configured in " << configPath << std::endl;
			return (1);
		}

		std::cout << "✓ Loaded " << servers.size() << " server(s) from " << configPath << std::endl;

		server webServer(servers);
		std::cout << "✓ Server setup successful. Starting..." << std::endl;
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


