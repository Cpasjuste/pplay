//
// See remote_protocol.h
//

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "remote_protocol.h"

using namespace pplay;

namespace {

    bool startsWith(const std::string &s, const std::string &prefix) {
        return s.compare(0, prefix.size(), prefix) == 0;
    }

    bool parseNumber(const std::string &text, double &out) {
        if (text.empty() || text.size() > 16) {
            return false;
        }
        char *end = nullptr;
        double v = std::strtod(text.c_str(), &end);
        if (end == nullptr || *end != '\0' || !std::isfinite(v)) {
            return false;
        }
        out = v;
        return true;
    }

    std::string jsonString(const std::string &s) {
        std::string out = "\"";
        for (unsigned char c: s) {
            switch (c) {
                case '"':
                    out += "\\\"";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    if (c < 0x20) {
                        char buf[8];
                        std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                        out += buf;
                    } else {
                        out += (char) c;
                    }
            }
        }
        return out + "\"";
    }

    std::string number(double v) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f", std::isfinite(v) ? v : 0.0);
        return buf;
    }

    const char *statusText(int status) {
        switch (status) {
            case 200:
                return "OK";
            case 400:
                return "Bad Request";
            case 404:
                return "Not Found";
            case 405:
                return "Method Not Allowed";
            default:
                return "Error";
        }
    }

    // the page is plain HTML + a little JavaScript: no external files, works offline on the LAN
    const char *PAGE = R"HTML(<!DOCTYPE html>
