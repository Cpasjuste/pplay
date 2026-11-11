//
// See subtitles.h
//

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "subtitles.h"
#include "subtitle_tables.h"

using namespace pplay;

static const size_t MAX_SUB_SIZE = 16 * 1024 * 1024;
static const size_t MAX_SUBS = 32;

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

static std::string lower(std::string s) {
    for (auto &c: s) {
        c = (char) tolower((unsigned char) c);
    }
    return s;
}

static void appendUtf8(std::string &out, unsigned cp) {
    if (cp < 0x80) {
        out += (char) cp;
    } else if (cp < 0x800) {
        out += (char) (0xC0 | (cp >> 6));
        out += (char) (0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += (char) (0xE0 | (cp >> 12));
        out += (char) (0x80 | ((cp >> 6) & 0x3F));
        out += (char) (0x80 | (cp & 0x3F));
    } else {
        out += (char) (0xF0 | (cp >> 18));
        out += (char) (0x80 | ((cp >> 12) & 0x3F));
        out += (char) (0x80 | ((cp >> 6) & 0x3F));
        out += (char) (0x80 | (cp & 0x3F));
    }
}

static bool validUtf8(const std::string &s) {
    size_t i = 0, n = s.size();
    while (i < n) {
        unsigned char c = (unsigned char) s[i];
        size_t len;
        if (c < 0x80) len = 1;
        else if (c >= 0xC2 && c <= 0xDF) len = 2;
        else if (c >= 0xE0 && c <= 0xEF) len = 3;
        else if (c >= 0xF0 && c <= 0xF4) len = 4;
        else return false;
        if (i + len > n) {
            return false;
        }
        for (size_t k = 1; k < len; k++) {
            if (((unsigned char) s[i + k] & 0xC0) != 0x80) return false;
        }
        if (len == 3) {
            unsigned cp = ((c & 0x0F) << 12) | (((unsigned char) s[i + 1] & 0x3F) << 6);
            if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) return false;
        } else if (len == 4) {
            unsigned cp = ((c & 0x07) << 18) | (((unsigned char) s[i + 1] & 0x3F) << 12);
            if (cp < 0x10000 || cp > 0x10FFFF) return false;
        }
        i += len;
    }
    return true;
}

// Windows-1252 0x80..0x9F
static const unsigned short cp1252_hi[32] = {
        0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
        0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
        0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
        0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178
};

struct DbcsResult {
    std::string out;
    size_t pairs = 0;       // double byte characters decoded
    size_t errors = 0;      // bytes that could not be decoded
    size_t hangul = 0, kana = 0, kanaPairs = 0, han = 0;
    size_t simplified = 0, traditional = 0;
};

static const unsigned short SIMPLIFIED[] = {
        0x4EEC, 0x8FD9, 0x8BF4, 0x4E3A, 0x65F6, 0x4E2A, 0x6765, 0x4F1A, 0x5BF9, 0x6CA1, 0x8FD8,
        0x8FC7, 0x5E94, 0x53D1, 0x6211, 0x4E48, 0x4E0D, 0x4E86, 0x4ED6, 0x5979, 0x4F60, 0x4E1C, 0};
static const unsigned short TRADITIONAL[] = {
        0x5011, 0x9019, 0x8AAA, 0x70BA, 0x6642, 0x500B, 0x4F86, 0x6703, 0x5C0D, 0x6C92, 0x9084,
        0x904E, 0x61C9, 0x767C, 0x6211, 0x9EBC, 0x4E0D, 0x4E86, 0x4ED6, 0x5979, 0x4F60, 0x6771, 0};

static bool inSet(const unsigned short *set, unsigned short c) {
    for (int i = 0; set[i]; i++) {
        if (set[i] == c) return true;
    }
    return false;
}

