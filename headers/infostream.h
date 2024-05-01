#pragma once
#include <Windows.h>
#include <iostream>
#include <string>

#define BLACK         0
#define DARK_BLUE     FOREGROUND_BLUE
#define DARK_GREEN    FOREGROUND_GREEN
#define DARK_CYAN     (FOREGROUND_GREEN | FOREGROUND_BLUE)
#define DARK_RED      FOREGROUND_RED
#define DARK_MAGENTA  (FOREGROUND_RED | FOREGROUND_BLUE)
#define DARK_YELLOW   (FOREGROUND_RED | FOREGROUND_GREEN)
#define GRAY          (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define DARK_GRAY     (FOREGROUND_INTENSITY)
#define BLUE          (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define GREEN         (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define CYAN          (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define RED           (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define MAGENTA       (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define YELLOW        (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define WHITE         (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)

void notify(const char* message) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, GRAY);
    std::cout << "[?] " << message << std::endl;
    SetConsoleTextAttribute(hConsole, WHITE);
}

void alert(const char* message) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, RED);
    std::cout << "[!] " << message << std::endl;
    SetConsoleTextAttribute(hConsole, WHITE);
}

void success(const char* message) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, GREEN);
    std::cout << "[-] " << message << std::endl;
    SetConsoleTextAttribute(hConsole, WHITE);
}

void warn(const char* message) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, YELLOW);
    std::cout << "[!] " << message << std::endl;
    SetConsoleTextAttribute(hConsole, WHITE);
}

const char* userInput(const char* message) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    notify(message);

    // Set the console text color to cyan
    SetConsoleTextAttribute(hConsole, CYAN);

    // Allocate a buffer for user input
    const int bufferSize = 256;  // Define the maximum input size
    char* input = new char[bufferSize];

    // Safely get input from user
    if (fgets(input, bufferSize, stdin)) {
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0'; // Remove trailing newline
        }
    } else {
        // Handle input error or EOF
        input[0] = '\0'; // Ensure the string is empty if input fails
    }

    // Reset the console text color to white
    SetConsoleTextAttribute(hConsole, WHITE);

    // Return the dynamically allocated string (caller must delete it later)
    return input;
}