//
// Phone remote control: the text part of the protocol (no sockets, no UI), testable on a PC.
//
//   GET  /            the remote control page (HTML, made for a phone)
//   GET  /state       {"title":"..","position":12,"duration":3600,"paused":false,"playing":true}
//   POST /cmd/key/<name>      up down left right ok back select start l1 r1 l2 r2 fire3 fire4
//   POST /cmd/playpause
//   POST /cmd/stop
//   POST /cmd/seek/<seconds>  relative, "-10", "30"
//   POST /cmd/speed/<factor>  "1", "2", "0.5"
//

#ifndef PPLAY_REMOTE_PROTOCOL_H
#define PPLAY_REMOTE_PROTOCOL_H

#include <string>
#include <vector>

namespace pplay {

    struct RemoteCommand {
        enum class Type {
            None,       // not a (valid) command
            Key,        // press a button of the controller (name in `key`)
            PlayPause,
            Stop,
            Seek,       // relative, seconds in `value`
            Speed       // factor in `value`
        };

        Type type = Type::None;
        std::string key;
        double value = 0;
    };

    class RemoteProtocol {
    public:
        // button names accepted by /cmd/key/<name>
        static const std::vector<std::string> &keyNames();

        // "POST /cmd/seek/10 HTTP/1.1\r\n..." -> method "POST", path "/cmd/seek/10".
        // False when the request line is malformed. The query string is dropped.
        static bool parseRequestLine(const std::string &request, std::string &method, std::string &path);

        // "/cmd/seek/-10" -> Seek -10. Type::None for anything unknown or out of range
        // (seek more than 10 hours, speed outside 0.25..100, unknown button...).
        static RemoteCommand parseCommand(const std::string &path);

        // complete HTTP/1.1 response (headers + body), connection closed after it
        static std::string response(int status, const std::string &contentType, const std::string &body);

        // JSON for GET /state
        static std::string stateJson(const std::string &title, double position, double duration,
                                     bool paused, bool playing);

        // the remote control page
        static const std::string &page();
    };
}

#endif //PPLAY_REMOTE_PROTOCOL_H
