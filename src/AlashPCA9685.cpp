// MIT License. Alash Electronics.
#include "AlashPCA9685.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

static const uint8_t REG_MODE1 = 0x00;
static const uint8_t REG_MODE2 = 0x01;
static const uint8_t REG_LED0_ON_L = 0x06;
static const uint8_t REG_ALL_LED_ON_L = 0xFA;
static const uint8_t REG_PRESCALE = 0xFE;
static const uint8_t MODE1_RESTART = 0x80;
static const uint8_t MODE1_AI = 0x20;          // автоувеличение адреса регистра: можно писать сразу 4 байта канала
static const uint8_t MODE1_SLEEP = 0x10;
static const uint8_t MODE2_OUTDRV = 0x04;      // выходы "push-pull" (так нужно для сервоприводов и светодиодов)

// ---- Запуск --------------------------------------------------------------------

bool AlashPCA9685::begin(TwoWire &wire, bool startWire) {
  _wire = &wire;
  if (startWire) _wire->begin();
  uint8_t wanted = _addr;
  if (probe(wanted)) return true;
  Error first = _err;
  if (_err == ERR_NO_DEVICE) {                 // по умолчанию 0x40 пусто: смотрим соседние адреса (перемычки A0-A2)
    for (uint8_t a = 0x41; a <= 0x47; a++) {
      if (probe(a)) return true;
      if (_err == ERR_WRONG_CHIP) first = ERR_WRONG_CHIP;
    }
  }
  _addr = wanted;
  _err = first;
  return false;
}

#if defined(ESP32) || defined(ESP8266)
bool AlashPCA9685::begin(int sdaPin, int sclPin) {
  Wire.begin(sdaPin, sclPin);
  return begin(Wire, false);
}
#endif

bool AlashPCA9685::probe(uint8_t address) {
  _addr = address;
  _wire->beginTransmission(address);
  if (_wire->endTransmission() != 0) {
    _err = ERR_NO_DEVICE;
    return false;
  }
  // Проверка, что это PCA9685: в режиме сна в регистр делителя можно записать число и прочитать его назад.
  uint8_t back = 0;
  if (!write8(REG_MODE1, MODE1_SLEEP | MODE1_AI) || !write8(REG_PRESCALE, 0x79) || !read8(REG_PRESCALE, back)) {
    _err = ERR_I2C;
    return false;
  }
  if (back != 0x79) {
    _err = ERR_WRONG_CHIP;
    return false;
  }
  write8(REG_MODE2, MODE2_OUTDRV);
  for (uint8_t i = 0; i < 16; i++) _s[i].ticks = 0xFFFF;
  if (!setPrescale()) return false;
  _err = ERR_OK;
  return true;
}

bool AlashPCA9685::isConnected() {
  uint8_t v;
  return read8(REG_MODE1, v);
}

// Частота ШИМ = кварц / (4096 * (делитель + 1))
bool AlashPCA9685::setPrescale() {
  float p = (float)_osc / (4096.0f * _freq) - 1.0f;
  long pre = lroundf(p);
  if (pre < 3) pre = 3;
  if (pre > 255) pre = 255;
  _actual = (float)_osc / (4096.0f * (float)(pre + 1));
  uint8_t old = 0;
  read8(REG_MODE1, old);
  if (!write8(REG_MODE1, (old & ~MODE1_RESTART) | MODE1_SLEEP)) return false;   // делитель меняется только во сне
  if (!write8(REG_PRESCALE, (uint8_t)pre)) return false;
  if (!write8(REG_MODE1, (old & ~(MODE1_RESTART | MODE1_SLEEP)) | MODE1_AI)) return false;
  delay(1);                                                                      // генератору нужно 500 мкс на пробуждение
  return write8(REG_MODE1, MODE1_AI | MODE1_RESTART);
}

void AlashPCA9685::setFrequency(float hz) {
  if (hz < 24.0f) hz = 24.0f;
  if (hz > 1526.0f) hz = 1526.0f;
  _freq = hz;
  for (uint8_t i = 0; i < 16; i++) _s[i].ticks = 0xFFFF;
  setPrescale();
}

void AlashPCA9685::sleep() {
  uint8_t m = 0;
  if (read8(REG_MODE1, m)) write8(REG_MODE1, (m & ~MODE1_RESTART) | MODE1_SLEEP);
}

