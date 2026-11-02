# PS4 hardware H.264 decoding for pplay

`h264_orbis` is a libavcodec decoder (patch for FFmpeg 5.0, the version PacBrew
ships) that drives the console's own video decoder, `libSceVideodec2`. mpv sees it
as the hwdec method **`orbis-copy`**, so the whole player (subtitles, OSD,
seeking, audio sync) stays unchanged: only the video decoding moves off the CPU.

## How it fits together

```
mpv (libmpv)  --hwdec=orbis-copy
   └─ libavcodec  h264_orbis            ffmpeg-orbis/0001-h264_orbis-decoder.patch
        ├─ h264_mp4toannexb bsf         MKV/MP4 (avcC) → Annex B access units
        ├─ SPS parser                   decides if the hardware can take the stream
        ├─ worker thread (16 MiB stack) every sceVideodec2* call runs here
        ├─ session (process wide)       compute queue + decoder + direct memory, never freed
        └─ NV12 → yuv420p copy          cropped, in ordinary memory
```

* **Subtitles / libass:** decoded pictures are copied into normal FFmpeg frames
  (`ff_get_buffer`) before mpv sees them. No GPU/Garlic/Onion buffer is ever shared
  with the renderer or libass, so subtitle drawing cannot collide with the decoder's
  frame buffers. Picture memory handed to `sceVideodec2Decode` is rotated
  (`dpb + 2` buffers) and read back before the next call.
* **Timestamps:** the decoder returns pictures in display order but without usable
  timestamps; input pts are kept in a sorted queue and the smallest is attached to
  every picture that comes out (correct with B-frames).
* **Memory / lifecycle:** the decoder, the compute queue and all direct memory are
  created when a video is opened (sized for that video, like Moonlight-ps4 and PS4 Cast
  do) and torn down when it is closed, in the order decoder → compute queue → memory.
  Nothing stays allocated between videos: a live compute queue makes the system fault
  when the app is suspended or closed. After a close the next open waits 250 ms.
  If a call into the library ever hangs (10 s) the session is marked broken, memory is
  deliberately leaked instead of freed under a running GPU, and later files use software.
* **Fallback:** only 8-bit 4:2:0 progressive Baseline/Main/High (no Extended, no FMO),
  level ≤ 5.1, ≤ 1920×1088 and ≤ `dpb` (default and maximum 8) reference frames are
  accepted. The first access unit sent to the hardware is an IDR, and if the stream
  changes format in the middle the hardware is left for good. Anything else (Hi10,
  4:2:2, interlaced, 4K...) is refused when the decoder opens and mpv decodes it in
  software automatically. A decoder error at the start of a file does the same.
  A call that hangs for 10 s marks the session broken and later files use software.

* **End of stream:** `sceVideodec2Flush` has not been verified on a console (neither
  reference calls it), so draining is off (`drain=0`): the last 1–3 pictures of a file
  may not be shown. Enable with `--vd-lavc-o=drain=1` once you trust it.

## Using it

**Options menu** (main menu → Options, or LEFT on it): `HW decoding` (auto / off / force),
`SW speed` (quality / balanced / fast) and `TMDB language` are saved to `pplay.cfg` right away.
`HW decoding` and `SW speed` apply the next time pplay is started. Long lists (subtitles,
languages) keep the entry under the cursor in the middle of the menu, with its position
(`12/48`) at the top; a menu opens with the cursor on the entry in use.