<html lang="es"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<meta name="theme-color" content="#101418">
<title>pplay remote</title>
<style>
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent;user-select:none;-webkit-user-select:none}
body{margin:0;background:#101418;color:#e8edf2;font:16px system-ui,sans-serif;display:flex;flex-direction:column;
min-height:100vh;padding:12px;gap:12px;max-width:480px;margin:0 auto}
#info{background:#1a2027;border-radius:14px;padding:12px 14px}
#title{font-weight:600;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
#bar{height:6px;background:#2a333d;border-radius:3px;margin:10px 0 6px;overflow:hidden}
#fill{height:100%;width:0;background:#e5484d}
#times{display:flex;justify-content:space-between;font-size:13px;color:#8b98a5}
#status{font-size:12px;color:#8b98a5;text-align:center}
.grid{display:grid;gap:10px}
.dpad{grid-template-columns:repeat(3,1fr);grid-template-rows:repeat(3,76px)}
.row{grid-template-columns:repeat(3,1fr)}
.row2{grid-template-columns:repeat(2,1fr)}
button{background:#232b34;color:#e8edf2;border:0;border-radius:14px;font-size:18px;min-height:56px;touch-action:manipulation}
button:active{background:#e5484d}
.dpad button{font-size:26px}
.ok{background:#e5484d}
.small{font-size:14px}
h2{margin:6px 0 0;font-size:12px;font-weight:600;color:#8b98a5;letter-spacing:.08em;text-transform:uppercase}
</style></head><body>
<div id="info"><div id="title">pplay</div><div id="bar"><div id="fill"></div></div>
<div id="times"><span id="pos">0:00</span><span id="dur">0:00</span></div></div>
<h2>Navegar</h2>
<div class="grid dpad">
<span></span><button data-k="up">&#9650;</button><span></span>
<button data-k="left">&#9664;</button><button data-k="ok" class="ok">OK</button><button data-k="right">&#9654;</button>
<span></span><button data-k="down">&#9660;</button><span></span></div>
<div class="grid row2"><button data-k="back">Atr&aacute;s</button><button data-k="start">Start</button></div>
<h2>Reproducci&oacute;n</h2>
<div class="grid row">
<button data-s="-10">&#9194; 10 s</button><button data-c="playpause" class="ok">&#9199;</button><button data-s="10">10 s &#9193;</button>
<button data-s="-60" class="small">&minus;1 min</button><button data-c="stop" class="small">&#9209; Stop</button><button data-s="60" class="small">+1 min</button>
<button data-s="-600" class="small">&minus;10 min</button><button data-c="speed/1" class="small">1&times;</button><button data-s="600" class="small">+10 min</button>
</div>
<div class="grid row2"><button data-c="speed/2" class="small">Velocidad 2&times;</button><button data-c="speed/4" class="small">Velocidad 4&times;</button></div>
<div id="status">conectando...</div>
<script>
function t(s){s=Math.max(0,Math.floor(s||0));var h=Math.floor(s/3600),m=Math.floor(s%3600/60),x=s%60;
return(h?h+":"+(m<10?"0":""):"")+m+":"+(x<10?"0":"")+x}
function send(path){if(navigator.vibrate)navigator.vibrate(15);
fetch("/cmd/"+path,{method:"POST"}).then(function(r){st(r.ok?"":"error "+r.status)}).catch(function(){st("sin conexión")})}
function st(m){document.getElementById("status").textContent=m}
document.querySelectorAll("button").forEach(function(b){b.addEventListener("click",function(){
if(b.dataset.k)send("key/"+b.dataset.k);else if(b.dataset.s)send("seek/"+b.dataset.s);else send(b.dataset.c)})});
function poll(){fetch("/state").then(function(r){return r.json()}).then(function(s){
document.getElementById("title").textContent=s.playing?s.title:"pplay";
document.getElementById("pos").textContent=t(s.position);document.getElementById("dur").textContent=t(s.duration);
document.getElementById("fill").style.width=(s.duration>0?Math.min(100,100*s.position/s.duration):0)+"%";
st(s.playing?(s.paused?"en pausa":"reproduciendo"):"sin reproducción")}).catch(function(){st("sin conexión")})}
poll();setInterval(poll,1000);
</script></body></html>
)HTML";
}

const std::vector<std::string> &RemoteProtocol::keyNames() {
    static const std::vector<std::string> names = {
            "up", "down", "left", "right", "ok", "back", "select", "start",
            "l1", "r1", "l2", "r2", "fire3", "fire4"
    };
    return names;
}

bool RemoteProtocol::parseRequestLine(const std::string &request, std::string &method, std::string &path) {
    size_t end = request.find_first_of("\r\n");
    std::string line = request.substr(0, end);

    size_t sp1 = line.find(' ');
    if (sp1 == std::string::npos || sp1 == 0) {
        return false;
    }
    size_t sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string::npos || sp2 == sp1 + 1) {
        return false;
    }
    if (!startsWith(line.substr(sp2 + 1), "HTTP/")) {
        return false;
    }

    method = line.substr(0, sp1);
    path = line.substr(sp1 + 1, sp2 - sp1 - 1);
    size_t query = path.find_first_of("?#");
    if (query != std::string::npos) {
        path.erase(query);
    }
    return !path.empty() && path[0] == '/';
}

RemoteCommand RemoteProtocol::parseCommand(const std::string &path) {
    RemoteCommand cmd;
    const std::string base = "/cmd/";
    if (!startsWith(path, base)) {
        return cmd;
    }

    const std::string rest = path.substr(base.size());
    size_t slash = rest.find('/');
    const std::string name = rest.substr(0, slash);
    const std::string arg = slash == std::string::npos ? "" : rest.substr(slash + 1);

    if (name == "playpause" && arg.empty()) {
        cmd.type = RemoteCommand::Type::PlayPause;
    } else if (name == "stop" && arg.empty()) {
        cmd.type = RemoteCommand::Type::Stop;
    } else if (name == "key") {
        const auto &names = keyNames();
        if (std::find(names.begin(), names.end(), arg) != names.end()) {
            cmd.type = RemoteCommand::Type::Key;
            cmd.key = arg;
        }
    } else if (name == "seek") {
        double v;
        if (parseNumber(arg, v) && v != 0 && std::fabs(v) <= 36000) {
            cmd.type = RemoteCommand::Type::Seek;
            cmd.value = v;
        }
    } else if (name == "speed") {
        double v;
        if (parseNumber(arg, v) && v >= 0.25 && v <= 100) {
            cmd.type = RemoteCommand::Type::Speed;
            cmd.value = v;
        }
    }
    return cmd;
}

std::string RemoteProtocol::response(int status, const std::string &contentType, const std::string &body) {
    std::string out = "HTTP/1.1 " + std::to_string(status) + " " + statusText(status) + "\r\n";
    out += "Content-Type: " + contentType + "\r\n";
    out += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    out += "Cache-Control: no-store\r\n";
    out += "Connection: close\r\n\r\n";
    return out + body;
}

std::string RemoteProtocol::stateJson(const std::string &title, double position, double duration,
                                      bool paused, bool playing) {
    return "{\"title\":" + jsonString(title)
           + ",\"position\":" + number(position)
           + ",\"duration\":" + number(duration)
           + ",\"paused\":" + (paused ? "true" : "false")
           + ",\"playing\":" + (playing ? "true" : "false") + "}";
}

const std::string &RemoteProtocol::page() {
    static const std::string html = PAGE;
    return html;
}
