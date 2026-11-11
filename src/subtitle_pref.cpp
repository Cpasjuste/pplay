//
// See subtitle_pref.h
//

#include <algorithm>
#include <cctype>
#include "subtitle_pref.h"

using namespace pplay;

namespace {

    // lower case (ASCII and the UTF-8 encoded Latin-1 capitals: "ESPAÑOL" -> "español")
    std::string lower(const std::string &s) {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); i++) {
            unsigned char c = (unsigned char) s[i];
            if (c >= 'A' && c <= 'Z') {
                out += (char) (c + 32);
            } else if (c == 0xC3 && i + 1 < s.size()
                       && (unsigned char) s[i + 1] >= 0x80 && (unsigned char) s[i + 1] <= 0x9E
                       && (unsigned char) s[i + 1] != 0x97) {
                out += (char) c;
                out += (char) ((unsigned char) s[++i] + 0x20);
            } else {
                out += (char) c;
            }
        }
        return out;
    }

    // words of a text, lower case: runs of letters/digits (bytes >= 0x80 are part of a word
    // so "español" stays in one piece)
    std::vector<std::string> words(const std::string &text) {
        std::vector<std::string> out;
        std::string cur;
        for (unsigned char c: lower(text)) {
            if (std::isalnum(c) || c >= 0x80) {
                cur += (char) c;
            } else if (!cur.empty()) {
                out.push_back(cur);
                cur.clear();
            }
        }
        if (!cur.empty()) {
            out.push_back(cur);
        }
        return out;
    }

    bool startsWith(const std::string &s, const char *prefix) {
        return s.compare(0, std::char_traits<char>::length(prefix), prefix) == 0;
    }

    bool has(const std::vector<std::string> &w, const char *word) {
        return std::find(w.begin(), w.end(), word) != w.end();
    }

    bool hasPrefix(const std::vector<std::string> &w, const char *prefix) {
        for (auto &s: w) {
            if (startsWith(s, prefix)) return true;
        }
        return false;
    }

    // ISO 639-2 (3 letters) -> ISO 639-1 (2 letters)
    std::string twoLetters(const std::string &code) {
        static const char *map[][2] = {
                {"spa", "es"}, {"eng", "en"}, {"jpn", "ja"}, {"fre", "fr"}, {"fra", "fr"},
                {"ger", "de"}, {"deu", "de"}, {"ita", "it"}, {"por", "pt"}, {"rus", "ru"},
                {"kor", "ko"}, {"chi", "zh"}, {"zho", "zh"}, {"cmn", "zh"}, {"yue", "zh"},
                {"ara", "ar"}, {"dut", "nl"}, {"nld", "nl"}, {"pol", "pl"}, {"tur", "tr"},
                {"swe", "sv"}, {"nor", "no"}, {"nob", "no"}, {"dan", "da"}, {"fin", "fi"},
                {"cze", "cs"}, {"ces", "cs"}, {"hun", "hu"}, {"gre", "el"}, {"ell", "el"},
                {"heb", "he"}, {"tha", "th"}, {"vie", "vi"}, {"ind", "id"}, {"ukr", "uk"},
                {"rum", "ro"}, {"ron", "ro"}, {"bul", "bg"}, {"cat", "ca"}, {"hin", "hi"},
                {"lat", "es"} // "movie.lat.srt" means Latino, not the Latin language
        };
        if (code.size() == 2) {
            return std::isalpha((unsigned char) code[0]) && std::isalpha((unsigned char) code[1]) ? code : "";
        }
        for (auto &m: map) {
            if (code == m[0]) return m[1];
        }
        return "";
    }

    // language named in a title ("Español Latino", "English [SDH]", "Japanese")
    std::string titleLanguage(const std::vector<std::string> &w) {
        static const char *map[][2] = {
                {"spanish",  "es"}, {"espa",     "es"}, {"castell", "es"}, {"latin",  "es"},
                {"latam",    "es"}, {"hispan",   "es"}, {"english", "en"}, {"ingl",   "en"},
                {"japan",    "ja"}, {"japon",    "ja"}, {"french",  "fr"}, {"franc",  "fr"},
                {"german",   "de"}, {"deutsch",  "de"}, {"alem",    "de"}, {"portu",  "pt"},
                {"ital",     "it"}, {"chin",     "zh"}, {"mandarin", "zh"}, {"korea", "ko"},
                {"corea",    "ko"}, {"russ",     "ru"}, {"ruso",    "ru"}, {"arab",   "ar"}
        };
        for (auto &word: w) {
            for (auto &m: map) {
                if (startsWith(word, m[0])) return m[1];
            }
        }
        return "";
    }

    bool isLatinRegion(const std::string &r) {
        static const char *regions[] = {"419", "la", "mx", "ar", "cl", "co", "pe", "ve", "uy", "ec", "bo",
                                        "py", "cr", "pa", "gt", "hn", "ni", "sv", "do", "cu", "pr", "us", "lat"};
        for (auto *s: regions) {
            if (r == s) return true;
        }
        return false;
    }

    struct Info {
        int id = 0;
        size_t index = 0;
        std::string lang;                  // "es", "en"... or ""
        std::vector<std::string> subtags;  // region subtags of the language tag ("419", "gb"...)
        std::vector<std::string> title;    // words of the title
        bool latin = false;                // Latin American Spanish
        bool uk = false;                   // British English
        bool bitmap = false;
        bool sdh = false;
        int cls = 0;                       // 0 full track, 1 forced / signs & songs, 2 never chosen
    };

    Info analyze(const SubtitlePref::Track &t, size_t index) {
        Info in;
        in.id = t.id;
        in.index = index;
        in.title = words(t.title);
        in.bitmap = SubtitlePref::isBitmap(t.codec);

        // language tag: "spa", "es", "es-419", "pt_BR"
        std::vector<std::string> tag = words(t.lang);
        std::string primary = tag.empty() ? "" : twoLetters(tag[0]);
        for (size_t i = 1; i < tag.size(); i++) {
            in.subtags.push_back(tag[i]);
        }
        const bool tagLat = !tag.empty() && tag[0] == "lat";

        // a language written in the title wins: mpv reports "eng" for tracks without language
        std::string fromTitle = titleLanguage(in.title);
        in.lang = fromTitle.empty() ? primary : fromTitle;

        if (in.lang == "es") {
            bool explicitLatin = tagLat || hasPrefix(in.title, "latin") || hasPrefix(in.title, "mexic")
                                 || hasPrefix(in.title, "hispanoam") || has(in.title, "lat")
                                 || has(in.title, "latam") || has(in.title, "mx") || has(in.title, "mex")
                                 || has(in.title, "419");
            for (auto &s: in.subtags) {
                if (isLatinRegion(s)) explicitLatin = true;
            }
            for (size_t i = 0; i + 1 < in.title.size(); i++) {
                if (in.title[i] == "es" && isLatinRegion(in.title[i + 1])) explicitLatin = true;
            }
            bool spain = has(in.title, "spain") || has(in.title, "españa") || has(in.title, "espana")
                         || has(in.title, "castellano") || has(in.title, "castilian")
                         || has(in.title, "iberian") || has(in.title, "european")
                         || has(in.title, "europeo") || has(in.title, "peninsular");
            in.latin = explicitLatin && !spain;
        } else if (in.lang == "en") {
            in.uk = has(in.title, "uk") || has(in.title, "gb") || has(in.title, "british")
                    || has(in.title, "britain") || has(in.title, "england");
            for (auto &s: in.subtags) {
                if (s == "gb" || s == "uk") in.uk = true;
            }
        }

        in.sdh = has(in.title, "sdh") || has(in.title, "cc") || has(in.title, "hoh")
                 || has(in.title, "hearing") || has(in.title, "closed") || has(in.title, "sordos");

        if (hasPrefix(in.title, "commentar") || hasPrefix(in.title, "comentari")) {
            in.cls = 2;
        } else if (hasPrefix(in.title, "forced") || hasPrefix(in.title, "forzad") || has(in.title, "signs")
                   || has(in.title, "sign") || has(in.title, "songs") || has(in.title, "karaoke")
                   || has(in.title, "carteles") || has(in.title, "rotulos") || has(in.title, "rótulos")) {
            in.cls = 1;
        }

        return in;
    }

    // does the track match the preference ("es-419", "en", "pt-br")? `marked` is set when the track
    // is a regional variant that a plain language preference only accepts as a second choice
    bool matches(const Info &in, const std::string &pref, bool &marked) {
        marked = false;
        std::string primary = pref, region;
        size_t dash = pref.find_first_of("-_");
        if (dash != std::string::npos) {
            primary = pref.substr(0, dash);
            region = pref.substr(dash + 1);
        }
        if (primary.size() == 3) {
            primary = twoLetters(primary);
        }
        if (primary.empty() || in.lang != primary) {
            return false;
        }

        if (region.empty()) {
            marked = in.latin || in.uk;
            return true;
        }

        if (primary == "es" && isLatinRegion(region)) return in.latin;
        if (primary == "es" && region == "es") return !in.latin;
        if (primary == "en" && (region == "gb" || region == "uk")) return in.uk;
        if (primary == "en" && region == "us") return !in.uk;

        for (auto &s: in.subtags) {
            if (s == region) return true;
        }
        return has(in.title, region.c_str());
    }
}

