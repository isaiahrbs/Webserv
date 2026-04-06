#include "../inc/server.hpp"
#include "../inc/SocketServer.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <cerrno>
#include <csignal>

static volatile sig_atomic_t g_stop = 0;

void serverSigHandler(int) {
	g_stop = 1;
}

/*	============================================================================
	HELPER: vérifie si la requête HTTP dans le buffer est complète
	(headers + body entier selon Content-Length ou chunked)
	============================================================================ */

static bool isRequestComplete(const std::string& rawData)
{
	size_t headerEnd = rawData.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return (false);
	std::string headersSection = httpToLower(rawData.substr(0, headerEnd));
	size_t tePos = headersSection.find("transfer-encoding:");
	if (tePos != std::string::npos) {
		size_t lineEnd = headersSection.find('\n', tePos);
		std::string teLine = headersSection.substr(tePos,
			(lineEnd == std::string::npos ? headersSection.size() : lineEnd) - tePos);
		if (teLine.find("chunked") != std::string::npos) {
			std::string body = rawData.substr(headerEnd + 4);
			return (body.find("0\r\n\r\n") != std::string::npos);
		}
	}
	size_t clPos = headersSection.find("content-length:");
	if (clPos == std::string::npos)
		return (true);
	size_t valStart = clPos + 15;
	while (valStart < headersSection.size() && headersSection[valStart] == ' ')
		valStart++;
	size_t valEnd = headersSection.find('\n', valStart);
	if (valEnd == std::string::npos)
		return (false);
	long contentLength = atol(headersSection.substr(valStart, valEnd - valStart).c_str());
	if (contentLength <= 0)
		return (true);
	size_t bodySize = rawData.size() - (headerEnd + 4);
	return ((long)bodySize >= contentLength);
}

/*	============================================================================
	CONSTRUCTEUR / DESTRUCTEUR
	============================================================================ */

server::server(const std::vector<ServerConfig>& serverConfigs)
	: _maxUsers(1024), _engine(NULL)
{
	for (size_t i = 0; i < serverConfigs.size(); ++i) {
		const ServerConfig& config = serverConfigs[i];
		SocketServer* newServer = NULL;
		try {
			if (_serverPorts.find(config.port) == _serverPorts.end()) {
				newServer = new SocketServer(config.port, config.host, _maxUsers);
				newServer->create();
				newServer->setNonBlocking();
				newServer->bindSocket();
				newServer->listenSocket();
				_serverPorts[config.port] = newServer;
				newServer = NULL;
				std::cout << "Server listening on " << config.host
				          << ":" << config.port << std::endl;
			} else {
				std::cout << "Port " << config.port
				          << " already listened. Skipping duplicate." << std::endl;
			}
		} catch (const ASocket::socketException& e) {
			delete newServer;
			for (std::map<int, SocketServer*>::iterator it = _serverPorts.begin();
			     it != _serverPorts.end(); ++it)
				delete it->second;
			_serverPorts.clear();
			std::cerr << "Error on port " << config.port << ": " << e.what() << std::endl;
			throw serverException(std::string("Failed to set up socket: ") + e.what());
		}
	}
	_engine = new HTTPServerEngine(serverConfigs);
}

server::~server()
{
	for (std::map<int, SocketServer*>::iterator it = _serverPorts.begin();
	     it != _serverPorts.end(); ++it) {
		delete it->second;
	}
	_serverPorts.clear();

	for (std::map<int, SocketClient*>::iterator it = _clients.begin();
	     it != _clients.end(); ++it) {
		delete it->second;
	}
	_clients.clear();
	_clientPorts.clear();

	if (_engine) {
		delete _engine;
		_engine = NULL;
	}
}

int server::getServerLimit()
{
	return (_maxUsers);
}

/*	============================================================================
	BOUCLE PRINCIPALE
	Utilise select() pour read ET write, conformément au sujet :
	  - jamais de recv/send sans passer par select() d'abord
	  - pas de vérification de errno après read/write
	============================================================================ */

void server::run()
{
	signal(SIGINT, serverSigHandler);
	signal(SIGTERM, serverSigHandler);
	while (!g_stop)
	{
		fd_set read_fds, write_fds;
		int    max_fd = 0;
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		for (std::map<int, SocketServer*>::iterator it = _serverPorts.begin();
		     it != _serverPorts.end(); ++it) {
			int fd = it->second->getFd();
			FD_SET(fd, &read_fds);
			if (fd > max_fd)
				max_fd = fd;
		}
		for (std::map<int, SocketClient*>::iterator it = _clients.begin();
		     it != _clients.end(); ++it) {
			int           fd     = it->first;
			SocketClient* client = it->second;
			if (!client->getResponseBuffer().empty())
				FD_SET(fd, &write_fds);
			else
				FD_SET(fd, &read_fds);
			if (fd > max_fd)
				max_fd = fd;
		}
		int activity = select(max_fd + 1, &read_fds, &write_fds, NULL, NULL);
		if (activity < 0) {
			if (errno == EINTR)
				break;
			std::cerr << "select() error" << std::endl;
			continue;
		}
		for (std::map<int, SocketServer*>::iterator it = _serverPorts.begin();
		     it != _serverPorts.end(); ++it) {
			int listening_fd = it->second->getFd();
			if (!FD_ISSET(listening_fd, &read_fds))
				continue;
			SocketClient* newClient = it->second->acceptClient();
			if (newClient) {
				newClient->setNonBlocking();
				int clientFd = newClient->getFd();
				_clients[clientFd]     = newClient;
				_clientPorts[clientFd] = it->first;
				std::cout << "New client fd=" << clientFd
				          << " on port " << it->first << std::endl;
			}
		}
		std::vector<int> toRemove;
		for (std::map<int, SocketClient*>::iterator it = _clients.begin();
		     it != _clients.end(); ++it)
		{
			int           fd     = it->first;
			SocketClient* client = it->second;
			if (FD_ISSET(fd, &read_fds)) {
				char    buf[8192];
				ssize_t bytes_read = recv(fd, buf, sizeof(buf), 0);
				if (bytes_read <= 0) {
					toRemove.push_back(fd);
					continue;
				}
				client->getRequestBuffer().append(buf, (size_t)bytes_read);
				if (isRequestComplete(client->getRequestBuffer())) {
					int port = _clientPorts[fd];
					std::string response = _engine->processRequest(
						client->getRequestBuffer(), port);
					client->getResponseBuffer() = response;
					client->getRequestBuffer().clear();
				}
			}
			if (FD_ISSET(fd, &write_fds)) {
				std::string& resp = client->getResponseBuffer();
				if (!resp.empty()) {
					ssize_t sent = send(fd, resp.c_str(), resp.size(), 0);
					if (sent < 0) {
						toRemove.push_back(fd);
						continue;
					}
					if (sent > 0)
						resp = resp.substr((size_t)sent);
					if (resp.empty())
						toRemove.push_back(fd);
				}
			}
		}
		for (size_t i = 0; i < toRemove.size(); i++) {
			int fd = toRemove[i];
			std::map<int, SocketClient*>::iterator it = _clients.find(fd);
			if (it != _clients.end()) {
				delete it->second;
				_clients.erase(it);
				_clientPorts.erase(fd);
			}
		}
	}
}

/*	============================================================================
	EXCEPTION
	============================================================================ */

server::serverException::serverException()
{
	_msg = "Server Exception";
}

server::serverException::serverException(const std::string& msg)
{
	_msg = msg;
}

const char* server::serverException::what() const throw()
{
	return (_msg.c_str());
}

server::serverException::~serverException() throw()
{
}
