//
// See remote_server.h
//

#include <atomic>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "remote_server.h"

// In pplay threads and mutexes are SDL's (the ones the rest of the application already uses on the
// console); the PC tests use the C++ ones.
#ifdef __PPLAY__

#include <SDL2/SDL.h>

#else
#include <mutex>
#include <thread>
#endif

using namespace pplay;

namespace {
    const size_t MAX_CLIENTS = 8;
    const size_t MAX_REQUEST = 8192;
    const long long CLIENT_TIMEOUT_MS = 5000;
    const long long LINGER_MS = 1000;
    const long long STATE_WANTED_MS = 3000;
    const long long MIN_LOOP_MS = 20;        // never spin, whatever select() does
    const int MAX_LOG_LINES = 300;

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

    void sleepMs(long long ms) {
        if (ms <= 0) return;
#ifdef __PPLAY__
        SDL_Delay((Uint32) ms);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
    }

    struct Lock {
#ifdef __PPLAY__
        Lock() { m = SDL_CreateMutex(); }

        ~Lock() { SDL_DestroyMutex(m); }

        void lock() { SDL_LockMutex(m); }

        void unlock() { SDL_UnlockMutex(m); }

        SDL_mutex *m;
#else
        void lock() { m.lock(); }

        void unlock() { m.unlock(); }

        std::mutex m;
#endif
    };

    struct Guard {
        explicit Guard(Lock &l) : l(l) { l.lock(); }

        ~Guard() { l.unlock(); }

        Lock &l;
    };
}

struct RemoteServer::Shared {
    struct Client {
        int fd = -1;
        std::string in;
        std::string out;
        size_t sent = 0;
        bool answered = false;
        bool shut = false;          // answer sent, our side closed, waiting for the peer to finish
        long long start = 0;
        long long shutTime = 0;
    };

    // shared with the main loop
    Lock lock;
    std::vector<RemoteCommand> commands;
    std::string state = "{}";
    std::atomic<long long> stateAskedMs{0};
    std::atomic<bool> running{true};

    // thread only
    int listenFd = -1;
    std::string logPath;
    int logLines = 0;
    std::vector<Client> clients;

    void log(const char *fmt, ...) {
        char line[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(line, sizeof(line), fmt, args);
        va_end(args);

        printf("RemoteServer: %s\n", line);
        if (logPath.empty()) {
            return;
        }
        Guard g(lock); // the main thread logs too (start/stop)
        if (logLines >= MAX_LOG_LINES) {
            return;
        }
        FILE *f = fopen(logPath.c_str(), logLines == 0 ? "w" : "a");
        if (f != nullptr) {
            fprintf(f, "%s\n", line);
            fclose(f);
            logLines++;
        }
    }

    void drop(size_t index, const char *why) {
        if (why != nullptr) {
            log("client %i dropped: %s", clients[index].fd, why);
        }
        close(clients[index].fd);
        clients.erase(clients.begin() + (long) index);
    }

    void answer(Client &client) {
        std::string method, path;
        int status = 200;
        client.answered = true;

        if (!RemoteProtocol::parseRequestLine(client.in, method, path)) {
            status = 400;
            client.out = RemoteProtocol::response(400, "text/plain", "bad request\n");
        } else if (path.compare(0, 5, "/cmd/") == 0) {
            if (method != "POST") {
                status = 405;
                client.out = RemoteProtocol::response(405, "text/plain", "use POST\n");
            } else {
                RemoteCommand cmd = RemoteProtocol::parseCommand(path);
                if (cmd.type == RemoteCommand::Type::None) {
                    status = 400;
                    client.out = RemoteProtocol::response(400, "text/plain", "unknown command\n");
                } else {
                    {
                        Guard g(lock);
                        if (commands.size() < 64) {
                            commands.push_back(cmd);
                        }
                    }
                    client.out = RemoteProtocol::response(200, "text/plain", "ok\n");
                }
            }
        } else if (method == "GET" && (path == "/" || path == "/index.html")) {
            client.out = RemoteProtocol::response(200, "text/html; charset=utf-8", RemoteProtocol::page());
        } else if (method == "GET" && path == "/state") {
            std::string json;
            {
                Guard g(lock);
                json = state;
            }
            stateAskedMs = RemoteServer::nowMs();
            client.out = RemoteProtocol::response(200, "application/json", json);
        } else {
            status = 404;
            client.out = RemoteProtocol::response(404, "text/plain", "not found\n");
        }

        // /state is asked every second: do not fill the log with it
        if (path != "/state") {
            log("%s %s -> %i", method.c_str(), path.c_str(), status);
        }
    }

    // one pass: wait up to timeoutMs for activity, then accept / read / answer / time out
    void serviceOnce(int timeoutMs) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(listenFd, &rfds);
        int maxFd = listenFd;
        for (auto &c: clients) {
            // waiting for a request, or (after our answer) for the peer to close
            if (!c.answered || c.shut) {
                FD_SET(c.fd, &rfds);
                if (c.fd > maxFd) maxFd = c.fd;
            }
        }

        struct timeval tv;
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        int ready = select(maxFd + 1, &rfds, nullptr, nullptr, &tv);

        if (ready > 0) {
            // new connections (select says accept() will not block)
            if (FD_ISSET(listenFd, &rfds)) {
                int fd = accept(listenFd, nullptr, nullptr);
                if (fd >= 0) {
                    if (fd >= FD_SETSIZE || clients.size() >= MAX_CLIENTS) {
                        log("connection refused (%i clients, socket %i)", (int) clients.size(), fd);
                        close(fd);
                    } else {
#ifdef SO_NOSIGPIPE
                        int yes = 1;
                        setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *) &yes, sizeof(yes));
#endif
                        Client c;
                        c.fd = fd;
                        c.start = RemoteServer::nowMs();
                        clients.push_back(c);
                    }
                } else if (!wouldBlock()) {
                    log("accept() failed, errno %i", errno);
                }
            }

