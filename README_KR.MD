pPlay
======


pPlay는 닌텐도 스위치의 비디오 플레이어입니다. pPlay는 가장 인기있는 비디오 포맷, 자막, http, ftp 스트리밍 등을 지원합니다.
pPlay는 [MPV](https://mpv.io/) 인터페이스를 사용하여 빠르고 안정적이며 강력한 경험을 제공하는 ffmpeg 상호 운용성을 처리합니다.

설치 
----
"pplay" 폴더를 복사하여 SD 카드로 전환하십시오 (/switch/pplay)

사용법
-----
- 왼쪽/오른쪽으로 메뉴 및 창 전환... 미디어를 선택하고... 즐기세요.
- When playing a media, you can press the "DOWN" or "A" button to bring the UI, the "R" shoulder for fast forward ("L" button to stop).
- 미디어를 재생할 때 "DOWN" 또는 "A" 버튼을 누르면 UI가 표시되고 "R" 버튼은 빨리 감기 입니다 ("L" 버튼은 중지).

참고
----
- NSP HBL (홈브류 로더)을 사용하여 pPlay를 시작하기 위해 모든 스위치 메모리를 해제해야합니다.
또는 Atmosphere를 사용하는 경우 게임을 시작할 때 (R)을 누를 수 있습니다. 그러면 모든 메모리가 잠금 해제된 HBL이 로드됩니다.

- 재생은 대부분의 미디어를 재생할 수 있어야 합니다. 720p 및 1080p 미디어가 정상적으로 작동해야하는 동안 메인 옵션 메뉴 (왼쪽 버튼)에서 무거운 미디어에 대한 모든 스위치 CPU 속도를 해제할 수 있습니다.

- pPlay는 디렉토리 목록이 활성화 된 http 서버 또는 ftp 서버에서 미디어를 스트리밍 할 수 있습니다. 그렇게하기 위해,
서버 주소를 추가하려면 구성 파일 (pplay.cfg)을 편집해야합니다:
  - NETWORK = "http://samples.ffmpeg.org/";
  - NETWORK = "ftp://user:password@ip:port/";

- pPlay는 [MPV](https://mpv.io/) 를 사용하기 때문에 "mpv" 폴더에 "mpv.conf" 라는 파일을 넣어 재생할 수 있습니다.
보다 많은 정보는 [구성 파일](https://mpv.io/manual/master/#configuration-files) 과 [옵션](https://mpv.io/manual/master/#options) 섹션의 [MPV](https://mpv.io/manual/master/#) 설명서를 참조하세요. 
