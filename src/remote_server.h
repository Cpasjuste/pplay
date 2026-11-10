//
// Tiny HTTP server for the phone remote control (see remote_protocol.h).
//
// It runs in its own thread, so whatever the sockets do (a slow phone, a call that blocks on the
// console) can never freeze the application: the main loop only calls poll() (takes the commands
// received) and, when a phone asks for it, setState(). No std::chrono either (the console libc has
// no reliable monotonic clock; the rest of pplay uses gettimeofday).
//

#ifndef PPLAY_REMOTE_SERVER_H
#define PPLAY_REMOTE_SERVER_H

#include <memory>
#include <string>
#include <vector>
#include "remote_protocol.h"

namespace pplay {

    class RemoteServer {
    public:
        RemoteServer() = default;

        ~RemoteServer();

        // Listen on all interfaces, on `port` or, if it is taken, on the next ones (port+1...
        // up to `tries` ports), and start answering. False if none can be used (see getError()).
        bool start(int port, int tries = 10);

        // Stops answering. Never waits for the thread (it ends by itself within a fraction of a second).
        void stop();

        bool isRunning() const { return shared != nullptr; }

        // port really used (may differ from the one asked)
        int getPort() const { return port; }

        // why the last start() failed ("" if it did not)
        const std::string &getError() const { return error; }

        // append what happens (start, connections, requests, errors) to this file, so a console
        // user can read it by FTP when something does not work. Capped to a few hundred lines.
        // Must be called before start().
        void setLogFile(const std::string &path) { logPath = path; }

        // main loop: the commands received since the last call are appended to `commands`
        void poll(std::vector<RemoteCommand> &commands);

        // true while a phone is asking for GET /state (every second): build it and call setState()
        bool wantsState() const;

        void setState(const std::string &json);

        // IPv4 address other devices use to reach this machine ("192.168.1.20"), "" if unknown
        static std::string localAddress();

        // milliseconds (gettimeofday)
        static long long nowMs();

        struct Shared;

    private:
        std::shared_ptr<Shared> shared;
        int port = 0;
        std::string error;
        std::string logPath;
    };
}

#endif //PPLAY_REMOTE_SERVER_H