static void decodeDbcs(const std::string &s, const DbcsTable &t, bool sjis, DbcsResult &r) {
    size_t i = 0, n = s.size();
    size_t cols = (size_t) t.trailMax - t.trailMin + 1;
    r.out.reserve(n * 2);
    while (i < n) {
        unsigned char c = (unsigned char) s[i];
        if (c < 0x80) {
            r.out += (char) c;
            i++;
            continue;
        }
        if (sjis && c >= 0xA1 && c <= 0xDF) {      // half-width katakana
            appendUtf8(r.out, 0xFF61 + (c - 0xA1));
            r.kana++;
            i++;
            continue;
        }
        bool lead = c >= t.leadMin && c <= t.leadMax && (!sjis || c <= 0x9F || c >= 0xE0);
        if (lead && i + 1 < n) {
            unsigned char d = (unsigned char) s[i + 1];
            if (d >= t.trailMin && d <= t.trailMax) {
                unsigned short u = t.map[(size_t) (c - t.leadMin) * cols + (d - t.trailMin)];
                if (u) {
                    appendUtf8(r.out, u);
                    r.pairs++;
                    if (u >= 0xAC00 && u <= 0xD7A3) r.hangul++;
                    else if (u >= 0x3040 && u <= 0x30FF) r.kanaPairs++;
                    else if (u >= 0x4E00 && u <= 0x9FFF) {
                        r.han++;
                        if (inSet(SIMPLIFIED, u)) r.simplified++;
                        if (inSet(TRADITIONAL, u)) r.traditional++;
                    }
                    i += 2;
                    continue;
                }
            }
        }
        r.errors++;
        appendUtf8(r.out, 0xFFFD);
        i++;
    }
}

// declared charset in the first bytes ("charset=euc-kr", "encoding=...")
static pplay::Subtitles::Encoding declared(const std::string &data) {
    using E = pplay::Subtitles::Encoding;
    std::string head = lower(data.substr(0, 4096));
    size_t p = head.find("charset=");
    size_t skip = 8;
    if (p == std::string::npos) {
        p = head.find("encoding=");
        skip = 9;
    }
    if (p == std::string::npos) return E::Unknown;
    std::string v = head.substr(p + skip, 24);
    auto has = [&](const char *k) { return v.find(k) != std::string::npos; };
    if (has("euc-kr") || has("ks_c_5601") || has("cp949") || has("uhc")) return E::Cp949;
    if (has("shift_jis") || has("shift-jis") || has("sjis") || has("cp932") || has("windows-31j") || has("euc-jp")) return E::Cp932;
    if (has("gb2312") || has("gbk") || has("gb18030")) return E::Gbk;
    if (has("big5")) return E::Big5;
    if (has("windows-125") || has("iso-8859") || has("latin")) return E::Cp1252;
    return E::Unknown;
}

// ---------------------------------------------------------------------------
// detection / conversion
// ---------------------------------------------------------------------------

Subtitles::Encoding Subtitles::detect(const std::string &data) {
    size_t n = data.size();
    if (n >= 3 && (unsigned char) data[0] == 0xEF && (unsigned char) data[1] == 0xBB &&
        (unsigned char) data[2] == 0xBF) {
        return Encoding::Utf8;
    }
    if (n >= 2 && (((unsigned char) data[0] == 0xFF && (unsigned char) data[1] == 0xFE) ||
                   ((unsigned char) data[0] == 0xFE && (unsigned char) data[1] == 0xFF))) {
        return Encoding::Utf8; // UTF-16 with BOM: libavformat converts it
    }

    size_t high = 0, adjacent = 0;
    for (size_t i = 0; i < n; i++) {
        if ((unsigned char) data[i] >= 0x80) {
            high++;
            if (i + 1 < n && (unsigned char) data[i + 1] >= 0x40 && (unsigned char) data[i + 1] != 0x7F) {
                // lead byte followed by something a double byte trail could be
                if (i + 1 < n && (unsigned char) data[i] >= 0x81 && (unsigned char) data[i] <= 0xFE &&
                    (unsigned char) data[i + 1] >= 0x80) {
                    adjacent++;
                }
            }
        }
    }
    if (high == 0 || validUtf8(data)) return Encoding::Utf8;

    Encoding hint = declared(data);
    if (hint == Encoding::Utf8) return Encoding::Utf8;
    if (hint != Encoding::Unknown) return hint;

    // Western text has isolated accented letters, double byte text has them in pairs
    bool doubleByte = high >= 8 && adjacent * 10 >= high * 4;
    if (!doubleByte) return Encoding::Cp1252;

    DbcsResult k, j, g, b;
    decodeDbcs(data, table_cp949, false, k);
    decodeDbcs(data, table_cp932, true, j);
    decodeDbcs(data, table_gbk, false, g);
    decodeDbcs(data, table_big5, false, b);

    // clean decodes only (a couple of stray bytes are tolerated)
    auto clean = [&](const DbcsResult &r) { return r.pairs > 0 && r.errors * 50 <= r.pairs + 2; };

    // kana is the Japanese fingerprint (Korean decoded as Shift-JIS has none),
    // hangul the Korean one (Japanese decoded as cp949 maps to rare syllables)
    if (clean(j) && j.kanaPairs * 5 >= j.pairs) return Encoding::Cp932;
    if (clean(k) && k.hangul * 10 >= k.pairs * 8) return Encoding::Cp949;

    bool cg = clean(g), cb = clean(b);
    if (cg && cb) {
        if (b.traditional > g.simplified) return Encoding::Big5;
        if (g.simplified > b.traditional) return Encoding::Gbk;
        return Encoding::Gbk;
    }
    if (cg) return Encoding::Gbk;
    if (cb) return Encoding::Big5;
    if (clean(j)) return Encoding::Cp932;
    if (clean(k)) return Encoding::Cp949;
    return Encoding::Cp1252;
}

