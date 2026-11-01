// Сокращённое число каналов состояния (экономия памяти Uno): сервоприводы только на них, обычный ШИМ на всех 16.
//   g++ -std=c++17 -Wall -DALASH_PCA9685_CHANNELS=4 -I mock -I ../../src test_pca9685_small.cpp ../../src/AlashPCA9685.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "Wire.h"
#include "AlashPCA9685.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
static uint16_t off(int ch) { return g_pca.reg[8 + 4 * ch] | (g_pca.reg[9 + 4 * ch] << 8); }

int main() {
  g_pca = FakePca();
  AlashPCA9685 p; CHECK(p.begin());
  CHECK(ALASH_PCA9685_CHANNELS == 4);
  p.servo(0, 90); CHECK(off(0) == 307);
  p.servo(3, 0); CHECK(off(3) == 123);
  p.servo(4, 90); CHECK(p.error() == AlashPCA9685::ERR_BAD_CHANNEL);          // нет места для состояния
  p.setServoRange(9, 500, 2500); CHECK(p.error() == AlashPCA9685::ERR_BAD_CHANNEL);
  p.setDuty(15, 50); CHECK(p.error() == AlashPCA9685::ERR_OK && off(15) == 2048);   // обычный ШИМ работает на всех
  p.servoOff(15); CHECK(off(15) == 0x1000);
  float a[16]; for (int i = 0; i < 16; i++) a[i] = 90;
  p.poseAll(a, 0);                                                              // только первые 4 каналов
  CHECK(p.servoAngle(2) == 90.0f && isnan(p.servoAngle(10)));
  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
