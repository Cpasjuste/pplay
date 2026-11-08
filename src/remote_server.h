//
// Tiny HTTP server for the phone remote control (see remote_protocol.h).
//
// No threads: everything is done by service(), called once per frame by the main loop, with
// select() polling so nothing ever blocks. Requests are small (a line of text), one frame is
// enough to read and answer them.
//

#ifndef PPLAY_REMOTE_SERVER_H
#define PPLAY_REMOTE_SERVER_H

#include <chrono>
#include <string>
#include <vector>
#include "remote_protocol.h"

namespace pplay {

    class RemoteServer {
    public:
        RemoteServer() = default;

        ~RemoteServer();

        // listen on all interfaces. False if the port cannot be used.
        bool start(int port);

        void stop();

        bool isRunning() const { return listenFd >= 0; }

        int getPort() const { return port; }

        // Accept/read/answer pending connections. Commands received are appended to `commands`;
        // `stateJson` is what GET /state answers (see RemoteProtocol::stateJson).
        void service(const std::string &stateJson, std::vector<RemoteCommand> &commands);

        // IPv4 address other devices use to reach this machine ("192.168.1.20"), "" if unknown
        static std::string localAddress();

    private:
        struct Client {
            int fd = -1;
            std::string in;
            std::string out;
            size_t sent = 0;
            bool answered = false;
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        };

        void answer(Client &client, const std::string &stateJson, std::vector<RemoteCommand> &commands);

        void drop(size_t index);

        int listenFd = -1;
        int port = 0;
        std::vector<Client> clients;
    };
}

#endif //PPLAY_REMOTE_SERVER_H