bool Subtitles::toUtf8(const std::string &data, Encoding enc, std::string &out) {
    out.clear();
    switch (enc) {
        case Encoding::Cp1252:
            out.reserve(data.size() + data.size() / 8);
            for (unsigned char c: data) {
                if (c < 0x80) out += (char) c;
                else if (c >= 0xA0) appendUtf8(out, c);
                else appendUtf8(out, cp1252_hi[c - 0x80]);
            }
            return true;
        case Encoding::Cp949: {
            DbcsResult r;
            decodeDbcs(data, table_cp949, false, r);
            out.swap(r.out);
            return true;
        }
        case Encoding::Cp932: {
            DbcsResult r;
            decodeDbcs(data, table_cp932, true, r);
            out.swap(r.out);
            return true;
        }
        case Encoding::Gbk: {
            DbcsResult r;
            decodeDbcs(data, table_gbk, false, r);
            out.swap(r.out);
            return true;
        }
        case Encoding::Big5: {
            DbcsResult r;
            decodeDbcs(data, table_big5, false, r);
            out.swap(r.out);
            return true;
        }
        default:
            return false;
    }
}

const char *Subtitles::name(Encoding enc) {
    switch (enc) {
        case Encoding::Utf8:
            return "utf-8";
        case Encoding::Cp1252:
            return "cp1252";
        case Encoding::Cp949:
            return "cp949";
        case Encoding::Cp932:
            return "cp932";
        case Encoding::Gbk:
            return "gbk";
        case Encoding::Big5:
            return "big5";
        default:
            return "unknown";
    }
}

// ---------------------------------------------------------------------------
// discovery
// ---------------------------------------------------------------------------

bool Subtitles::isTextExtension(const std::string &ext) {
    static const char *exts[] = {"srt", "ass", "ssa", "vtt", "smi", "sami", "sub", "mpl", nullptr};
    for (int i = 0; exts[i]; i++) {
        if (ext == exts[i]) return true;
    }
    return false;
}

static bool readFile(const std::string &path, std::string &data) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0 || st.st_size <= 0 || (size_t) st.st_size > MAX_SUB_SIZE) return false;
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) return false;
    data.resize((size_t) st.st_size);
    size_t r = fread(&data[0], 1, data.size(), f);
    fclose(f);
    data.resize(r);
    return r > 0;
}

static bool writeFile(const std::string &path, const std::string &data) {
    FILE *f = fopen(path.c_str(), "wb");
    if (!f) return false;
    size_t w = fwrite(data.data(), 1, data.size(), f);
    fclose(f);
    return w == data.size();
}

static unsigned hashString(const std::string &s) {
    unsigned h = 2166136261u;
    for (unsigned char c: s) {
        h ^= c;
        h *= 16777619u;
    }
    return h;
}

