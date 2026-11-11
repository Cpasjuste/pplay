#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "remote_protocol.h"
#include "remote_server.h"

using namespace pplay;

static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

typedef RemoteCommand::Type T;

static void protocol() {
    std::string m, p;
    CHECK(RemoteProtocol::parseRequestLine("POST /cmd/seek/10?x=1 HTTP/1.1\r\nHost: a\r\n\r\n", m, p));
    CHECK(m == "POST" && p == "/cmd/seek/10");
    CHECK(RemoteProtocol::parseRequestLine("GET / HTTP/1.0\r\n\r\n", m, p) && p == "/");
    CHECK(!RemoteProtocol::parseRequestLine("garbage\r\n\r\n", m, p));
    CHECK(!RemoteProtocol::parseRequestLine("GET  HTTP/1.1\r\n\r\n", m, p));
    CHECK(!RemoteProtocol::parseRequestLine("GET nopath HTTP/1.1\r\n\r\n", m, p));
    CHECK(!RemoteProtocol::parseRequestLine("GET / FTP/1.1\r\n\r\n", m, p));
    CHECK(!RemoteProtocol::parseRequestLine("", m, p));

    RemoteCommand c = RemoteProtocol::parseCommand("/cmd/key/up");
    CHECK(c.type == T::Key && c.key == "up");
    for (auto &name: RemoteProtocol::keyNames()) {
        CHECK(RemoteProtocol::parseCommand("/cmd/key/" + name).type == T::Key);
    }
    CHECK(RemoteProtocol::parseCommand("/cmd/key/power").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/cmd/key/").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/cmd/playpause").type == T::PlayPause);
    CHECK(RemoteProtocol::parseCommand("/cmd/stop").type == T::Stop);
    CHECK(RemoteProtocol::parseCommand("/cmd/stop/now").type == T::None);

    c = RemoteProtocol::parseCommand("/cmd/seek/-10");
    CHECK(c.type == T::Seek && c.value == -10);
    c = RemoteProtocol::parseCommand("/cmd/seek/600");
    CHECK(c.type == T::Seek && c.value == 600);
    CHECK(RemoteProtocol::parseCommand("/cmd/seek/0").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/cmd/seek/abc").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/cmd/seek/10x").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/cmd/seek/99999").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/cmd/seek/nan").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/cmd/seek/inf").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/cmd/seek/").type == T::None);

    c = RemoteProtocol::parseCommand("/cmd/speed/2");
    CHECK(c.type == T::Speed && c.value == 2);
    CHECK(RemoteProtocol::parseCommand("/cmd/speed/0.5").type == T::Speed);
    CHECK(RemoteProtocol::parseCommand("/cmd/speed/0").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/cmd/speed/1000").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/state").type == T::None);
    CHECK(RemoteProtocol::parseCommand("/cmd/").type == T::None);

    std::string j = RemoteProtocol::stateJson("a \"b\"\\c\n\x01", 12.34, 100, true, true);
    CHECK(j == "{\"title\":\"a \\\"b\\\"\\\\c\\n\\u0001\",\"position\":12.3,\"duration\":100.0,"
               "\"paused\":true,\"playing\":true}");

    std::string r = RemoteProtocol::response(200, "text/plain", "hi");
    CHECK(r.find("HTTP/1.1 200 OK\r\n") == 0);
    CHECK(r.find("Content-Length: 2\r\n") != std::string::npos);
    CHECK(r.size() >= 2 && r.substr(r.size() - 2) == "hi");
    CHECK(RemoteProtocol::page().find("/cmd/") != std::string::npos);
}

// connect to the server, send the request, pump the server until the answer is complete
static std::string request(RemoteServer &s, const std::string &req, std::vector<RemoteCommand> &cmds,
                           const std::string &state = "{}") {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short) s.getPort());
    a.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(fd, (struct sockaddr *) &a, sizeof(a)) != 0) {
        close(fd);
        return "CONNECT FAILED";
    }
    if (!req.empty()) {
        send(fd, req.data(), req.size(), 0);
    }

    std::string out;
    for (int i = 0; i < 200; i++) {
        if (i == 0) s.setState(state);
        char buf[4096];
        ssize_t n = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
        if (n > 0) {
            out.append(buf, (size_t) n);
        } else if (n == 0) {
            break;
        }
        usleep(2000);
    }
    close(fd);
    s.poll(cmds);
    return out;
}

