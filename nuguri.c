#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 윈도우일 경우와 리눅스의 경우 다른 코드 사용을 위한 분리
#ifdef _WIN32
    #include <windows.h> // sleep, setConsoleMode 함수 사용
    #include <conio.h> // _khbit 함수 사용
    #include "screen_control.h"
#else
  #include <unistd.h>
  #include <termios.h>
  #include <fcntl.h>
#endif

// 맵 및 게임 요소 정의 (수정된 부분)
#define MAP_WIDTH 40  // 맵 너비를 40으로 변경
#define MAP_HEIGHT 20
#define MAX_STAGES 2
#define MAX_ENEMIES 15 // 최대 적 개수 증가
#define MAX_COINS 30   // 최대 코인 개수 증가

#ifdef _WIN32
    #define getchar() _getch()
#endif

// 구조체 정의
typedef struct {
    int x, y;
    int dir; // 1: right, -1: left
} Enemy;

typedef struct {
    int x, y;
    int collected;
} Coin;

// 전역 변수
char map[MAX_STAGES][MAP_HEIGHT][MAP_WIDTH + 1];
int player_x, player_y;
int stage = 0;
int score = 0;

// 플레이어 상태
int is_jumping = 0;
int velocity_y = 0;
int on_ladder = 0;

// 게임 객체
Enemy enemies[MAX_ENEMIES];
int enemy_count = 0;
Coin coins[MAX_COINS];
int coin_count = 0;

// 터미널 설정
#ifdef _WIN32
    DWORD orig = 0;
#else
    struct termios orig_termios;
#endif


// 함수 선언
void disable_raw_mode();
void enable_raw_mode();
void load_maps();
void init_stage();
void draw_game();
void update_game(char input);
void move_player(char input);
void move_enemies();
void check_collisions();
int kbhit();

int main() {

    #ifdef _WIN32
        SetConsoleOutputCP(65001); // 콘솔에서 출력을 utf-8로 하도록 만듦. main실행되자 마자 바로 utf-8로 바꿔야 나머지 출력도 다 적용됨.
        //SetConsoleCP(65001) -> 콘솔에서 출력할 때 code page를 설정하는 역할을 함. / 65001 -> UTF-8을 나타내는 코드페이지 번호임.
        // 이 아래는 printf("\x1b[2J\x1b[H"); 같은 안시 이스케이프 코드를 윈도우에서도 쓸 수 있게 해줌.
        enable_ansiEscapeCode_in_window();
        //이거는 커서 보이기 문제임
        disable_cursor();
    #endif
    
    printf("\x1b[2J\x1b[H"); // 화면 지우고 게임 출력하기
    srand(time(NULL));
    enable_raw_mode();
    load_maps();
    init_stage();

    char c = '\0';
    int game_over = 0;

    while (!game_over && stage < MAX_STAGES) {
        if (kbhit()) {
            #ifdef _WIN32
                c = getchar();
                if (c == 'q') {
                    game_over = 1;
                    continue;
                }
                if(c == -32 || c == 224){
                    switch (getchar()) {
                        case 72: c = 'w'; break; // Up
                        case 80: c = 's'; break; // Down
                        case 77: c = 'd'; break; // Right
                        case 75: c = 'a'; break; // Left
                    }
                }
            #else
                c = getchar();
                if (c == 'q') {
                    game_over = 1;
                    continue;
                }
                if (c == '\x1b') {
                getchar(); // '['
                switch (getchar()) {
                    case 'A': c = 'w'; break; // Up
                    case 'B': c = 's'; break; // Down
                    case 'C': c = 'd'; break; // Right
                    case 'D': c = 'a'; break; // Left
                }
            }
            #endif

        } else {
            c = '\0';
        }

        update_game(c);
        draw_game();

        #ifdef _WIN32
            Sleep(90);
        #else
            usleep(90000);
        #endif
        

        if (map[stage][player_y][player_x] == 'E') {
            stage++;
            score += 100;
            if (stage < MAX_STAGES) {
                init_stage();
            } else {
                game_over = 1;
                printf("\x1b[2J\x1b[H");
                printf("축하합니다! 모든 스테이지를 클리어했습니다!\n");
                printf("최종 점수: %d\n", score);
            }
        }
    }

    disable_raw_mode();

    #ifdef _WIN32
        disable_ansiEscapeCode_in_window();
        enable_cursor();
    #endif

    return 0;
}