            // requests (and the end of the connections we already answered)
            for (size_t i = 0; i < clients.size();) {
                Client &c = clients[i];
                if ((c.answered && !c.shut) || !FD_ISSET(c.fd, &rfds)) {
                    i++;
                    continue;
                }
                char buf[2048];
                ssize_t n = recv(c.fd, buf, sizeof(buf), RECV_FLAGS);
                if (n > 0) {
                    if (c.shut) {
                        i++; // the phone sent more (a body): read it and ignore it
                        continue;
                    }
                    c.in.append(buf, (size_t) n);
                    if (c.in.find("\r\n\r\n") != std::string::npos || c.in.find("\n\n") != std::string::npos) {
                        answer(c);
                    } else if (c.in.size() > MAX_REQUEST) {
                        drop(i, "request too big");
                        continue;
                    }
                } else if (n == 0) {
                    // the phone closed its side: nothing more to say (or to read)
                    drop(i, c.answered ? nullptr : "closed before sending a request");
                    continue;
                } else if (!wouldBlock()) {
                    drop(i, "recv error");
                    continue;
                }
                i++;
            }
        }

        // answers and timeouts
        const long long now = RemoteServer::nowMs();
        for (size_t i = 0; i < clients.size();) {
            Client &c = clients[i];
            if (c.answered && !c.shut) {
                ssize_t n = send(c.fd, c.out.data() + c.sent, c.out.size() - c.sent, SEND_FLAGS);
                if (n > 0) {
                    c.sent += (size_t) n;
                } else if (n < 0 && !wouldBlock()) {
                    drop(i, "send error");
                    continue;
                }
                if (c.sent >= c.out.size()) {
                    // close our side only (FIN) and let the phone read everything: a plain close()
                    // with unread data in our buffer would reset the connection and the phone could
                    // lose the answer
#ifdef SHUT_WR
                    shutdown(c.fd, SHUT_WR);
#endif
                    c.shut = true;
                    c.shutTime = now;
                }
            }

            long long age = now - (c.shut ? c.shutTime : c.start);
            if (age < 0 || age > (c.shut ? LINGER_MS : CLIENT_TIMEOUT_MS)) {
                drop(i, c.shut ? nullptr : "timeout");
                continue;
            }
            i++;
        }
    }

    void run() {
        log("thread started");
        int pass = 0;
        while (running) {
            const long long t0 = RemoteServer::nowMs();
            serviceOnce(100);
            if (pass < 5) {
                // for the log: how long select() really waits on the console (100 ms expected)
                log("pass %i took %lld ms", pass, RemoteServer::nowMs() - t0);
                pass++;
            }
            // select() should have waited; if it came back at once with nothing to do, wait here
            sleepMs(MIN_LOOP_MS - (RemoteServer::nowMs() - t0));
        }
        for (auto &c: clients) {
            close(c.fd);
        }
        clients.clear();
        close(listenFd);
        listenFd = -1;
        log("thread ended");
    }
};

