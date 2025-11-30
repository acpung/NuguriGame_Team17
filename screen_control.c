#include "screen_control.h"

#ifdef _WIN32

    void enable_ansiEscapeCode_in_window(){
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD ansiEscapeCode;
        GetConsoleMode(hStdOut, &ansiEscapeCode);
        SetConsoleMode(hStdOut, ansiEscapeCode | (ENABLE_VIRTUAL_TERMINAL_PROCESSING));
    }

    void disable_ansiEscapeCode_in_window(){
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD ansiEscapeCode;
        GetConsoleMode(hStdOut, &ansiEscapeCode);
        SetConsoleMode(hStdOut, ansiEscapeCode | ~(ENABLE_VIRTUAL_TERMINAL_PROCESSING));
    }

    void disable_cursor(){
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(hStdOut, &info);
    }

    void enable_cursor(){
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = TRUE;
        SetConsoleCursorInfo(hStdOut, &info);
    }

#endif