// 터미널 Raw 모드 활성화/비활성화
void disable_raw_mode() {
    #ifdef _WIN32
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), orig & (ENABLE_ECHO_INPUT));
    #else
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    #endif
}

void enable_raw_mode() {
    #ifdef _WIN32
        HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hStdIn, &orig);
        DWORD new = orig;
        SetConsoleMode(hStdIn, new & ~(ENABLE_ECHO_INPUT));
    #else
        tcgetattr(STDIN_FILENO, &orig_termios);
        atexit(disable_raw_mode);
        struct termios raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    #endif
}

// 맵 파일 로드
void load_maps() {
    FILE *file = fopen("map.txt", "r");
    if (!file) {
        perror("map.txt 파일을 열 수 없습니다.");
        exit(1);
    }
    int s = 0, r = 0;
    char line[MAP_WIDTH + 2]; // 버퍼 크기는 MAP_WIDTH에 따라 자동 조절됨
    while (s < MAX_STAGES && fgets(line, sizeof(line), file)) {
        if ((line[0] == '\n' || line[0] == '\r') && r > 0) {
            s++;
            r = 0;
            continue;
        }
        if (r < MAP_HEIGHT) {
            line[strcspn(line, "\n\r")] = 0;
            strncpy(map[s][r], line, MAP_WIDTH + 1);
            r++;
        }
    }
    fclose(file);
}


// 현재 스테이지 초기화
void init_stage() {
    enemy_count = 0;
    coin_count = 0;
    is_jumping = 0;
    velocity_y = 0;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char cell = map[stage][y][x];
            if (cell == 'S') {
                player_x = x;
                player_y = y;
            } else if (cell == 'X' && enemy_count < MAX_ENEMIES) {
                enemies[enemy_count] = (Enemy){x, y, (rand() % 2) * 2 - 1};
                enemy_count++;
            } else if (cell == 'C' && coin_count < MAX_COINS) {
                coins[coin_count++] = (Coin){x, y, 0};
            }
        }
    }
}

// 게임 화면 그리기
void draw_game() {
    printf("\x1b[H"); //깜빡임 없애기 위해 \x1b[2J를 삭제함
    printf("Stage: %d | Score: %d\n", stage + 1, score);
    printf("조작: ← → (이동), ↑ ↓ (사다리), Space (점프), q (종료)\n");

    char display_map[MAP_HEIGHT][MAP_WIDTH + 1];
    for(int y=0; y < MAP_HEIGHT; y++) {
        for(int x=0; x < MAP_WIDTH; x++) {
            char cell = map[stage][y][x];
            if (cell == 'S' || cell == 'X' || cell == 'C') {
                display_map[y][x] = ' ';
            } else {
                display_map[y][x] = cell;
            }
        }
    }
    
    for (int i = 0; i < coin_count; i++) {
        if (!coins[i].collected) {
            display_map[coins[i].y][coins[i].x] = 'C';
        }
    }

    for (int i = 0; i < enemy_count; i++) {
        display_map[enemies[i].y][enemies[i].x] = 'X';
    }

    display_map[player_y][player_x] = 'P';

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for(int x=0; x< MAP_WIDTH; x++){
            printf("%c", display_map[y][x]);
        }
        printf("\n");
    }
}

// 게임 상태 업데이트
void update_game(char input) {
    move_player(input);
    move_enemies();
    check_collisions();
}