void AlashPCA9685::wake() {
  uint8_t m = 0;
  if (read8(REG_MODE1, m)) {
    write8(REG_MODE1, (m & ~MODE1_SLEEP) | MODE1_AI);
    delay(1);
    write8(REG_MODE1, MODE1_AI | MODE1_RESTART);
  }
}

// ---- Общий ШИМ ---------------------------------------------------------------------

bool AlashPCA9685::check(uint8_t channel) {
  if (channel > 15) {
    _err = ERR_BAD_CHANNEL;
    return false;
  }
  return true;
}

void AlashPCA9685::setPWM(uint8_t channel, uint16_t on, uint16_t off) {
  if (!check(channel)) return;
  _wire->beginTransmission(_addr);
  _wire->write((uint8_t)(REG_LED0_ON_L + 4 * channel));
  _wire->write((uint8_t)(on & 0xFF));
  _wire->write((uint8_t)(on >> 8));
  _wire->write((uint8_t)(off & 0xFF));
  _wire->write((uint8_t)(off >> 8));
  _err = _wire->endTransmission() == 0 ? ERR_OK : ERR_I2C;
}

void AlashPCA9685::setLevel(uint8_t channel, bool high) {
  if (high) setPWM(channel, 0x1000, 0);       // бит 12 в ON: всегда включён
  else setPWM(channel, 0, 0x1000);            // бит 12 в OFF: всегда выключен
  if (channel < 16) _s[channel].ticks = 0xFFFF;
}

void AlashPCA9685::setDuty(uint8_t channel, float percent) {
  if (!check(channel)) return;
  if (percent <= 0.0f) { setLevel(channel, false); return; }
  if (percent >= 100.0f) { setLevel(channel, true); return; }
  setPWM(channel, 0, (uint16_t)lroundf(percent * 40.96f));
  _s[channel].ticks = 0xFFFF;
}

uint16_t AlashPCA9685::usToTicks(float us) const {
  float t = us * 4096.0f * _actual / 1000000.0f;
  if (t < 0) t = 0;
  if (t > 4095) t = 4095;
  return (uint16_t)lroundf(t);
}

void AlashPCA9685::setPulseUs(uint8_t channel, uint16_t us) {
  if (!check(channel)) return;
  setPWM(channel, 0, usToTicks(us));
  _s[channel].ticks = 0xFFFF;
}

void AlashPCA9685::allOff() {
  _wire->beginTransmission(_addr);
  _wire->write(REG_ALL_LED_ON_L);
  _wire->write((uint8_t)0); _wire->write((uint8_t)0);
  _wire->write((uint8_t)0); _wire->write((uint8_t)0x10);   // ALL_LED_OFF_H бит 4: все каналы выключены
  _err = _wire->endTransmission() == 0 ? ERR_OK : ERR_I2C;
  for (uint8_t i = 0; i < 16; i++) { _s[i].active = false; _s[i].ticks = 0xFFFF; }
  _moving = 0;
}

// ---- Сервоприводы -------------------------------------------------------------------

// Угол -> длительность импульса. Центр (90°) можно сдвинуть: тогда половины шкалы растягиваются по-разному, как в калибровке робота.
float AlashPCA9685::angleToUs(const Servo &s, float deg) const {
  if (deg < 0) deg = 0;
  if (deg > 180) deg = 180;
  if (s.inverted) deg = 180.0f - deg;
  float mid = (s.minUs + s.maxUs) / 2.0f + s.centerOff;
  if (deg <= 90.0f) return s.minUs + (mid - s.minUs) * deg / 90.0f;
  return mid + (s.maxUs - mid) * (deg - 90.0f) / 90.0f;
}

void AlashPCA9685::servo(uint8_t channel, float degrees) {
  if (!check(channel)) return;
  Servo &s = _s[channel];
  if (degrees < s.minDeg) degrees = s.minDeg;
  if (degrees > s.maxDeg) degrees = s.maxDeg;
  s.angle = degrees;
  s.touched = true;
  if (s.active) { s.active = false; if (_moving) _moving--; }
  uint16_t t = usToTicks(angleToUs(s, degrees));
  if (t == s.ticks) return;                    // тот же импульс: по I2C ничего не шлём
  s.ticks = t;
  setPWM(channel, 0, t);
}

float AlashPCA9685::servoAngle(uint8_t channel) const { return channel < 16 ? _s[channel].angle : NAN; }

void AlashPCA9685::servoOff(uint8_t channel) {
  if (!check(channel)) return;
  if (_s[channel].active) { _s[channel].active = false; if (_moving) _moving--; }
  setLevel(channel, false);
}

