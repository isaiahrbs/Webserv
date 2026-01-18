#include "inc/server.hpp"
#include "inc/SocketServer.hpp"
#include <iostream>
#include "inc/Config.hpp"


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


//		TEST MAIN PARSER
/*
int	main(void) {
	try {
		ConfigParser	parser;
		std::vector<ServerConfig>	servers = parser.parse("config/server.conf");

		std::cout << "✓ Configuration parsed successfully!" << std::endl;
		std::cout << "Found " << servers.size() << " server(s)" << std::endl << std::endl;

		for (size_t i = 0; i < servers.size(); i++) {
			std::cout << "=== SERVER " << (i + 1) << " ===" << std::endl;
			std::cout << "  Port: " << servers[i].port << std::endl;
			std::cout << "  Host: " << servers[i].host << std::endl;
			std::cout << "  Root: " << servers[i].root << std::endl;
			std::cout << "  Max Body Size: " << servers[i].maxBodySize << std::endl;
			std::cout << "  Server Names: ";
			for (size_t j = 0; j < servers[i].serverNames.size(); j++) {
				std::cout << servers[i].serverNames[j];
				if (j < servers[i].serverNames.size() - 1)
					std::cout << ", ";
			}
			std::cout << std::endl;

			std::cout << "  Error Pages:" << std::endl;
			for (std::map<int, std::string>::iterator it = servers[i].errorPages.begin();
					it != servers[i].errorPages.end(); ++it) {
				std::cout << "    " << it->first << " -> " << it->second << std::endl;
			}

			std::cout << "  Locations: " << servers[i].locations.size() << std::endl;
			for (size_t j = 0; j < servers[i].locations.size(); j++) {
				std::cout << "    [" << j << "] Path: " << servers[i].locations[j].path << std::endl;
				std::cout << "        Root: " << servers[i].locations[j].root << std::endl;
				std::cout << "        Index: " << servers[i].locations[j].index << std::endl;
				std::cout << "        AutoIndex: " << (servers[i].locations[j].autoIndex ? "on" : "off") << std::endl;
				std::cout << "        Allow Upload: " << (servers[i].locations[j].allowUpload ? "on" : "off") << std::endl;
				std::cout << "        Allowed Methods: ";
				for (size_t k = 0; k < servers[i].locations[j].allowedMethods.size(); k++) {
					std::cout << servers[i].locations[j].allowedMethods[k];
					if (k < servers[i].locations[j].allowedMethods.size() - 1)
						std::cout << ", ";
				}
				std::cout << std::endl;
				std::cout << "        Redirect URL: " << servers[i].locations[j].redirectUrl << std::endl;
				std::cout << "        Upload Store: " << servers[i].locations[j].uploadStore << std::endl;
				if (!servers[i].locations[j].cgiHandlers.empty()) {
					std::cout << "        CGI Handlers:" << std::endl;
					for (std::map<std::string, std::string>::iterator it = servers[i].locations[j].cgiHandlers.begin();
							it != servers[i].locations[j].cgiHandlers.end(); ++it) {
						std::cout << "          " << it->first << " -> " << it->second << std::endl;
					}
				}
			}
			std::cout << std::endl;
		}
	}
	catch (const std::exception &e) {
		std::cerr << "✗ Error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}

*/
