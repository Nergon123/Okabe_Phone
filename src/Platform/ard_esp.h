#pragma once
#ifdef PC
#include <iostream>
#include <cstdint>
using byte = uint8_t;
unsigned long millis();
void delay(unsigned long ms);
class SerialClass {
public:
    void begin(unsigned long baud) {
        // PC implementation doesn't need actual baud rate
        (void)baud;
    }
    
    void updateBaudRate(unsigned long baud) {
        // PC implementation doesn't need actual baud rate
        (void)baud;
    }

    void println(const char* str) {
        std::cout << str << std::endl;
    }

    void println(const std::string& str) {
        std::cout << str << std::endl;
    }

    void println(int num) {
        std::cout << num << std::endl;
    }

    void println(float num) {
        std::cout << num << std::endl;
    }

    void print(const char* str) {
        std::cout << str;
    }

    void print(const std::string& str) {
        std::cout << str;
    }

    void print(int num) {
        std::cout << num;
    }

    void print(float num) {
        std::cout << num;
    }

    bool available() {
        return std::cin.rdbuf()->in_avail() > 0;
    }

    int read() {
        char c;
        if (std::cin.get(c))
            return c;
        return -1;
    }
};

extern SerialClass Serial;
extern SerialClass Serial1;

#endif