void AlashPCA9685::setServoRange(uint8_t channel, uint16_t minUs, uint16_t maxUs, int16_t centerOffsetUs) {
  if (!check(channel) || minUs >= maxUs) return;
  _s[channel].minUs = minUs;
  _s[channel].maxUs = maxUs;
  _s[channel].centerOff = centerOffsetUs;
  _s[channel].touched = true;
  _s[channel].ticks = 0xFFFF;
}

void AlashPCA9685::setAllServoRange(uint16_t minUs, uint16_t maxUs) {
  for (uint8_t i = 0; i < 16; i++) setServoRange(i, minUs, maxUs, _s[i].centerOff);
}

void AlashPCA9685::setServoLimits(uint8_t channel, float minDeg, float maxDeg) {
  if (!check(channel) || minDeg >= maxDeg) return;
  _s[channel].minDeg = minDeg < 0 ? 0 : minDeg;
  _s[channel].maxDeg = maxDeg > 180 ? 180 : maxDeg;
  _s[channel].touched = true;
}

void AlashPCA9685::invertServo(uint8_t channel, bool on) {
  if (!check(channel)) return;
  _s[channel].inverted = on;
  _s[channel].touched = true;
  _s[channel].ticks = 0xFFFF;
}

void AlashPCA9685::servoMove(uint8_t channel, float degrees, uint16_t durationMs) {
  if (!check(channel)) return;
  Servo &s = _s[channel];
  if (degrees < s.minDeg) degrees = s.minDeg;
  if (degrees > s.maxDeg) degrees = s.maxDeg;
  if (durationMs == 0 || !s.touched) {         // положение неизвестно: плавно ехать неоткуда, ставим сразу
    servo(channel, degrees);
    return;
  }
  if (!s.active) _moving++;
  s.active = true;
  s.from = s.angle;
  s.to = degrees;
  s.startMs = millis();
  s.durMs = durationMs;
}

void AlashPCA9685::pose(const uint8_t *channels, const float *angles, uint8_t count, uint16_t durationMs) {
  for (uint8_t i = 0; i < count; i++) servoMove(channels[i], angles[i], durationMs);
}

void AlashPCA9685::poseAll(const float *angles16, uint16_t durationMs) {
  for (uint8_t i = 0; i < 16; i++) servoMove(i, angles16[i], durationMs);
}

// Плавное начало и конец: угол меняется по косинусу, а не рывком
bool AlashPCA9685::run() {
  if (_moving == 0) return false;
  uint32_t now = millis();
  for (uint8_t i = 0; i < 16; i++) {
    Servo &s = _s[i];
    if (!s.active) continue;
    uint32_t el = now - s.startMs;
    if (el >= s.durMs) {
      servo(i, s.to);                          // servo() снимает active и уменьшает счётчик
    } else {
      float t = (float)el / s.durMs;
      float e = 0.5f - 0.5f * cosf(3.14159265f * t);
      bool keep = s.active;
      float ang = s.from + (s.to - s.from) * e;
      uint16_t tk = usToTicks(angleToUs(s, ang));
      s.angle = ang;
      if (tk != s.ticks) { s.ticks = tk; setPWM(i, 0, tk); }
      s.active = keep;
    }
  }
  return _moving != 0;
}

void AlashPCA9685::wait() {
  while (run()) delay(2);
}

void AlashPCA9685::test(uint8_t channel) {
  servo(channel, 90);
  delay(500);
  servoMove(channel, 60, 700); wait();
  servoMove(channel, 120, 1200); wait();
  servoMove(channel, 90, 700); wait();
}

// ---- I2C -----------------------------------------------------------------------------------

bool AlashPCA9685::write8(uint8_t reg, uint8_t v) {
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  _wire->write(v);
  return _wire->endTransmission() == 0;
}

bool AlashPCA9685::read8(uint8_t reg, uint8_t &v) {
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  if (_wire->endTransmission(false) != 0) return false;
  if (_wire->requestFrom(_addr, (uint8_t)1) != 1) return false;
  v = (uint8_t)_wire->read();
  return true;
}

// ---- Диагностика --------------------------------------------------------------------------------