static void server() {
    RemoteServer s;
    CHECK(s.start(0));
    CHECK(s.isRunning() && s.getPort() > 0);

    std::vector<RemoteCommand> cmds;
    std::string r = request(s, "GET / HTTP/1.1\r\nHost: x\r\n\r\n", cmds);
    CHECK(r.find("200 OK") != std::string::npos);
    CHECK(r.find("<title>pplay remote</title>") != std::string::npos);
    CHECK(cmds.empty());

    r = request(s, "GET /state HTTP/1.1\r\n\r\n", cmds, "{\"playing\":false}");
    CHECK(r.find("application/json") != std::string::npos);
    CHECK(r.find("{\"playing\":false}") != std::string::npos);

    r = request(s, "POST /cmd/key/left HTTP/1.1\r\nContent-Length: 0\r\n\r\n", cmds);
    CHECK(r.find("200 OK") != std::string::npos);
    CHECK(cmds.size() == 1 && cmds[0].type == T::Key && cmds[0].key == "left");

    r = request(s, "POST /cmd/seek/-10 HTTP/1.1\r\n\r\n", cmds);
    CHECK(cmds.size() == 2 && cmds[1].type == T::Seek && cmds[1].value == -10);

    // a browser pre-flight or a GET must not press buttons
    r = request(s, "GET /cmd/key/up HTTP/1.1\r\n\r\n", cmds);
    CHECK(r.find("405") != std::string::npos);
    r = request(s, "POST /cmd/key/nope HTTP/1.1\r\n\r\n", cmds);
    CHECK(r.find("400") != std::string::npos);
    r = request(s, "GET /secret HTTP/1.1\r\n\r\n", cmds);
    CHECK(r.find("404") != std::string::npos);
    r = request(s, "nonsense\r\n\r\n", cmds);
    CHECK(r.find("400") != std::string::npos);
    CHECK(cmds.size() == 2);

    // client that connects and says nothing / sends half a request: server stays alive
    request(s, "", cmds);
    request(s, "GET / HTTP/1.1\r\nHost", cmds);
    r = request(s, "GET /state HTTP/1.1\r\n\r\n", cmds, "{}");
    CHECK(r.find("200 OK") != std::string::npos);

    // huge request is dropped
    std::string big = "POST /cmd/stop HTTP/1.1\r\nX: " + std::string(20000, 'a');
    request(s, big, cmds);
    CHECK(cmds.size() == 2);

    s.stop();
    CHECK(!s.isRunning());

    // port already in use: the next one is used
    RemoteServer a, b;
    CHECK(a.start(0));
    int busy = a.getPort();
    CHECK(b.start(busy, 5));
    CHECK(b.getPort() == busy + 1);
    RemoteServer c;
    CHECK(c.start(busy, 1) == false);
    CHECK(!c.getError().empty());
    printf("expected error: %s\n", c.getError().c_str());

    // the answer survives a request whose body arrives after the headers (no connection reset)
    {
        RemoteServer s2;
        CHECK(s2.start(0));
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((unsigned short) s2.getPort());
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        CHECK(connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == 0);
        std::string head = "POST /cmd/key/ok HTTP/1.1\r\nContent-Length: 5\r\n\r\n";
        send(fd, head.data(), head.size(), 0);
        std::vector<RemoteCommand> got;
        usleep(100000);
        send(fd, "hello", 5, 0);
        std::string out;
        for (int i = 0; i < 100; i++) {
            char buf[512];
            ssize_t n = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
            if (n > 0) out.append(buf, (size_t) n);
            else if (n == 0) break;
            usleep(2000);
        }
        close(fd);
        s2.poll(got);
        CHECK(out.find("200 OK") != std::string::npos);
        CHECK(got.size() == 1 && got[0].key == "ok");
    }

    // idle connections (browsers open spare ones) do not delay the real request, and /state
    // is only built while a phone asks for it
    {
        RemoteServer s3;
        CHECK(s3.start(0));
        CHECK(!s3.wantsState());
        std::vector<int> idle;
        for (int i = 0; i < 3; i++) {
            int fd = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons((unsigned short) s3.getPort());
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            CHECK(connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == 0);
            idle.push_back(fd);
        }
        std::vector<RemoteCommand> c3;
        long long t0 = RemoteServer::nowMs();
        std::string r3 = request(s3, "GET /state HTTP/1.1\r\n\r\n", c3, "{\"x\":1}");
        // request() waits for the close of the connection: the answer must have come well before 400 ms
        CHECK(r3.find("{\"x\":1}") != std::string::npos);
        CHECK(RemoteServer::nowMs() - t0 < 1000);
        CHECK(s3.wantsState());
        for (int fd: idle) close(fd);
        s3.stop();
        CHECK(!s3.isRunning());
        CHECK(!s3.wantsState());
        std::vector<RemoteCommand> none;
        s3.poll(none);
        CHECK(none.empty());
    }

    // a browser using https:// sends a TLS handshake: answered at once, not after a time out
    {
        RemoteServer s4;
        CHECK(s4.start(0));
        std::vector<RemoteCommand> c4;
        std::string hello("\x16\x03\x01\x07\xc6\x01\x00\x07\xc2\x03\x03\x0a\x0a\x0a\x0a", 15);
        hello += std::string(300, 'x');
        long long t0 = RemoteServer::nowMs();
        std::string r4 = request(s4, hello, c4);
        CHECK(r4.find("400") != std::string::npos);
        CHECK(r4.find("use http://") != std::string::npos);
        CHECK(RemoteServer::nowMs() - t0 < 1000);
        CHECK(c4.empty());
    }

    // log file
    {
        std::string path = "remote_test.log";
        RemoteServer l;
        l.setLogFile(path);
        CHECK(l.start(0));
        std::vector<RemoteCommand> cmds2;
        request(l, "GET / HTTP/1.1\r\n\r\n", cmds2);
        FILE *f = fopen(path.c_str(), "r");
        CHECK(f != nullptr);
        char line[256];
        std::string all;
        while (f && fgets(line, sizeof(line), f)) all += line;
        if (f) fclose(f);
        CHECK(all.find("listening on port") != std::string::npos);
        CHECK(all.find("GET / -> 200") != std::string::npos);
    }

    printf("local address: '%s'\n", RemoteServer::localAddress().c_str());
}

int main() {
    protocol();
    server();
    if (failures) {
        printf("%d FAILURE(S)\n", failures);
        return 1;
    }
    printf("remote: all tests passed\n");
    return 0;
}
