#!/usr/bin/env bash
# Encoding detection/conversion test for src/subtitles.cpp (host, no console needed).
set -euo pipefail
H=$(cd "$(dirname "$0")" && pwd); R=$H/../..
W=$(mktemp -d); cd "$W"
python3 - <<'PY'
import os
os.makedirs('d')
es="1\n00:00:01,000 --> 00:00:03,000\nÑandú comió piñas, canción y más… ¿Qué tal? «Hola» – “ok”\n\n2\n00:00:04,000 --> 00:00:06,000\nÁrbol niño éxito güero maña pingüino\n"
ko="1\n00:00:01,000 --> 00:00:03,000\n안녕하세요. 저는 학생입니다. 오늘은 날씨가 정말 좋네요. 같이 산책하러 갈까요?\n\n2\n00:00:04,000 --> 00:00:06,000\n감사합니다. 다음에 또 만나요. 사랑해요 친구들\n"
ja="1\n00:00:01,000 --> 00:00:03,000\nこんにちは、私は学生です。今日はとても天気がいいですね。一緒に散歩に行きませんか？\n\n2\n00:00:04,000 --> 00:00:06,000\nありがとうございました。また会いましょう。ｱｲｳ\n"
zhs="1\n00:00:01,000 --> 00:00:03,000\n我们没有时间了，这个会议对你来说很重要。他们说我还没过来。为什么不应该发这个？\n"
zht="1\n00:00:01,000 --> 00:00:03,000\n我們沒有時間了，這個會議對你來說很重要。他們說我還沒過來。為什麼不應該發這個？\n"
smi='<SAMI><HEAD><Title>x</Title></HEAD><BODY><SYNC Start=1000><P Class=KRCC>안녕하세요</P></BODY></SAMI>\n'
for n,t,enc in [('es.srt',es,'cp1252'),('ko.srt',ko,'cp949'),('ja.srt',ja,'cp932'),('zhs.srt',zhs,'gbk'),('zht.srt',zht,'cp950'),('ko.smi',smi,'cp949')]:
    open('d/'+n,'wb').write(t.encode(enc)); open('d/'+n+'.want','wb').write(t.encode())
for n,b in [('utf8.srt',es.encode()),('bom.srt',b'\xef\xbb\xbf'+es.encode()),('ascii.srt',b'1\n00:00:01,000 --> 00:00:02,000\nhello\n'),('u16.srt',es.encode('utf-16'))]:
    open('d/'+n,'wb').write(b); open('d/'+n+'.want','wb').write(b'')
PY
g++ -std=c++11 -O1 -Wall -I"$R/src" "$H/test.cpp" "$R/src/subtitles.cpp" "$R/src/subtitle_tables.cpp" -o t
./t d/es.srt d/es.srt.want cp1252 d/ko.srt d/ko.srt.want cp949 d/ja.srt d/ja.srt.want cp932 \
    d/zhs.srt d/zhs.srt.want gbk d/zht.srt d/zht.srt.want big5 d/ko.smi d/ko.smi.want cp949 \
    d/utf8.srt d/utf8.srt.want utf-8 d/bom.srt d/bom.srt.want utf-8 d/ascii.srt d/ascii.srt.want utf-8 \
    d/u16.srt d/u16.srt.want utf-8
echo "subtitle encoding tests passed"
