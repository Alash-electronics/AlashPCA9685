// Тесты AlashPCA9685 без железа (имитация регистров):
//   g++ -std=c++17 -Wall -I mock -I ../../src test_pca9685.cpp ../../src/AlashPCA9685.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "Wire.h"
#include "AlashPCA9685.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
static void reset() { g_pca = FakePca(); g_ms = 100000; }
static uint16_t on(int ch) { return g_pca.reg[6 + 4 * ch] | (g_pca.reg[7 + 4 * ch] << 8); }
static uint16_t off(int ch) { return g_pca.reg[8 + 4 * ch] | (g_pca.reg[9 + 4 * ch] << 8); }

int main() {
  // 1) запуск: 50 Гц = делитель 121 (0x79), автоувеличение адреса включено, сон выключен
  reset(); AlashPCA9685 p;
  CHECK(p.begin());
  CHECK(g_pca.reg[0xFE] == 121);
  CHECK((g_pca.reg[0] & 0x20) && !(g_pca.reg[0] & 0x10));
  CHECK(p.frequency() == 50.0f);

  // 2) углы: 90° = 1500 мкс = 307 тиков, 0° = 600 мкс = 123, 180° = 2400 мкс = 491
  p.servo(0, 90); CHECK(on(0) == 0 && off(0) == 307);
  p.servo(0, 0);  CHECK(off(0) == 123);
  p.servo(0, 180); CHECK(off(0) == 492);
  p.servo(5, 45); CHECK(off(5) > 123 && off(5) < 307);
  CHECK(p.servoAngle(5) == 45.0f);
  p.servo(1, 400); CHECK(off(1) == 492);                    // за пределами 0..180 ограничивается
  p.servo(1, -20); CHECK(off(1) == 123);

  // 3) калибровка: свой диапазон и смещение центра
  p.setServoRange(2, 500, 2500, 0);
  p.servo(2, 0);  CHECK(off(2) == 102);                     // 500 мкс
  p.servo(2, 180); CHECK(off(2) == 512);                    // 2500 мкс
  p.setServoRange(3, 600, 2400, 100);                        // центр сдвинут на 100 мкс: 1600 мкс
  p.servo(3, 90); CHECK(off(3) == 328);
  p.servo(3, 0);  CHECK(off(3) == 123);                      // края не двигаются
  p.servo(3, 180); CHECK(off(3) == 492);
  p.setAllServoRange(1000, 2000);
  p.servo(4, 0); CHECK(off(4) == 205);

  // 4) инверсия и границы углов
  p.setAllServoRange(600, 2400);
  p.invertServo(6, true);
  p.servo(6, 0); CHECK(off(6) == 492);
  p.servo(6, 180); CHECK(off(6) == 123);
  p.setServoLimits(7, 30, 150);
  p.servo(7, 0); CHECK(p.servoAngle(7) == 30.0f);
  p.servo(7, 180); CHECK(p.servoAngle(7) == 150.0f);

  // 5) повтор того же угла не шлёт лишнего по I2C
  p.servo(8, 90);
  int w = g_pca.writes;
  p.servo(8, 90); p.servo(8, 90.01f);
  CHECK(g_pca.writes == w);

  // 6) плавное движение: косинусная кривая, в конце ровно целевой угол, run() нужен
  p.servo(9, 0);
  p.servoMove(9, 180, 1000);
  CHECK(p.isMoving());
  g_ms += 100; p.run();
  float a1 = p.servoAngle(9);
  g_ms += 400; p.run();
  float a2 = p.servoAngle(9);
  CHECK(a1 > 0 && a1 < 20);                                  // начало медленное
  CHECK(a2 > 80 && a2 < 100);                                 // середина около 90
  g_ms += 600; p.run();
  CHECK(p.servoAngle(9) == 180.0f && !p.isMoving() && off(9) == 492);

  // 7) поза: несколько каналов приходят в цель одновременно
  const uint8_t ch[] = {10, 11, 12};
  const float ang[] = {30, 90, 150};
  p.servo(10, 90); p.servo(11, 0); p.servo(12, 180);
  p.pose(ch, ang, 3, 500);
  g_ms += 250; p.run();
  CHECK(p.isMoving());
  g_ms += 300; p.run();
  CHECK(!p.isMoving() && p.servoAngle(10) == 30.0f && p.servoAngle(11) == 90.0f && p.servoAngle(12) == 150.0f);
  float all[16]; for (int i = 0; i < 16; i++) all[i] = 90;
  p.poseAll(all, 0);
  CHECK(off(15) == 307 && off(0) == 307);

  // 8) движение с неизвестного положения ставится сразу
  reset(); AlashPCA9685 q; q.begin();
  q.servoMove(3, 120, 1000);
  CHECK(!q.isMoving() && q.servoAngle(3) == 120.0f);

  // 9) обычный ШИМ: проценты, постоянный уровень, микросекунды
  q.setDuty(0, 50); CHECK(on(0) == 0 && off(0) == 2048);
  q.setDuty(1, 100); CHECK(on(1) == 0x1000 && off(1) == 0);
  q.setDuty(2, 0); CHECK(on(2) == 0 && off(2) == 0x1000);
  q.setLevel(3, true); CHECK(on(3) == 0x1000);
  q.setPulseUs(4, 1500); CHECK(off(4) == 307);
  q.servoOff(4); CHECK(off(4) == 0x1000);
  q.allOff(); CHECK(g_pca.reg[0xFD] == 0x10);

  // 10) частота: 1000 Гц, пределы; импульсы пересчитываются под новую частоту
  q.setFrequency(1000); CHECK(g_pca.reg[0xFE] == 5);         // 25 МГц / 4096 / 1000 - 1 = 5.1
  q.setFrequency(10); CHECK(q.frequency() == 24.0f);
  q.setFrequency(50);
  q.setOscillatorHz(27000000UL);
  q.servo(0, 90); CHECK(off(0) >= 306 && off(0) <= 308);      // 50 Гц сохраняется с другим кварцем: делитель 131
  CHECK(g_pca.reg[0xFE] == 131);

  // 11) ошибки: канал, нет модуля, чужое устройство, другой адрес
  q.servo(16, 90); CHECK(q.error() == AlashPCA9685::ERR_BAD_CHANNEL);
  reset(); g_pca.present = false; AlashPCA9685 n; CHECK(!n.begin() && n.error() == AlashPCA9685::ERR_NO_DEVICE);
  reset(); g_pca.isPca = false; AlashPCA9685 w2; CHECK(!w2.begin() && w2.error() == AlashPCA9685::ERR_WRONG_CHIP);
  reset(); g_pca.addr = 0x43; AlashPCA9685 a; CHECK(a.begin() && a.address() == 0x43);   // нашёл на соседнем адресе
  reset(); g_pca.addr = 0x44; AlashPCA9685 b(0x44); CHECK(b.begin() && b.address() == 0x44);

  // 11б) сокращённое число каналов состояния (экономия памяти Uno): сервоприводы только на них, обычный ШИМ на всех 16
#if ALASH_PCA9685_CHANNELS < 16
  reset(); AlashPCA9685 sm; sm.begin();
  sm.servo(ALASH_PCA9685_CHANNELS, 90); CHECK(sm.error() == AlashPCA9685::ERR_BAD_CHANNEL);
  sm.setDuty(15, 50); CHECK(sm.error() == AlashPCA9685::ERR_OK && off(15) == 2048);
  sm.servo(0, 90); CHECK(off(0) == 307);
#endif

  // 12) печать калибровки и диагностика
  reset(); AlashPCA9685 d; d.begin();
  d.setServoRange(1, 550, 2450, -20); d.invertServo(1); d.setServoLimits(1, 10, 170);
  printf("--- calibration ---\n"); d.printCalibration(*(new Print));
  d.setLanguage(AlashPCA9685::LANG_KK); d.diagnose(*(new Print));
  d.test(0); CHECK(d.servoAngle(0) == 90.0f);

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
