pPlay
======

pPlay is a video player for the Nintendo Switch. pPlay support most popular video formats,
have subtitles (embedded ass) and http streaming support.

Installation 
----
Copy "pplay" folder to switch sdcard (/switch/pplay)

Usage
-----
- Use left/right to switch menu's and windows... select a media and... have fun.

Notes
----
- You should really use an NSP HBL (HomeBrew Loader) or any other way to unleash all switch memory 
to launch pPlay, else you'll have some memory problems/crashes. 

- pPlay should be able to play most media. While 720p should be mostly fine, 
1080p support can vary with medias and bit rates. If so (audio de-sync for example), 
increasing buffering can help in most situation. This can be done in the main menu (left).
Please note that increasing the buffer size will have some side effect like longer loading and seeking time,
but also to stop a video (buffers needs to be released). This is more visible
when using http streams, of course.

- pPlay can stream media from an http server with directory listing enabled. To do so,
you must edit the pPlay configuration file (pplay.cfg) to add your server address (NETWORK = "http://samples.ffmpeg.org/";).
Please note that you need a good and stable network connection for this, and you will probably have
to set buffering to "VeryHigh" for smoother experience.

- pPlay will extract media information in background, but i did spot a very few files which
make the device crash. If so, you can disable this in pPlay configuration file (pplay.cfg):
CACHE_MEDIA_INFO = 0;

- pPlay will push the cpu speed when a 1080p media is played, and restore previous speed when paused or stopped.
This is safe but can be disabled in the main option menu (left).

- Please note that there is a (big?) delay when switching video/audio/subtitles streams to see the change.
This is worked on but in the meantime, be patient :)
