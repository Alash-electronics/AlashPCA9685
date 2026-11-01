// Минимальная имитация Arduino.h для запуска тестов на компьютере (g++).
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
using __FlashStringHelper = char;
#define F(x) ((const __FlashStringHelper *)(x))
#define HEX 16
inline uint32_t g_ms = 100000;
inline uint32_t millis() { return g_ms; }
inline void delay(uint32_t ms) { g_ms += ms; }
class Print {
 public:
  void print(const char *s) { printf("%s", s); }
  void print(char c) { printf("%c", c); }
  void print(unsigned v, int base = 10) { printf(base == HEX ? "%X" : "%u", v); }
  void print(int v, int base = 10) { printf(base == HEX ? "%X" : "%d", v); }
  void print(float v, int d = 2) { printf("%.*f", d, v); }
  void println(const char *s = "") { printf("%s\n", s); }
  void println(unsigned v, int base = 10) { print(v, base); printf("\n"); }
  void println(int v, int base = 10) { print(v, base); printf("\n"); }
};
