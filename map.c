#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#define 제거 / map 크기 저장, 초기값=0
int map_width=0; // 가로
int map_height=0; // 세로
int max_stages=0; // 총 스테이지

// 3중 포인터 (스테이지, 세로, 가로)
char ***map = NULL;

// malloc, 스테이지->세로->가로 / map[s][r][c]
void load_maps(){
	FILE *file = fopen("map.txt", "r"); // 파일이 없으면 열기 실패
	if (file == NULL) {
		perror("map.txt 파일을 열 수 없습니다.");

	       // perror: 오류 시 구체적인 이유를 출력함, 포인터가 필요x	
	       // ferror: 파일 입출력시 발생하는 오류 확인, 오류 발생 안하면 0을 반환, 발생하면 0이 아닌 값을 반환
		exit(1);
	}

if(fscanf(file, "%d %d %d", &map_width, &map_height, &max_stages) != 3) {
	printf("map.txt 파일에 크기 정보가 없습니다.");
	exit(1);
}
//fscanf 후 버퍼에 남은 \n 제거
while (fgetc(file) != '\n');

// 메모리 동적 할당
// stage 목록 생성
map = (char ***)malloc(sizeof(char **) * max_stages);
if(map == NULL) {perror("메모리 할당 실패"); exit(1); }

for (int s = 0; s < max_stages; s++) {
	// 각 스테이지의 height(세로/행) 생성
	map[s]=(char **)malloc(sizeof(char *) * map_height);
	if (map[s] == NULL) {perror("메모리 할당 실패"); exit(1); }

	for (int r = 0;  r <map_height; r++) {
		// 각 행의 width 생성, +1: NULL문자
		map[s][r] = (char *)malloc(sizeof(char) * (map_width+1));
		if (map[s][r] == NULL) {perror("메모리 할당 실패"); exit(1); }

		memset(map[s][r], ' ', map_width);
		map[s][r][map_width] = '\0';
				}

			}

// map.txt의 map 데이터 읽기
char *line_buf = (char *)malloc(sizeof(char) * (map_width+10));

int s=0; // 현재 읽는 stage
int r=0; // 현재 읽는 row(행)

// 임시버퍼: 개행문자 '\n', NULL 처리
    while (fgets(line_buf, map_width + 10, file) != NULL) {
        if (s >= max_stages) break;

	// [수정]
	line_buf[strcspn(line_buf, "\r\n")] = '\0';

	 if (strlen(line_buf)==0) {
            if (r > 0) { // 데이터를 읽던 도중에 빈 줄을 만남 -> 스테이지 조기 종료
                while (r < map_height) {
                    for (int i = 0; i < map_width; i++) {
                        map[s][r][i] = ' '; // 남은 공간 공백 채우기
                    }
                    map[s][r][map_width] = '\0';
                    r++;
                }
                s++; 
                r = 0;
            }
            continue; 
        }

	strncpy(map[s][r], line_buf, map_width);
	map[s][r][map_width] = '\0';

	int len = strlen(line_buf);
		if(len<map_width) {
			memset(map[s][r] + len, ' ', map_width - len);
		}

      
        r++;

        if (r >= map_height) {
            s++;
            r = 0;
        }
    }
free(line_buf);
fclose(file);
}


// 메모리 해제 
void free_maps() {
	if (map == NULL) return;

	for(int s=0; s<max_stages; s++) {
		for(int r=0; r<map_height; r++) {
			free(map[s][r]); // 가로 해제

		}
		free(map[s]); // 세로 해제

	}
	free(map); // 스테이지 해제

}


