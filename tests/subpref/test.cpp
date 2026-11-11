// Automatic subtitle choice (src/subtitle_pref.cpp): order "es-419,es,en,en-GB".
#include <cstdio>
#include "subtitle_pref.h"

using namespace pplay;
typedef SubtitlePref::Track T;

static int failures = 0;

static T t(int id, const char *lang, const char *title, const char *codec = "subrip") {
    T x; x.id = id; x.lang = lang; x.title = title; x.codec = codec; return x;
}

static void expect(const char *name, const std::vector<T> &tracks, int want, const char *prefs = "es-419,es,en,en-GB") {
    int got = SubtitlePref::choose(tracks, SubtitlePref::parse(prefs));
    if (got != want) {
        failures++;
        printf("FAIL %-48s got %d want %d\n", name, got, want);
    } else {
        printf("ok   %-48s -> %d\n", name, got);
    }
}

int main() {
    // the order of the user: Latin American Spanish > Spanish > English > English (UK)
    expect("latino beats spain beats english",
           {t(1, "eng", "English"), t(2, "spa", "Spanish (Spain)"), t(3, "spa", "Spanish (Latin America)")}, 3);
    expect("spanish when there is no latino",
           {t(1, "eng", "English"), t(2, "spa", "Español")}, 2);
    expect("english when there is no spanish",
           {t(1, "jpn", "Japanese"), t(2, "eng", "English")}, 2);
    expect("plain english before english UK",
           {t(1, "eng", "English (UK)"), t(2, "eng", "English")}, 2);
    expect("english UK alone is still english",
           {t(1, "jpn", ""), t(2, "eng", "English (UK)")}, 2);
    expect("nothing matches", {t(1, "jpn", "Japanese"), t(2, "fre", "Français")}, -1);
    expect("no tracks", {}, -1);

    // how the files label things
    expect("tag es-419", {t(1, "eng", "English"), t(2, "es-419", "")}, 2);
    expect("tag es-MX", {t(1, "es", "Spanish"), t(2, "es-MX", "")}, 2);
    expect("title Latino, language missing", {t(1, "eng", "English"), t(2, "N/A", "Español Latino")}, 2);
    expect("title wins over mpv's default eng", {t(1, "eng", "Español Latino"), t(2, "eng", "English")}, 1);
    expect("uppercase accents", {t(1, "spa", "ESPAÑA"), t(2, "spa", "ESPAÑOL LATINO")}, 2);
    expect("castellano is not latino", {t(1, "spa", "Castellano"), t(2, "spa", "Latinoamericano")}, 2);
    expect("latam", {t(1, "spa", "Spanish"), t(2, "spa", "Spanish [LATAM]")}, 2);
    expect("external movie.lat.srt", {t(1, "es", "es"), t(2, "lat", "lat")}, 2);
    expect("external movie.es-419.srt", {t(1, "es", "es"), t(2, "es", "es-419")}, 2);
    expect("external movie.Spanish.Latino.srt", {t(1, "", "English"), t(2, "", "Spanish.Latino")}, 2);

    // forced / signs & songs / commentary
    expect("full spanish beats forced latino",
           {t(1, "spa", "Latino forced"), t(2, "spa", "Español")}, 2);
    expect("full english beats forced spanish",
           {t(1, "spa", "Forced"), t(2, "eng", "English")}, 2);
    expect("forced alone is better than nothing", {t(1, "jpn", ""), t(2, "spa", "Forced")}, 2);
    expect("signs & songs last", {t(1, "eng", "Signs & Songs"), t(2, "eng", "Dialogue")}, 2);
    expect("commentary never", {t(1, "spa", "Commentary"), t(2, "eng", "English")}, 2);
    expect("only commentary", {t(1, "spa", "Comentarios del director")}, -1);

    // same tier: text before pictures, normal before SDH
    expect("text before PGS",
           {t(1, "spa", "Latino", "hdmv_pgs_subtitle"), t(2, "spa", "Latino", "subrip")}, 2);
    expect("latino PGS still beats spanish srt",
           {t(1, "spa", "Español", "subrip"), t(2, "spa", "Latino", "hdmv_pgs_subtitle")}, 2);
    expect("normal before SDH", {t(1, "eng", "English SDH"), t(2, "eng", "English")}, 2);
    expect("first of equals", {t(5, "eng", "English"), t(6, "eng", "English")}, 5);
    expect("dvd_subtitle is a picture",
           {t(1, "spa", "", "dvd_subtitle"), t(2, "spa", "", "ass")}, 2);

    // other lists
    expect("custom: japanese first", {t(1, "eng", "English"), t(2, "jpn", "Japanese")}, 2, "ja,en");
    expect("custom: pt-BR", {t(1, "por", "Portugal"), t(2, "por", "pt-BR")}, 2, "pt-br");
    expect("custom: three letter code", {t(1, "eng", "English"), t(2, "fre", "Français")}, 2, "fra,eng");
    expect("empty preference list", {t(1, "eng", "English")}, -1, "");
    expect("spaces and case in the list", {t(1, "eng", "English")}, 1, " ES-419 ;  EN ");

    auto p = SubtitlePref::parse(" ES-419,es ;;en , en-GB");
    if (p.size() != 4 || p[0] != "es-419" || p[3] != "en-gb") { failures++; printf("FAIL parse\n"); }
    if (SubtitlePref::language(t(1, "spa", "")) != "es" || SubtitlePref::language(t(1, "und", "")) != "") {
        failures++; printf("FAIL language\n");
    }

    if (failures) { printf("%d subtitle preference test(s) failed\n", failures); return 1; }
    printf("subtitle preference tests passed\n");
    return 0;
}
