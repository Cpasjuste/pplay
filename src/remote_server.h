//
// Tiny HTTP server for the phone remote control (see remote_protocol.h).
//
// No threads and no std::chrono (the console libc has no reliable monotonic clock, the rest of
// pplay uses gettimeofday too): everything is done by service(), called once per frame by the
// main loop, with select() polling so nothing ever blocks. Requests are small (a line of text),
// one frame is enough to read and answer them.
//

#ifndef PPLAY_REMOTE_SERVER_H
#define PPLAY_REMOTE_SERVER_H

#include <functional>
#include <string>
#include <vector>
#include "remote_protocol.h"

namespace pplay {

    class RemoteServer {
    public:
        RemoteServer() = default;

        ~RemoteServer();

        // Listen on all interfaces, on `port` or, if it is taken, on the next ones (port+1...
        // up to `tries` ports). False if none can be used (see getError()).
        bool start(int port, int tries = 10);

        void stop();

        bool isRunning() const { return listenFd >= 0; }

        // port really used (may differ from the one asked)
        int getPort() const { return port; }

        // why the last start() failed ("", or "socket() errno 13", "port 8080-8089 busy"...)
        const std::string &getError() const { return error; }

        // append what happens (start, connections, requests, errors) to this file, so a console
        // user can read it by FTP when something does not work. Capped to a few hundred lines.
        void setLogFile(const std::string &path) { logPath = path; }

        // Accept/read/answer pending connections. Commands received are appended to `commands`;
        // `state` builds the answer of GET /state (called only when a phone asks for it).
        void service(const std::function<std::string()> &state, std::vector<RemoteCommand> &commands);

        // IPv4 address other devices use to reach this machine ("192.168.1.20"), "" if unknown
        static std::string localAddress();

        // milliseconds (gettimeofday)
        static long long nowMs();

    private:
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

        void answer(Client &client, const std::function<std::string()> &state,
                    std::vector<RemoteCommand> &commands);

        void drop(size_t index, const char *why);

        void log(const char *fmt, ...);

        int listenFd = -1;
        int port = 0;
        int logLines = 0;
        std::string error;
        std::string logPath;
        std::vector<Client> clients;
    };
}

#endif //PPLAY_REMOTE_SERVER_H
