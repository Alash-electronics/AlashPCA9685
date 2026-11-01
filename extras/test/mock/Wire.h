// Имитация I2C и PCA9685 (регистры, автоувеличение адреса, делитель только во сне).
#pragma once
#include "Arduino.h"
struct FakePca {
  uint8_t addr = 0x40; bool present = true; bool isPca = true;
  uint8_t reg[256] = {0};
  int writes = 0;
};
inline FakePca g_pca;
class TwoWire {
 public:
  void begin() {}
  void begin(int, int) {}
  void setClock(uint32_t) {}
  void beginTransmission(uint8_t a) { _a = a; _n = 0; }
  size_t write(uint8_t v) {
    if (_n == 0) _reg = v;
    else {
      uint8_t r = _reg + _n - 1;
      if (r == 0xFE && !(g_pca.reg[0] & 0x10)) { _n++; return 1; }   // делитель принимается только во сне
      if (!g_pca.isPca && r == 0xFE) { _n++; return 1; }               // чужое устройство: регистра нет
      g_pca.reg[r] = v;
    }
    _n++; return 1;
  }
  uint8_t endTransmission(bool = true) { if (_n > 1) g_pca.writes++; return (g_pca.present && _a == g_pca.addr) ? 0 : 2; }
  uint8_t requestFrom(uint8_t a, uint8_t len) {
    if (!(g_pca.present && a == g_pca.addr)) return 0;
    for (uint8_t i = 0; i < len; i++) _buf[i] = g_pca.reg[_reg + i];
    _len = len; _pos = 0; return len;
  }
  int read() { return _pos < _len ? _buf[_pos++] : -1; }
 private:
  uint8_t _a = 0, _reg = 0, _n = 0, _buf[32], _len = 0, _pos = 0;
};
inline TwoWire Wire;
