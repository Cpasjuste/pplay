//
// Automatic subtitle choice: pick the subtitle track to show when a video is loaded, following
// an order of preference ("es-419,es,en,en-GB": Latin American Spanish, Spanish, English,
// English UK). mpv's own "slang" only knows language codes, but the fansub/BD files that carry
// "Spanish (Latin America)" vs "Spanish (Spain)" tracks tell them apart by title, so the title
// is looked at too. Pure code (no mpv, no UI) so it can be tested on a PC.
//

#ifndef PPLAY_SUBTITLE_PREF_H
#define PPLAY_SUBTITLE_PREF_H

#include <string>
#include <vector>

namespace pplay {

    class SubtitlePref {
    public:
        struct Track {
            int id = 0;
            std::string lang;   // as reported by mpv: "spa", "es", "es-419", "N/A", ""...
            std::string title;
            std::string codec;  // "subrip", "ass", "hdmv_pgs_subtitle"...
        };

        // "es-419, es,EN,en-GB" -> {"es-419", "es", "en", "en-gb"} (empty entries dropped)
        static std::vector<std::string> parse(const std::string &list);

        // id of the track to show or -1 when no track matches any preference.
        // Full subtitle tracks are tried first, "forced"/"signs & songs" ones only if there
        // is no full track in any preferred language; commentary tracks are never chosen.
        static int choose(const std::vector<Track> &tracks, const std::vector<std::string> &prefs);

        // 2 letter language of a track ("es", "en", "ja"...) or "" when unknown
        static std::string language(const Track &track);

        // true for picture subtitles (PGS, VobSub, DVB): drawn by the CPU, text ones are nicer
        static bool isBitmap(const std::string &codec);
    };
}

#endif //PPLAY_SUBTITLE_PREF_H