// 플레이어 이동 로직
void move_player(char input) {
    int next_x = player_x, next_y = player_y;
    char floor_tile = (player_y + 1 < MAP_HEIGHT) ? map[stage][player_y + 1][player_x] : '#';
    char current_tile = map[stage][player_y][player_x];

    on_ladder = (current_tile == 'H');

    switch (input) {
        case 'a': next_x--; break;
        case 'd': next_x++; break;
        case 'w': if (on_ladder) next_y--; break;
        case 's': if (on_ladder && (player_y + 1 < MAP_HEIGHT) && map[stage][player_y + 1][player_x] != '#') next_y++; break;
        case ' ':
            if (!is_jumping && (floor_tile == '#' || on_ladder)) {
                is_jumping = 1;
                velocity_y = -3; // 점프 시 한 칸씩 이동하도록 바꾸었기에 총 3칸을 점프시키기 위해 -2에서 -3으로 바꿈
            }
            break;
    }

    // 속도가 있는 경우(점프 한 경우)에 다음 위치의 좌우 이동이 벽인지 감지하는 문장이 없어 추가함
    // 속도가 없는 경우 중 떨어지는 경우에도 다음 위치의 좌우 이동을 벽인지 감지하도록 추가함
    if(velocity_y > 0){
        if (next_x >= 0 && next_x < MAP_WIDTH && map[stage][player_y][next_x] != '#' && map[stage][player_y+1][next_x] != '#') player_x = next_x;
    } else if(velocity_y > 0){
        if (next_x >= 0 && next_x < MAP_WIDTH && map[stage][player_y][next_x] != '#' && map[stage][player_y-1][next_x] != '#') player_x = next_x;
    } else if (floor_tile != '#' && floor_tile != 'H') {
        if (map[stage][player_y+1][next_x] != '#') player_x = next_x;
    } else {
        if (next_x >= 0 && next_x < MAP_WIDTH && map[stage][player_y][next_x] != '#') player_x = next_x;
    }
    
    if (on_ladder && (input == 'w' || input == 's')) {
        if(next_y >= 0 && next_y < MAP_HEIGHT && map[stage][next_y][player_x] != '#') {
            player_y = next_y;
            is_jumping = 0;
            velocity_y = 0;
        }
    } 
    else {
        if (is_jumping) {
            next_y = player_y + (velocity_y > 0 ? 1 : (velocity_y < 0 ? -1 : 0)); // 기울기 확인 후 1씩 이동하도록 수정
            if(next_y < 0) next_y = 0;

            if (velocity_y < 0 && next_y < MAP_HEIGHT && map[stage][next_y][player_x] == '#') {
                velocity_y = 0;
            } else if (next_y < MAP_HEIGHT) {
                player_y = next_y;
            }
            
            if ((player_y + 1 < MAP_HEIGHT) && map[stage][player_y + 1][player_x] == '#') {
                is_jumping = 0;
                velocity_y = 0;
            } else {
                velocity_y++; // 조건 검사 후 기울기를 증가시키도록 수정
            }
        } else {
            if (floor_tile != '#' && floor_tile != 'H') {
                 if (player_y + 1 < MAP_HEIGHT) player_y++;
                 else init_stage();
            }
        }
    }
    
    if (player_y >= MAP_HEIGHT) init_stage();
}


// 적 이동 로직
void move_enemies() {
    for (int i = 0; i < enemy_count; i++) {
        int next_x = enemies[i].x + enemies[i].dir;
        if (next_x < 0 || next_x >= MAP_WIDTH || map[stage][enemies[i].y][next_x] == '#' || (enemies[i].y + 1 < MAP_HEIGHT && map[stage][enemies[i].y + 1][next_x] == ' ')) {
            enemies[i].dir *= -1;
        } else {
            enemies[i].x = next_x;
        }
    }
}

// 충돌 감지 로직
void check_collisions() {
    for (int i = 0; i < enemy_count; i++) {
        if (player_x == enemies[i].x && player_y == enemies[i].y) {
            score = (score > 50) ? score - 50 : 0;
            init_stage();
            return;
        }
    }
    for (int i = 0; i < coin_count; i++) {
        if (!coins[i].collected && player_x == coins[i].x && player_y == coins[i].y) {
            coins[i].collected = 1;
            score += 20;
        }
    }
}

// 비동기 키보드 입력 확인
int kbhit() {
    #ifdef _WIN32
        return _kbhit();
    #else
        struct termios oldt, newt;
        int ch;
        int oldf;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        if(ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }
        return 0;
    #endif
}