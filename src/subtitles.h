//
// External subtitle discovery + text encoding conversion.
//
// The PS4 libmpv is built without iconv, so mpv can neither guess nor convert the
// encoding of text subtitles (.srt/.ass/.ssa/.vtt/.smi/.sami/...) that are not
// UTF-8 (Latin-1/Windows-1252 Spanish/Portuguese/French, Korean SMI, Japanese,
// Chinese...). pplay finds the external subtitles of a video itself, converts the
// ones that need it to UTF-8 (cache folder) and hands them to mpv with "sub-add".
// Bitmap formats (.idx/.sub VobSub, .sup PGS) are passed through untouched.
//

#ifndef PPLAY_SUBTITLES_H
#define PPLAY_SUBTITLES_H

#include <string>
#include <vector>

namespace pplay {

    class Subtitles {
    public:
        struct Item {
            std::string path;   // file to give to mpv (converted copy when needed)
            std::string title;  // shown in the subtitles menu
            std::string lang;   // 2-3 letter language tag or empty
            std::string note;   // "converted from cp1252", "VobSub"... (log only)
        };

        enum class Encoding {
            Utf8,       // ASCII, UTF-8 or UTF-16 with BOM: nothing to do
            Cp1252,
            Cp949,      // Korean
            Cp932,      // Japanese (Shift-JIS)
            Gbk,        // Chinese simplified
            Big5,       // Chinese traditional
            Unknown
        };

        // detect the encoding of a text subtitle
        static Encoding detect(const std::string &data);

        // convert data to UTF-8 (returns false if nothing was converted)
        static bool toUtf8(const std::string &data, Encoding enc, std::string &out);

        static const char *name(Encoding enc);

        // external subtitles next to a local video; converted copies go in cacheDir
        static std::vector<Item> find(const std::string &videoPath, const std::string &cacheDir);

        // true for the text based subtitle extensions we know how to convert
        static bool isTextExtension(const std::string &ext);
    };
}

#endif //PPLAY_SUBTITLES_H
