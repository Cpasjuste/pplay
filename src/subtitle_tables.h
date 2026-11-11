#ifndef PPLAY_SUBTITLE_TABLES_H
#define PPLAY_SUBTITLE_TABLES_H

namespace pplay {
    // double byte code page: entry [(lead - leadMin) * (trailMax - trailMin + 1) + (trail - trailMin)]
    // is the UTF-16 code unit, or 0 when the pair is not assigned
    struct DbcsTable {
        unsigned char leadMin, leadMax, trailMin, trailMax;
        const unsigned short *map;
    };

    extern const DbcsTable table_cp949;
    extern const DbcsTable table_cp932;
    extern const DbcsTable table_gbk;
    extern const DbcsTable table_big5;
}

#endif