bool SubtitlePref::isBitmap(const std::string &codec) {
    return codec == "hdmv_pgs_subtitle" || codec == "dvd_subtitle"
           || codec == "dvb_subtitle" || codec == "xsub";
}

std::string SubtitlePref::language(const Track &track) {
    return analyze(track, 0).lang;
}

std::vector<std::string> SubtitlePref::parse(const std::string &list) {
    std::vector<std::string> out;
    std::string cur;
    for (size_t i = 0; i <= list.size(); i++) {
        char c = i < list.size() ? list[i] : ',';
        if (c == ',' || c == ';' || c == ' ' || c == '\t') {
            if (!cur.empty()) {
                out.push_back(lower(cur));
                cur.clear();
            }
        } else {
            cur += c;
        }
    }
    return out;
}

int SubtitlePref::choose(const std::vector<Track> &tracks, const std::vector<std::string> &prefs) {
    std::vector<Info> infos;
    for (size_t i = 0; i < tracks.size(); i++) {
        infos.push_back(analyze(tracks[i], i));
    }

    // full tracks first, forced/signs only when nothing else can be shown
    for (int cls = 0; cls <= 1; cls++) {
        for (auto &pref: prefs) {
            const Info *best = nullptr;
            int bestKey[4] = {0, 0, 0, 0};
            for (auto &in: infos) {
                bool marked = false;
                if (in.cls != cls || !matches(in, pref, marked)) {
                    continue;
                }
                // plain before regional variant, text before pictures, normal before SDH/CC
                int key[4] = {marked ? 1 : 0, in.bitmap ? 1 : 0, in.sdh ? 1 : 0, (int) in.index};
                if (best == nullptr || std::lexicographical_compare(key, key + 4, bestKey, bestKey + 4)) {
                    best = &in;
                    std::copy(key, key + 4, bestKey);
                }
            }
            if (best != nullptr) {
                return best->id;
            }
        }
    }

    return -1;
}