namespace {
    int threadMain(void *arg) {
        auto *keep = static_cast<std::shared_ptr<RemoteServer::Shared> *>(arg);
        (*keep)->run();
        delete keep; // the last reference may be this one: the server object can be gone already
        return 0;
    }
}

long long RemoteServer::nowMs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

RemoteServer::~RemoteServer() {
    stop();
}

bool RemoteServer::start(int p, int tries) {
    stop();
    error.clear();

    int lastErrno = 0;
    const char *lastStep = "socket()";
    auto sh = std::make_shared<Shared>();
    sh->logPath = logPath;

    for (int i = 0; i < tries; i++) {
        const int tryPort = (p == 0) ? 0 : p + i;

        // IPPROTO_TCP like the ftp client (ftplib) does on the console
        int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd < 0) {
            lastErrno = errno;
            lastStep = "socket()";
            break; // the same for every port
        }
        if (fd >= FD_SETSIZE) {
            close(fd);
            lastErrno = EMFILE;
            lastStep = "select() limit";
            break;
        }

        int yes = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &yes, sizeof(yes));

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
#ifdef __PS4__
        addr.sin_len = sizeof(addr);
#endif
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons((unsigned short) tryPort);

        if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            lastErrno = errno;
            lastStep = "bind()";
            sh->log("bind(%i) failed, errno %i", tryPort, lastErrno);
            close(fd);
            if (p == 0) break;
            continue; // port busy: try the next one
        }
        if (listen(fd, 8) < 0) {
            lastErrno = errno;
            lastStep = "listen()";
            sh->log("listen(%i) failed, errno %i", tryPort, lastErrno);
            close(fd);
            continue;
        }

        // port 0 = the system picks one (tests)
        int used = tryPort;
        if (tryPort == 0) {
            socklen_t len = sizeof(addr);
            if (getsockname(fd, (struct sockaddr *) &addr, &len) == 0) {
                used = ntohs(addr.sin_port);
            }
        }

        sh->listenFd = fd;
        port = used;
        sh->log("listening on port %i (socket %i)", port, fd);

        auto *keep = new std::shared_ptr<Shared>(sh);
#ifdef __PPLAY__
        SDL_Thread *thread = SDL_CreateThread(threadMain, "pplay remote", keep);
        if (thread == nullptr) {
            delete keep;
            close(fd);
            error = "cannot create the thread";
            sh->log("%s", error.c_str());
            return false;
        }
        SDL_DetachThread(thread);
#else
        std::thread(threadMain, keep).detach();
#endif
        shared = sh;
        return true;
    }

    char buf[96];
    snprintf(buf, sizeof(buf), "%s failed, errno %i (ports %i-%i)", lastStep, lastErrno, p, p + tries - 1);
    error = buf;
    sh->log("cannot start: %s", error.c_str());
    return false;
}

void RemoteServer::stop() {
    if (shared) {
        shared->running = false; // the thread closes everything and ends by itself
        shared.reset();
    }
}

void RemoteServer::poll(std::vector<RemoteCommand> &commands) {
    if (!shared) {
        return;
    }
    Guard g(shared->lock);
    commands.insert(commands.end(), shared->commands.begin(), shared->commands.end());
    shared->commands.clear();
}

bool RemoteServer::wantsState() const {
    if (!shared) {
        return false;
    }
    const long long now = nowMs();
    const long long asked = shared->stateAskedMs;
    return asked != 0 && now - asked < STATE_WANTED_MS && now >= asked;
}

void RemoteServer::setState(const std::string &json) {
    if (!shared) {
        return;
    }
    Guard g(shared->lock);
    shared->state = json;
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
#ifdef __PS4__
    remote.sin_len = sizeof(remote);
#endif
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