const __FlashStringHelper *AlashPCA9685::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NO_DEVICE:
      return T("Модуль PCA9685 не найден: проверьте SDA / SCL, питание VCC 3.3-5 В (логика) и GND",
               "PCA9685 модулі табылмады: SDA / SCL, VCC 3.3-5 В (логика) қуатын және GND тексеріңіз",
               "PCA9685 not found: check SDA / SCL, VCC 3.3-5 V (logic) power and GND");
    case ERR_WRONG_CHIP:
      return T("По этому адресу отвечает другое устройство (например, INA219 тоже 0x40). Адрес PCA9685 меняется перемычками A0-A5",
               "Бұл мекенжайда басқа құрылғы жауап береді (мысалы, INA219 да 0x40). PCA9685 мекенжайы A0-A5 перемычкаларымен өзгереді",
               "Another device answers at this address (for example INA219 is also 0x40). The PCA9685 address is changed with the A0-A5 jumpers");
    case ERR_I2C:
      return T("Ошибка I2C: проверьте контакты и подтяжку линий", "I2C қатесі: байланысты және тартқыш резисторларды тексеріңіз",
               "I2C error: check connections and pull-up resistors");
    case ERR_BAD_CHANNEL:
      return T("Канал должен быть от 0 до 15", "Арна 0-ден 15-ке дейін болуы керек", "The channel must be from 0 to 15");
  }
  return F("");
}

void AlashPCA9685::diagnose(Print &out) {
  out.println(T("AlashPCA9685: диагностика", "AlashPCA9685: диагностика", "AlashPCA9685: diagnostics"));
  out.println(T("Найденные устройства I2C:", "Табылған I2C құрылғылары:", "I2C devices found:"));
  for (uint8_t a = 1; a < 127; a++) {
    _wire->beginTransmission(a);
    if (_wire->endTransmission() == 0) {
      out.print(F("  0x"));
      if (a < 16) out.print('0');
      out.print(a, HEX);
      if (a >= 0x40 && a <= 0x47) out.print(F("  PCA9685 / INA219 / HTU21D"));
      else if (a == 0x70) out.print(F("  PCA9685 (общий адрес / all-call)"));
      else if (a == 0x3C || a == 0x3D) out.print(F("  OLED SSD1306"));
      else if (a == 0x68 || a == 0x69) out.print(F("  MPU6050 / DS3231"));
      else if (a == 0x76 || a == 0x77) out.print(F("  BME280 / BMP280"));
      out.println();
    }
  }
  if (_err != ERR_OK) {
    out.println(errorText());
    return;
  }
  out.print(T("PCA9685 найден, адрес 0x", "PCA9685 табылды, мекенжайы 0x", "PCA9685 found, address 0x"));
  out.println(_addr, HEX);
  out.print(T("Частота ШИМ: ", "ШИМ жиілігі: ", "PWM frequency: "));
  out.print(_actual, 1);
  out.println(F(" Hz"));
  out.println(T("Сервоприводы питаются отдельно: 5-6 В на клемму V+ модуля (не от пина 5V платы Arduino), GND общий. Если сервоприводы дёргаются или плата перезагружается, не хватает мощности блока питания: 1 сервопривод тянет до 0.5-1 А. Для проверки канала вызовите test(номер).",
                "Сервоприводтар бөлек қуатталады: модульдің V+ қысқышына 5-6 В (Arduino 5V пинінен емес), GND ортақ. Сервоприводтар тартылса немесе тақта қайта қосылса, қуат көзі әлсіз: 1 сервопривод 0.5-1 А дейін тартады. Арнаны тексеру үшін test(нөмір) шақырыңыз.",
                "Servos need their own power: 5-6 V on the module V+ terminal (not from the Arduino 5V pin), common GND. If servos jitter or the board resets, the supply is too weak: one servo can draw 0.5-1 A. Call test(channel) to check a channel."));
}

void AlashPCA9685::printCalibration(Print &out) {
  for (uint8_t i = 0; i < 16; i++) {
    const Servo &s = _s[i];
    if (!s.touched) continue;
    out.print(F("pca.setServoRange("));
    out.print(i); out.print(F(", ")); out.print(s.minUs); out.print(F(", ")); out.print(s.maxUs); out.print(F(", ")); out.print(s.centerOff);
    out.println(F(");"));
    if (s.minDeg != 0 || s.maxDeg != 180) {
      out.print(F("pca.setServoLimits("));
      out.print(i); out.print(F(", ")); out.print(s.minDeg, 0); out.print(F(", ")); out.print(s.maxDeg, 0);
      out.println(F(");"));
    }
    if (s.inverted) {
      out.print(F("pca.invertServo("));
      out.print(i);
      out.println(F(");"));
    }
  }
}