static std::vector<std::string> listDir(const std::string &dir) {
    std::vector<std::string> names;
    DIR *d = opendir(dir.c_str());
    if (!d) return names;
    while (struct dirent *e = readdir(d)) {
        if (e->d_name[0] == '.') continue;
        names.emplace_back(e->d_name);
    }
    closedir(d);
    std::sort(names.begin(), names.end());
    return names;
}

static bool isDir(const std::string &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

std::vector<Subtitles::Item> Subtitles::find(const std::string &videoPath, const std::string &cacheDir) {
    std::vector<Item> items;

    // local files only (remote listings are not available here)
    if (videoPath.empty() || videoPath[0] != '/') return items;
    size_t slash = videoPath.find_last_of('/');
    std::string dir = videoPath.substr(0, slash + 1);
    std::string file = videoPath.substr(slash + 1);
    size_t dot = file.find_last_of('.');
    std::string stem = lower(dot == std::string::npos ? file : file.substr(0, dot));
    if (stem.empty()) return items;

    std::vector<std::string> dirs = {dir};
    for (const char *sub: {"Subs", "Sub", "Subtitles", "Subtitle", "subs", "sub", "subtitles"}) {
        if (isDir(dir + sub)) dirs.push_back(dir + sub + "/");
    }

    for (auto &d: dirs) {
        std::vector<std::string> names = listDir(d);
        for (auto &name: names) {
            if (items.size() >= MAX_SUBS) return items;
            size_t p = name.find_last_of('.');
            if (p == std::string::npos) continue;
            std::string ext = lower(name.substr(p + 1));
            std::string base = lower(name.substr(0, p));
            bool bitmap = ext == "idx" || ext == "sup";
            if (!bitmap && !isTextExtension(ext)) continue;
            // same name as the video, followed by nothing or a separator (".es", "_eng", " forced")
            if (base.compare(0, stem.size(), stem) != 0) continue;
            if (base.size() > stem.size()) {
                char c = base[stem.size()];
                if (c != '.' && c != '_' && c != '-' && c != ' ' && c != '[' && c != '(') continue;
            }
            // "x.sub" next to "x.idx" is the bitmap data of the VobSub: the .idx is what is loaded
            if (ext == "sub" && std::find(names.begin(), names.end(), name.substr(0, p) + ".idx") != names.end()) {
                continue;
            }
            if (ext == "sub") {
                std::string head;
                if (readFile(d + name, head) && head.size() > 4 && head[0] == 0 && head[1] == 0 && head[2] == 1) {
                    continue; // raw MPEG-PS VobSub without .idx: unusable
                }
            }

            Item it;
            it.path = d + name;
            std::string suffix = name.substr(stem.size(), p - stem.size());
            while (!suffix.empty() && (suffix[0] == '.' || suffix[0] == '_' || suffix[0] == '-' || suffix[0] == ' ')) {
                suffix.erase(0, 1);
            }
            it.title = suffix.empty() ? (ext == "idx" ? std::string("VobSub") : name.substr(p + 1)) : suffix;
            // language: first token of the suffix if it looks like a code
            std::string tok = suffix.substr(0, suffix.find_first_of("._- "));
            if (tok.size() >= 2 && tok.size() <= 3 && std::all_of(tok.begin(), tok.end(), ::isalpha)) {
                it.lang = lower(tok);
            }

            if (bitmap) {
                it.note = ext == "idx" ? "VobSub" : "PGS";
            } else {
                std::string data;
                if (!readFile(it.path, data)) continue;
                Encoding enc = detect(data);
                it.note = Subtitles::name(enc);
                if (enc != Encoding::Utf8) {
                    std::string conv;
                    if (toUtf8(data, enc, conv)) {
                        mkdir(cacheDir.c_str(), 0777);
                        std::string out = cacheDir + std::to_string(hashString(it.path)) + "." + ext;
                        if (writeFile(out, conv)) {
                            it.path = out;
                            it.note = std::string("converted from ") + Subtitles::name(enc);
                        }
                    }
                }
            }
            items.push_back(it);
        }
    }
    return items;
}
