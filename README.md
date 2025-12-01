<h1 style="text-align:center">고급자료구조 너구리게임 17팀 보고서</h1>

---

# 1. 프로젝트 정보

- **과제명:** Git 협업을 통한 크로스플랫폼 기반 너구리게임 구현
- **제출팀:** 고급자료구조 17팀
- **팀원명:** 20223108 박지환 (팀장),
              20243085 구본승,
              20243124 이채은,
              20243145 허진서
- **제출일:** 2025년 12월 3일
---

# 2. GitHub 주소
- **저장소:** https://github.com/acpung/NuguriGame_Team17
---

# 3. OS별 컴파일 및 실행 방법 가이드
- **기본 전제:** 모든 크로스플랫폼에서 gcc 컴파일러와 make를 사용하여 실행함
  
                 기본 window의 CMD는 이를 인식할 수 없기에
                 MinGW-w64를 사용한 터미널이 필요함
- **Windows**
  - 컴파일 : make
  - 실행 : ./nuguri.exe
  - 삭제 : make clean
- **Mac**
  - 컴파일 : make
  - 실행 : ./diablo
  - 삭제 : make clean
- **Linux**
  - 컴파일 : make
  - 실행 : ./diablo
  - 삭제 : make clean
---

# 4. 구현 기능 리스트 및 게임 스크린샷

- **전체 게임 화면**

  <img width="200" height="300" alt="image" src="https://github.com/user-attachments/assets/348a3419-d1ba-478f-9d9c-912468c41c26" />

- **크로스 플랫폼 지원**
- **생명력 시스템**
  
  <img width="82" height="18" alt="image" src="https://github.com/user-attachments/assets/f57e2948-b79a-4a3a-ac80-5fefead1669b" />

- **타이틀 및 엔딩 화면**
  - 타이틀 화면

    <img width="200" height="300" alt="image" src="https://github.com/user-attachments/assets/6858e59d-4657-4431-bf40-3b64ed64195f" />

  - 엔딩 화면
 
    <img width="200" height="300" alt="image" src="https://github.com/user-attachments/assets/57398e85-d004-49fa-81e9-15d8db7fae13" />

  - 사망 화면
 
    <img width="200" height="300" alt="image" src="https://github.com/user-attachments/assets/1d906db3-18d2-44d1-8ea3-4d13e0b96250" />

- **동적 맵 할당**
- **사운드 효과**
---

# 5. 개발 중 발생한 OS 호환성 문제와 해결 과정 기술
- **헤더파일 분리 문제**

전처리기를 사용하여 윈도우에서만 사용되는 헤더파일과 리눅스에서만 사용되는 헤더파일을 분리함.
이 과정에서 에러가 뜬 khbit, enable_raw_mode, disable_row_mode를 수정함

    1. khbit : 비동기 키보드 입력 확인
    리눅스와 달리 윈도우는 khbit가 conio(control input output) 헤더파일에 저장되어 있기에 _khbit() 함수를 사용함

    2. enable_raw_mode : 키를 입력하면 화면에 출력하는 ECHO 설정과 엔터를 누를 때까지 입력을 받는 ICANON 설정을 끔
     
     리눅스에서는 unistd 헤더파일을 통해 키보드의 상수 값을 얻어오고, termios의 tcsetattr을 통해 터미널 속성을 변경함.
     
     그러나 윈도우에서는 termios와 unistd를 사용하지 않기에 이를 windows 헤더파일로 대체함
     
     - GetStdHandle 함수를 사용하여 키보드의 제어권을 hStdIn 변수에 저장함
     - GetConsoleMode를 통해 콘솔의 설정을 orig 변수에 저장함
       (이때 프로그램 종료 시 기존 설정으로 돌아가기 위해 전역변수 orig에 저장함)
     - SetConsoleMode를 통해 ECHO와 LINE_INPUT(엔터를 쳐야 입력되는 기능)을 끔

       -> 그러나 LINE_INPUT이 제대로 작동하지 않아 엔터를 쳐야 입력이 가능한 문제가 생김

       -> 이에 윈도우의 경우 conio의 _getch() 함수를 getchar() 대신 사용하여 키를 즉시 입력받는 문제를 해결함

     3. disable_raw_mode : raw_mode 비활성화

        - enable_raw_mode에서 저장해 둔 orig를 사용하여 ECHO와 LINE_INPUT을 다시 설정함


- **방향키 입력 문제**
  
리눅스에서는 방향키를 입력하면 \x1b + [ + A, B, C, D가 입력 버퍼에 저장됨

그러나 윈도우에서 방향키를 입력하면 224 + 72, 80, 77, 75가 입력 버퍼에 저장됨

    1. 이에 방향키 입력을 전처리기를 통해 경우를 나눠서 분리함
     
     -> 그러나 윈도우에서 방향키가 입력되지 않는 오류 발생

    2. unsigned char 기준으로는 224이지만 현 컴파일러의 char는 signed char로 저장되기에 224에 2의 보수를 취해 -32로 표현해야 한다는 사실을 발견 후 수정함


- **usleep 함수**

  리눅스에서는 usleep 함수를 사용하지만, 윈도우에서는 sleep 함수를 사용하기에 이를 수정함


- **윈도우 글자 깨짐 문제**

코드는 utf-8로 저장되어 있지만, 윈도우는 콘솔에서 출력을 EUC-KR로 인코딩함. 

이에 한글이 깨지는데, 이를 해결하기 위해 windows 헤더파일의 SetConsoleOutputCP 함수를 사용하여 콘솔의 출력을 utf-8로 설정함


- **윈도우 화면, 커서 깜빡임 문제**

리눅스의 \x1b[2J를 대체하기 위해 윈도우에서 clrscr()함수를 사용하니 화면과 커서가 깜빡임

    1. windows의 SetConsoleMode에 ENABLE_VIRTUAL_TERMINAL_PROCESSING 을 통해 리눅스의 안시 이스케이프 코드를 사용할 수 있는 것을 확인 후 수정함
     
     -> 그러나 깜빡임은 지속됨

    2. draw_game 함수는 전체 화면을 지우고 다시 그리는 함수임
     
     - 이에 전체 화면 지우는 \x1b[2J 를 삭제하고 기본 화면을 덮어써서 출력하도록 수정하여 해결함
    
     -> 커서가 화면 전체를 거쳐가며 깜빡이는 문제가 남아있음

    3. windows의 CONSOLE_CURSOR_INFO 구조체의 visible 변수와 SetConsoleCursorInfo 함수를 통해 커서가 안 보이도록 설정함

     -> 그러나 실행 시 문제가 그대로임

    4. CONSOLE_CURSOR_INFO 구조체의 dwSize 변수는 기본적으로 초기화가 안 되어있어 발생한 오류임을 확인하고 초기화함 (dwSize는 커서 크기 백분율)

