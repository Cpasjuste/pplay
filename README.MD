pPlay
======


pPlay is a video player for the Nintendo Switch. pPlay support most popular video format, subtitles, http and ftp streaming and more. 
pPlay use the [MPV](https://mpv.io/) interface to handle ffmpeg interoperability, which provide a fast, stable and powerful experience.

Installation 
----
Copy "pplay" folder to switch sdcard (/switch/pplay)

Usage
-----
- Use left/right to switch menu's and windows... select a media and... have fun.
- When playing a media, you can press the "DOWN" or "A" button to bring the UI, the "R" shoulder for fast forward ("L" button to stop).

Notes
----
- You should either use an NSP HBL (HomeBrew Loader) to unleash all switch memory to launch pPlay.
Alternatively if you are using Atmosphere, you can press (R) when launching a game, it will load HBL with all memory unlocked.

- pPlay should be able to play most media. While 720p and 1080p medias should be fine, you can unlock all the switch cpu speed for heavy medias.

- pPlay can stream media from an http server with directory listing enabled, or an ftp server. To do so,
you must edit the configuration file (pplay.cfg) to add your server address:
  - NETWORK = "http://samples.ffmpeg.org/";
  - NETWORK = "ftp://user:password@ip:port/";

- As pPlay use [MPV](https://mpv.io/), you can put a file named "mpv.conf" in the "mpv" folder to play with. 
See the [CONFIGURATION FILES](https://mpv.io/manual/master/#configuration-files) and [OPTIONS](https://mpv.io/manual/master/#options) sections in the [MPV](https://mpv.io/manual/master/#) manual for more informations. 