Without a `mpv.conf` entry for them, pplay limits the demuxer cache to 64 MiB ahead / 16 MiB
behind (libmpv's default is 150 MiB each way) and does not load fonts embedded in the
file, to keep the console from running out of memory on big 1080p files. Put
`demuxer-max-bytes=...` etc. in `/data/pplay/mpv/mpv.conf` to choose your own values.

Or edit `/data/pplay/pplay.cfg` by hand:

| HWDEC     | meaning |
|-----------|---------|
| `"auto"`  | default. Hardware when possible, software otherwise, with the crash guard |
| `"no"`    | software only |
| `"force"` | like auto, but clears a previous automatic shutdown of the hardware decoder |
| other     | passed to mpv as is (`hwdec=<value>`), no guard |

**Crash guard.** While a file is being hardware decoded for the first seconds,
`/data/pplay/hwdec.guard` exists. If the console stops in that time, the next start
finds it, creates `hwdec.disabled` and uses software decoding. Delete
`hwdec.disabled` (or set `HWDEC="force"`) to try again.

Needs GoldHEN/HEN able to load `/system/common/lib/libSceVideodec2.sprx`. If a
plain module load fails, a GoldHEN jailbreak (syscall 500) is tried once.

## Subtitles: text and image formats

* Text subtitles (srt, ass/ssa, vtt, smi/sami, mpl...) are drawn by the GPU with libass.
* Image subtitles (PGS `hdmv_pgs_subtitle`, VobSub `dvd_subtitle`, `dvb_subtitle`, `xsub`,
  and external `.sup` / `.idx+.sub`) are drawn by the **CPU**: while one of them is the
  active track pplay puts mpv's `sub` video filter in the chain (`format=yuv420p,sub`).
  Reason: the PS4 libmpv only knows 5 precompiled shaders and image subtitles need a
  different one. When mpv has to compile a shader it does not have, it paints the whole
  screen solid blue (its own "broken frame" indicator): that was the blue screen seen when
  turning on a PGS track. The filter is removed again when the track is turned off.

### Automatic subtitles

When a video starts pplay shows the first subtitle track that matches `SUB_LANGS` in
`pplay.cfg` (default `"es-419,es,en,en-GB"`: Latin American Spanish, Spanish, English,
English UK). Turn it off in Options → Auto subtitles (or `SUB_AUTO="off"`). Notes:

* Latin American Spanish is told from Spanish by the language tag (`es-419`, `es-MX`...) or
  the title (`Latino`, `Latinoamérica`, `LATAM`, `es-419`, `lat`...). `Castellano`/`Spain` is plain Spanish.
* A language written in a track title wins over the language tag: mpv reports `eng` for
  MKV tracks that have no language.
* Full tracks are tried first; "forced" and "signs & songs" tracks are only used when no
  full track of any preferred language exists. Commentary tracks are never chosen.
* Among equals: text before picture subtitles, normal before SDH/CC.
* Nothing is chosen when a track is already selected (`slang` in mpv.conf, position saved
  by "watch later") or when no track matches.
* Other languages: `SUB_LANGS="ja,en"`, regions as `pt-BR`, 3 letter codes (`fra`) work too.

## Software decoding speed (Hi10, HEVC, anything the hardware refuses)

`pplay.cfg`: `SW_DECODE_SPEED="quality"` (default) | `"balanced"` (skip the loop filter on
non-reference frames, `vd-lavc-fast`) | `"fast"` (skip the loop filter everywhere and drop
frames in the decoder). Faster, slightly softer picture. HEVC is not hardware decoded:
the PS4 (non-Pro) has no HEVC block, and no HEVC codec type is known for `libSceVideodec2`.

To find out what the console's library accepts, create an empty file
`/data/pplay/hwdec_probe` and play any H.264 8-bit video once: the answer to "do you
take codec types 0..5?" is written to `/data/pplay/hwdec_probe.txt` (only memory
size queries are made, nothing is decoded).

## Subtitles

The PS4 libmpv is built **without iconv**, so mpv cannot convert legacy encodings. pplay
therefore finds external subtitles itself (`src/subtitles.cpp`) and turns text subtitles
that are not UTF-8 into UTF-8 copies in `/data/pplay/subcache/` before giving them to mpv
(`sub-add`); mpv's own `sub-auto` is switched off to avoid duplicates.

| Format | How |
|--------|-----|
| SubRip `.srt`, ASS/SSA `.ass` `.ssa`, WebVTT `.vtt`, SAMI `.smi` `.sami`, MicroDVD/other `.sub` `.txt` | ffmpeg demuxers + libass, text converted to UTF-8 when needed |
| VobSub `.idx` + `.sub` | bitmap, passed through (`.sub` next to an `.idx` is not added twice) |
| PGS `.sup` | bitmap, passed through |
| Embedded in MKV/MP4 (SRT, ASS, WebVTT, PGS, VobSub, mov_text...) | mpv/ffmpeg, no change |

Encodings detected (no iconv needed): UTF-8/UTF-16 (left alone), Windows-1252/Latin-1,
Korean CP949/EUC-KR, Japanese Shift-JIS, Chinese GBK and Big5 (a `charset=` declaration
in the file is honoured first). Files are matched by name (`Movie.mkv` ↔ `Movie.es.srt`,
`Movie.eng.forced.ass`...) in the video folder and in `Subs/`, `Sub/`, `Subtitles/`.
External subtitles are looked up for local files only (not for HTTP/FTP/SMB browsing).
`tests/subtitles/run.sh` checks the detection/conversion on the PC.

## Build

`build-ffmpeg.sh` downloads FFmpeg 5.0, applies the patch, builds it with the
PacBrew configure line and installs it over the toolchain (the CI does this before
building pplay). `test/run.sh` builds the same patch for the PC and checks it against
a mock of libSceVideodec2 (bit-exact frames, pts order with B-frames, seeks,
re-opening, cropping, padding, errors, rejection of unsupported streams).

## Status

The decoder logic is tested on the PC against the mock. The real
`libSceVideodec2` could not be exercised from the build environment, so behaviour on
a console is based on the working code of Moonlight-ps4 and PS4 Cast (structure
layouts, memory types, call sequence). Treat the first runs as experimental.
