//
// See remote_server.h
//

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "remote_server.h"

using namespace pplay;

namespace {
    const size_t MAX_CLIENTS = 8;
    const size_t MAX_REQUEST = 8192;
    const int CLIENT_TIMEOUT_MS = 5000;

#ifdef MSG_DONTWAIT
    const int DONTWAIT = MSG_DONTWAIT;
#else
    const int DONTWAIT = 0;
#endif
#ifdef MSG_NOSIGNAL
    const int SEND_FLAGS = DONTWAIT | MSG_NOSIGNAL;
#else
    const int SEND_FLAGS = DONTWAIT;
#endif
    const int RECV_FLAGS = DONTWAIT;

    bool wouldBlock() {
        return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR;
    }
}

RemoteServer::~RemoteServer() {
    stop();
}

bool RemoteServer::start(int p) {
    stop();

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("RemoteServer: socket() failed (errno %i)\n", errno);
        return false;
    }
    if (fd >= FD_SETSIZE) {
        close(fd);
        return false;
    }

    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short) p);

    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0 || listen(fd, 8) < 0) {
        printf("RemoteServer: cannot listen on port %i (errno %i)\n", p, errno);
        close(fd);
        return false;
    }

    // port 0 = the system picks one (tests)
    if (p == 0) {
        socklen_t len = sizeof(addr);
        if (getsockname(fd, (struct sockaddr *) &addr, &len) == 0) {
            p = ntohs(addr.sin_port);
        }
    }

    listenFd = fd;
    port = p;
    printf("RemoteServer: listening on port %i\n", port);
    return true;
}

void RemoteServer::stop() {
    for (auto &c: clients) {
        close(c.fd);
    }
    clients.clear();
    if (listenFd >= 0) {
        close(listenFd);
        listenFd = -1;
    }
}

void RemoteServer::drop(size_t index) {
    close(clients[index].fd);
    clients.erase(clients.begin() + (long) index);
}

void RemoteServer::answer(Client &client, const std::string &stateJson, std::vector<RemoteCommand> &commands) {
    std::string method, path;
    client.answered = true;

    if (!RemoteProtocol::parseRequestLine(client.in, method, path)) {
        client.out = RemoteProtocol::response(400, "text/plain", "bad request\n");
    } else if (path.compare(0, 5, "/cmd/") == 0) {
        if (method != "POST") {
            client.out = RemoteProtocol::response(405, "text/plain", "use POST\n");
        } else {
            RemoteCommand cmd = RemoteProtocol::parseCommand(path);
            if (cmd.type == RemoteCommand::Type::None) {
                client.out = RemoteProtocol::response(400, "text/plain", "unknown command\n");
            } else {
                commands.push_back(cmd);
                client.out = RemoteProtocol::response(200, "text/plain", "ok\n");
            }
        }
    } else if (method == "GET" && (path == "/" || path == "/index.html")) {
        client.out = RemoteProtocol::response(200, "text/html; charset=utf-8", RemoteProtocol::page());
    } else if (method == "GET" && path == "/state") {
        client.out = RemoteProtocol::response(200, "application/json", stateJson);
    } else {
        client.out = RemoteProtocol::response(404, "text/plain", "not found\n");
    }
}

void RemoteServer::service(const std::string &stateJson, std::vector<RemoteCommand> &commands) {
    if (listenFd < 0) {
        return;
    }

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(listenFd, &rfds);
    int maxFd = listenFd;
    for (auto &c: clients) {
        if (!c.answered) {
            FD_SET(c.fd, &rfds);
            if (c.fd > maxFd) maxFd = c.fd;
        }
    }

    struct timeval tv = {0, 0};
    int ready = select(maxFd + 1, &rfds, nullptr, nullptr, &tv);

    if (ready > 0) {
        // new connections (select says accept() will not block)
        if (FD_ISSET(listenFd, &rfds)) {
            int fd = accept(listenFd, nullptr, nullptr);
            if (fd >= 0) {
                if (fd >= FD_SETSIZE || clients.size() >= MAX_CLIENTS) {
                    close(fd);
                } else {
#ifdef SO_NOSIGPIPE
                    int yes = 1;
                    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *) &yes, sizeof(yes));
#endif
                    Client c;
                    c.fd = fd;
                    clients.push_back(c);
                }
            }
        }

        // requests
        for (size_t i = 0; i < clients.size();) {
            Client &c = clients[i];
            if (c.answered || !FD_ISSET(c.fd, &rfds)) {
                i++;
                continue;
            }
            char buf[2048];
            ssize_t n = recv(c.fd, buf, sizeof(buf), RECV_FLAGS);
            if (n > 0) {
                c.in.append(buf, (size_t) n);
                if (c.in.find("\r\n\r\n") != std::string::npos || c.in.find("\n\n") != std::string::npos) {
                    answer(c, stateJson, commands);
                } else if (c.in.size() > MAX_REQUEST) {
                    drop(i);
                    continue;
                }
            } else if (n == 0 || !wouldBlock()) {
                drop(i);
                continue;
            }
            i++;
        }
    }

    // answers and timeouts
    auto now = std::chrono::steady_clock::now();
    for (size_t i = 0; i < clients.size();) {
        Client &c = clients[i];
        if (c.answered) {
            ssize_t n = send(c.fd, c.out.data() + c.sent, c.out.size() - c.sent, SEND_FLAGS);
            if (n > 0) {
                c.sent += (size_t) n;
            } else if (n < 0 && !wouldBlock()) {
                drop(i);
                continue;
            }
            if (c.sent >= c.out.size()) {
                drop(i);
                continue;
            }
        }
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - c.start).count() > CLIENT_TIMEOUT_MS) {
            drop(i);
            continue;
        }
        i++;
    }
}

std::string RemoteServer::localAddress() {
    // connecting an UDP socket sends nothing: it only makes the system pick the interface
    // that would be used to reach the outside, whose address is the one the phone needs
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return "";
    }

    std::string out;
    struct sockaddr_in remote;
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(9);
    remote.sin_addr.s_addr = inet_addr("192.0.2.1");

    if (connect(fd, (struct sockaddr *) &remote, sizeof(remote)) == 0) {
        struct sockaddr_in local;
        socklen_t len = sizeof(local);
        if (getsockname(fd, (struct sockaddr *) &local, &len) == 0 && local.sin_addr.s_addr != 0) {
            // no inet_ntop/inet_ntoa: the console libc is minimal
            const unsigned char *b = (const unsigned char *) &local.sin_addr.s_addr;
            char buf[32];
            snprintf(buf, sizeof(buf), "%u.%u.%u.%u", b[0], b[1], b[2], b[3]);
            out = buf;
        }
    }
    close(fd);
    return out;
}
