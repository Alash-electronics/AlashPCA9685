// AlashPCA9685: 16 каналов ШИМ для сервоприводов и светодиодов (модуль PCA9685): углы вместо импульсов,
// калибровка каждого сервопривода, плавные движения, позы робота. Без зависимостей, только встроенная Wire.
// MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashPCA9685
#ifndef ALASH_PCA9685_H
#define ALASH_PCA9685_H

#include <Arduino.h>
#include <Wire.h>

// Сколько каналов хранит состояние (можно уменьшить до #include, чтобы сэкономить память Uno: #define ALASH_PCA9685_CHANNELS 8)
#ifndef ALASH_PCA9685_CHANNELS
#define ALASH_PCA9685_CHANNELS 16
#endif

class AlashPCA9685 {
 public:
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NO_DEVICE,     // на шине I2C нет ответа по адресам 0x40-0x47 (и выбранному)
    ERR_WRONG_CHIP,    // по адресу отвечает другое устройство (например, INA219 тоже 0x40)
    ERR_I2C,           // ошибка чтения или записи по I2C
    ERR_BAD_CHANNEL    // канал должен быть от 0 до 15
  };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  explicit AlashPCA9685(uint8_t address = 0x40) : _addr(address) {}

  // ---- Запуск --------------------------------------------------------------
  // begin() запускает модуль и ставит 50 Гц (частота сервоприводов). Адрес по умолчанию 0x40; если там ничего нет, ищет 0x41-0x47.
  bool begin(TwoWire &wire = Wire, bool startWire = true);
#if defined(ESP32) || defined(ESP8266)
  bool begin(int sdaPin, int sclPin);        // свои пины I2C
#endif
  void setClock(uint32_t hz) { _wire->setClock(hz); }   // скорость I2C: 400000 быстрее для многих сервоприводов

  // ---- Сервоприводы (углы 0..180) ------------------------------------------------
  void servo(uint8_t channel, float degrees);                 // сразу установить угол
  float servoAngle(uint8_t channel) const;                    // последний заданный угол
  void servoMove(uint8_t channel, float degrees, uint16_t durationMs);   // плавно за durationMs (нужен run() в loop)
  void servoOff(uint8_t channel);                             // снять управление: серво отдыхает и не гудит
  void center(uint8_t channel) { servo(channel, 90); }

  // Калибровка конкретного сервопривода: импульсы для 0° и 180° в микросекундах, при необходимости смещение центра
  void setServoRange(uint8_t channel, uint16_t minUs, uint16_t maxUs, int16_t centerOffsetUs = 0);
  void setAllServoRange(uint16_t minUs, uint16_t maxUs);      // для всех каналов сразу (по умолчанию 600..2400)
  void setServoLimits(uint8_t channel, float minDeg, float maxDeg);   // не пускать сервопривод за эти углы (защита механики)
  void invertServo(uint8_t channel, bool on = true);          // обратное направление без перепайки

  // Позы: несколько каналов сразу. channels[i] получает angles[i]. durationMs = 0: мгновенно.
  void pose(const uint8_t *channels, const float *angles, uint8_t count, uint16_t durationMs = 0);
  void poseAll(const float *angles16, uint16_t durationMs = 0);   // массив из 16 углов для каналов 0..15

  // ---- Движение без блокировки -----------------------------------------------------
  bool run();                                // обновить плавные движения. true, пока что-то движется
  bool isMoving() { run(); return _moving != 0; }
  void wait();                               // дождаться конца движений (блокирует)

  // ---- Общий ШИМ (светодиоды, моторы, любой выход) ----------------------------------
  void setDuty(uint8_t channel, float percent);               // скважность 0..100 %
  void setPulseUs(uint8_t channel, uint16_t us);              // длительность импульса в микросекундах
  void setPWM(uint8_t channel, uint16_t on, uint16_t off);    // «сырые» значения 0..4095
  void setLevel(uint8_t channel, bool high);                  // постоянно HIGH или LOW
  void allOff();                             // выключить все каналы (серво отпускает)
  void setFrequency(float hz);               // 24..1526 Гц (сервоприводы 50 Гц)
  float frequency() const { return _freq; }
  void setOscillatorHz(uint32_t hz) { _osc = hz; setFrequency(_freq); }   // точная частота кварца, если импульсы чуть "плывут" (25000000 по умолчанию)
  void sleep();                              // уснуть (меньше потребление)
  void wake();

  // ---- Диагностика -----------------------------------------------------------------
  void setLanguage(Language lang) { _lang = lang; }
  Error error() const { return _err; }
  const __FlashStringHelper *errorText();
  uint8_t address() const { return _addr; }
  bool isConnected();
  void test(uint8_t channel);                // плавно качнуть серво на этом канале (блокирует ~3 с)
  void diagnose(Print &out);
  void printCalibration(Print &out);         // выводит строки setServoRange(...), которые можно вставить в скетч

 private:
  TwoWire *_wire = &Wire;
  uint8_t _addr;
  Error _err = ERR_OK;
  Language _lang = LANG_RU;
  float _freq = 50.0f;                       // запрошенная частота
  float _actual = 50.0f;                     // частота, которую реально дал делитель
  uint32_t _osc = 25000000UL;

  // Состояние одного канала. Компактно (23 байта), потому что на Uno всего 2 КБ памяти: углы хранятся в десятых долях градуса.
  struct Servo {
    uint16_t minUs = 600, maxUs = 2400;
    int16_t centerOff = 0;
    uint8_t minDeg = 0, maxDeg = 180;
    int16_t angle = 900;                     // текущий угол, 0,1°
    int16_t from = 0, to = 0;                // начало и цель плавного движения, 0,1°
    uint32_t startMs = 0;
    uint16_t durMs = 0;
    uint16_t ticks = 0xFFFF;                 // последнее записанное значение (чтобы не слать лишнее по I2C)
    bool inverted = false, active = false, touched = false;
  };
  Servo _s[ALASH_PCA9685_CHANNELS];
  uint8_t _moving = 0;

  bool probe(uint8_t address);
  bool write8(uint8_t reg, uint8_t v);
  bool read8(uint8_t reg, uint8_t &v);
  bool setPrescale();
  uint16_t usToTicks(float us) const;
  float angleToUs(const Servo &s, float deg) const;
  bool check(uint8_t channel);                // канал 0..15 (любой выход)
  bool checkServo(uint8_t channel);           // канал, для которого хранится состояние сервопривода
};

#endif  // ALASH_PCA9685_H
