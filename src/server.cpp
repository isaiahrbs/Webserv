#include "server.hpp"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

server::server(int port) : _port(port), _listen_fd(-1) {};

// en ez il setup la connection avec la personne
// ta vrmt cru que j'allais deviner ca sans l'ia bro?!
bool server::_setup_socket() {
    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listen_fd < 0)
    { 
        perror("socket");
        return false;
    }
    int opt = 1;
    if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(_listen_fd);
        _listen_fd = -1;
        return false;
    }

    int flags = fcntl(_listen_fd, F_GETFL, 0);
    if (flags == -1 || fcntl(_listen_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        close(_listen_fd);
        _listen_fd = -1;
        return false;
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);
    if (bind(_listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(_listen_fd);
        _listen_fd = -1;
        return false;
    }
    if (listen(_listen_fd, 16) < 0)
    {
        perror("listen");
        close(_listen_fd);
        _listen_fd = -1;
        return false;
    }
    return true;
}

bool server::init()
{
    return _setup_socket();
}

void server::run_one_connexion()
{
    if (_listen_fd < 0)
    {
        std::cerr << "Server not initialized" << std::endl;
        return;
    }
    std::cout << "Listening on port " << _port << " (waiting for a single connection)..." << std::endl;

    int client = accept(_listen_fd, NULL, NULL);
    if (client < 0)
    {
        perror("accept");
        return;
    }
    const char* body = "Hello World\n";
    char resp[256];
    int n = std::snprintf(resp, sizeof(resp),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        std::strlen(body), body);

    ssize_t sent_total = 0;
    while (sent_total < n) {
        ssize_t s = send(client, resp + sent_total, n - sent_total, 0);
        if (s < 0) {
            perror("send");
            break;
        }
        sent_total += s;
    }

    ::close(client);
    std::cout << "Response sent, connection closed.\n";
}

/*
Comment rendre le code I/O asynchrone?
L'ia ma sortis ce code la que je n'ai pas encore regarder la je cherche just a faire un simple server

// ...existing code...
#include <fcntl.h> // <- ajouté
// ...existing code...

bool server::_setup_socket() {
    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listen_fd < 0) { perror("socket"); return false; }

    int opt = 1;
    if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(_listen_fd);
        return false;
    }

    // rendre le socket non-blocking pour I/O asynchrone
    int flags = fcntl(_listen_fd, F_GETFL, 0);
    if (flags == -1) { perror("fcntl(F_GETFL)"); close(_listen_fd); return false; }
    if (fcntl(_listen_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL)");
        close(_listen_fd);
        return false;
    }
    // ...existing code...
    struct sockaddr_in addr;
    // ...existing code...
}
// ...existing code...

*/

//****************************************************************** */

/*
CHATGPT MA SORTIS CA JE SAUVEGARDE OCAOU C INTERESSANT

// Exemple d'usage robuste (pas de filepath)
int fd = socket(AF_INET, SOCK_STREAM, 0);
if (fd < 0) { perror("socket"); return false; }

int opt = 1;
if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt");
    close(fd);
    return false;
}

// rendre non-blocking (optionnel pour I/O asynchrone) 
int flags = fcntl(fd, F_GETFL, 0);
if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl");
    close(fd);
    return false;
}

// préparer sockaddr_in 
struct sockaddr_in addr;
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY;
addr.sin_port = htons(port);

// bind 
if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close(fd);
    return false;
}

// listen 
if (listen(fd, 16) < 0) {
    perror("listen");
    close(fd);
    return false;
}
*/

server::~server() {
    std::cout << "Destructor called" << std::endl;
}