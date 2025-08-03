#pragma once

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdio>
#include <unistd.h>
#include <termios.h>
#endif

namespace DotBlue {
enum class ConsoleColour {
    Default,
    Red,
    Green,
    Blue,
    Yellow,
    Cyan,
    Magenta,
    White,
    BrightRed,
    BrightGreen,
    BrightBlue
};

class ColourConsole {
public:
    static void setColour(ConsoleColour colour) {
#ifdef _WIN32
        static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        WORD attr = 0;

        switch (colour) {
            case ConsoleColour::Default:     attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
            case ConsoleColour::Red:         attr = FOREGROUND_RED; break;
            case ConsoleColour::Green:       attr = FOREGROUND_GREEN; break;
            case ConsoleColour::Blue:        attr = FOREGROUND_BLUE; break;
            case ConsoleColour::Yellow:      attr = FOREGROUND_RED | FOREGROUND_GREEN; break;
            case ConsoleColour::Cyan:        attr = FOREGROUND_GREEN | FOREGROUND_BLUE; break;
            case ConsoleColour::Magenta:     attr = FOREGROUND_RED | FOREGROUND_BLUE; break;
            case ConsoleColour::White:       attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
            case ConsoleColour::BrightRed:   attr = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
            case ConsoleColour::BrightGreen: attr = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
            case ConsoleColour::BrightBlue:  attr = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        }

        SetConsoleTextAttribute(hConsole, attr);
#else
        const char* code = "";

        switch (colour) {
            case ConsoleColour::Default:     code = "\033[0m"; break;
            case ConsoleColour::Red:         code = "\033[31m"; break;
            case ConsoleColour::Green:       code = "\033[32m"; break;
            case ConsoleColour::Blue:        code = "\033[34m"; break;
            case ConsoleColour::Yellow:      code = "\033[33m"; break;
            case ConsoleColour::Cyan:        code = "\033[36m"; break;
            case ConsoleColour::Magenta:     code = "\033[35m"; break;
            case ConsoleColour::White:       code = "\033[37m"; break;
            case ConsoleColour::BrightRed:   code = "\033[91m"; break;
            case ConsoleColour::BrightGreen: code = "\033[92m"; break;
            case ConsoleColour::BrightBlue:  code = "\033[94m"; break;
        }

        std::cout << code;
#endif
    }

    static void reset() {
        setColour(ConsoleColour::Default);
    }

    static void gotoxy(int x, int y) {
#ifdef _WIN32
        COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
        std::printf("\033[%d;%dH", y + 1, x + 1);
        std::fflush(stdout);
#endif
    }

    static void getxy(int& x, int& y) {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            x = csbi.dwCursorPosition.X;
            y = csbi.dwCursorPosition.Y;
        } else {
            x = y = -1;
        }
#else
        struct termios oldt, newt;
        std::fflush(stdout);
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        std::printf("\033[6n");
        std::fflush(stdout);

        int row = 0, col = 0;
        if (std::scanf("\033[%d;%dR", &row, &col) == 2) {
            x = col - 1;
            y = row - 1;
        } else {
            x = y = -1;
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    }

    static void clear() {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD count, cells;
        COORD home = { 0, 0 };

        if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
        cells = csbi.dwSize.X * csbi.dwSize.Y;

        FillConsoleOutputCharacter(hConsole, ' ', cells, home, &count);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cells, home, &count);
        SetConsoleCursorPosition(hConsole, home);
#else
        std::cout << "\033[2J\033[H";
        std::cout.flush();
#endif
    }

    static void hideCursor() {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo(hConsole, &info);
        info.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &info);
#else
        std::cout << "\033[?25l";
        std::cout.flush();
#endif
    }

    static void showCursor() {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo(hConsole, &info);
        info.bVisible = TRUE;
        SetConsoleCursorInfo(hConsole, &info);
#else
        std::cout << "\033[?25h";
        std::cout.flush();
#endif
    }

    static void resize(int width, int height) {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        //CONSOLE_SCREEN_BUFFER_INFO csbi;
        SMALL_RECT win = { 0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1) };
        COORD buf = { static_cast<SHORT>(width), static_cast<SHORT>(height) };

        SetConsoleScreenBufferSize(hConsole, buf);
        SetConsoleWindowInfo(hConsole, TRUE, &win);
#else
        // Not portable; stub
        (void)width;
        (void)height;
#endif
    }
};
} // namespace DotBlue