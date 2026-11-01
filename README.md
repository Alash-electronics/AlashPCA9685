# AlashPCA9685

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для модуля **PCA9685** (16 каналов ШИМ по I2C): сервоприводы по **углам**, калибровка каждого сервопривода, плавные движения и позы целиком для роботов (например, четвероногий AlashSpot). Работает и со светодиодами. Только встроенная `Wire`, **Adafruit PWM Servo Driver не нужен**.

```cpp
#include <AlashPCA9685.h>
AlashPCA9685 pca;

void setup() { pca.begin(); }
void loop() {
  pca.servo(0, 0);          // канал 0: 0 градусов / channel 0: 0 degrees
  delay(1000);
  pca.servo(0, 180);
  delay(1000);
}
```

### Что умеет
- **Углы вместо импульсов:** `servo(канал, 90)`. Частота 50 Гц, делитель и тики считаются сами.
- **Адрес находится сам** (0x40-0x47) и проверяется, что это именно PCA9685, а не INA219 с тем же адресом.
- **Калибровка каждого сервопривода:** `setServoRange(канал, минМкс, максМкс, смещениеЦентра)`, границы `setServoLimits`, обратное направление `invertServo`. `printCalibration(Serial)` выводит строки, которые можно вставить в скетч.
- **Плавные движения без остановки программы:** `servoMove(канал, угол, мс)` и `run()` в `loop()` (кривая с мягким стартом и торможением).
- **Позы:** `pose(каналы, углы, число, мс)` и `poseAll(16 углов, мс)`: все сервоприводы приходят в цель одновременно.
- **Не шлёт лишнего:** тот же угол второй раз по I2C не передаётся.
- **Светодиоды и моторы:** `setDuty(канал, процент)`, `setLevel`, `setPulseUs`, частота `setFrequency(24..1526)`.
- **Понятные подсказки** на русском, казахском и английском: `errorText()`, `diagnose(Serial)` (в том числе про питание сервоприводов), `test(канал)` покачает сервопривод.
- **ESP32 / ESP8266:** `pca.begin(SDA, SCL)`.

### Подключение
| Модуль PCA9685 | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| V+ (клемма питания сервоприводов) | 5-6 В отдельный источник | 5-6 В отдельный источник |

**Сервоприводы питайте отдельно:** 5-6 В на клемму V+, минус общий с Arduino. От пина 5V платы сервоприводы не потянет: плата будет перезагружаться, сервоприводы дёргаться.

### Чем отличается от других библиотек
Мы посмотрели Adafruit PWM Servo Driver, PCA9685 (NachtRaveVL) и ServoEasing:
- Adafruit требует считать «тики» самому (`SERVOMIN 150`, `SERVOMAX 600`) и подбирать их для каждого сервопривода; у нас микросекунды и градусы, калибровка по каналам.
- Плавные движения и позы в Adafruit нет; есть ServoEasing, но он для встроенного Servo.
- Нет подсказок, если не хватает питания или адрес занят; у нас `diagnose()`.

### Примеры
`Basic`, `SmoothMove`, `Calibrate`, `Pose`, `LedFade`, `Diagnose`, `ESP32_CustomPins`

### Тесты
Делитель частоты, тики для углов, калибровка, смещение центра, инверсия, границы, плавная кривая, позы, ШИМ, определение чужого устройства проверены без железа: `extras/test`.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**PCA9685** модуліне (I2C арқылы 16 ШИМ арнасы) арналған қарапайым кітапхана: сервоприводтар **бұрыш** бойынша, әр сервоприводты калибрлеу, жайлап қозғалу және роботтарға арналған толық позалар. Жарықдиодтармен де жұмыс істейді. Тек кірістірілген `Wire`, **Adafruit PWM Servo Driver қажет емес**.

```cpp
#include <AlashPCA9685.h>
AlashPCA9685 pca;

void setup() { pca.begin(); }
void loop() {
  pca.servo(0, 0);          // канал 0: 0 градусов / channel 0: 0 degrees
  delay(1000);
  pca.servo(0, 180);
  delay(1000);
}
```

### Мүмкіндіктері
- **Импульс емес, бұрыш:** `servo(арна, 90)`. 50 Гц жиілік, бөлгіш және тиктер өздігінен есептеледі.
- **Мекенжай өзі табылады** (0x40-0x47), мұның PCA9685 екені тексеріледі.
- **Әр сервоприводты калибрлеу:** `setServoRange(арна, минМкс, максМкс, ортаЫғысу)`, `setServoLimits`, `invertServo`. `printCalibration(Serial)` скетчке қоюға болатын жолдарды шығарады.
- **Бағдарламаны тоқтатпай жайлап қозғалу:** `servoMove(арна, бұрыш, мс)` және `loop()` ішінде `run()`.
- **Позалар:** `pose(арналар, бұрыштар, саны, мс)` және `poseAll(16 бұрыш, мс)`.
- **Артық жібермейді:** бірдей бұрыш I2C арқылы екінші рет берілмейді.
- **Жарықдиодтар мен моторлар:** `setDuty(арна, пайыз)`, `setLevel`, `setPulseUs`, `setFrequency(24..1526)`.
- **Түсінікті кеңестер** орысша, қазақша, ағылшынша: `errorText()`, `diagnose(Serial)`, `test(арна)`.
- **ESP32 / ESP8266:** `pca.begin(SDA, SCL)`.

### Қосу
| PCA9685 модулі | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| V+ (клемма питания сервоприводов) | 5-6 В отдельный источник | 5-6 В отдельный источник |

**Сервоприводтарды бөлек қоректендіріңіз:** V+ қысқышына 5-6 В, минус Arduino-мен ортақ. Тақтаның 5V пинінен сервоприводтар тартпайды.

### Басқа кітапханалардан айырмашылығы
Adafruit PWM Servo Driver, PCA9685 (NachtRaveVL) және ServoEasing қарадық:
- Adafruit «тиктерді» өзің есептеуді талап етеді (`SERVOMIN 150`); бізде микросекунд пен градус, арна бойынша калибрлеу.
- Adafruit-те жайлап қозғалу мен позалар жоқ.
- Қуат жетпесе кеңес жоқ; бізде `diagnose()`.

### Мысалдар
`Basic`, `SmoothMove`, `Calibrate`, `Pose`, `LedFade`, `Diagnose`, `ESP32_CustomPins`

### Тесттер
Жиілік бөлгіші, бұрыштарға арналған тиктер, калибрлеу, инверсия, шектер, қисық, позалар, ШИМ тексерілген: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for the **PCA9685** module (16 PWM channels over I2C): servos by **angle**, per-servo calibration, smooth moves and whole robot poses (for example the four-legged AlashSpot). Works with LEDs too. Only the built-in `Wire`, **no Adafruit PWM Servo Driver needed**.

```cpp
#include <AlashPCA9685.h>
AlashPCA9685 pca;

void setup() { pca.begin(); }
void loop() {
  pca.servo(0, 0);          // канал 0: 0 градусов / channel 0: 0 degrees
  delay(1000);
  pca.servo(0, 180);
  delay(1000);
}
```

### Features
- **Angles instead of pulses:** `servo(channel, 90)`. 50 Hz, the prescaler and ticks are computed for you.
- **Finds the address itself** (0x40-0x47) and checks it is really a PCA9685, not an INA219 on the same address.
- **Per-servo calibration:** `setServoRange(channel, minUs, maxUs, centerOffset)`, `setServoLimits`, `invertServo`. `printCalibration(Serial)` prints lines you can paste into your sketch.
- **Smooth moves without blocking:** `servoMove(channel, angle, ms)` and `run()` in `loop()` (soft start and stop).
- **Poses:** `pose(channels, angles, count, ms)` and `poseAll(16 angles, ms)`: all servos arrive together.
- **No wasted traffic:** the same angle is not sent over I2C twice.
- **LEDs and motors:** `setDuty(channel, percent)`, `setLevel`, `setPulseUs`, `setFrequency(24..1526)`.
- **Clear tips** in Russian, Kazakh and English: `errorText()`, `diagnose(Serial)` (including servo power), `test(channel)` wiggles a servo.
- **ESP32 / ESP8266:** `pca.begin(SDA, SCL)`.

### Wiring
| PCA9685 module | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| V+ (клемма питания сервоприводов) | 5-6 В отдельный источник | 5-6 В отдельный источник |

**Power servos separately:** 5-6 V on the V+ terminal, ground shared with the Arduino. The board 5V pin cannot feed servos: the board will reset and servos will jitter.

### How it differs from other libraries
We looked at Adafruit PWM Servo Driver, PCA9685 (NachtRaveVL) and ServoEasing:
- Adafruit makes you compute raw ticks (`SERVOMIN 150`) and tune them per servo; we use degrees and microseconds with per-channel calibration.
- Adafruit has no smooth moves or poses.
- No tips when power is weak or the address is taken; we have `diagnose()`.

### Examples
`Basic`, `SmoothMove`, `Calibrate`, `Pose`, `LedFade`, `Diagnose`, `ESP32_CustomPins`

### Tests
Prescaler, ticks for angles, calibration, center offset, invert, limits, smooth curve, poses, PWM and foreign-chip detection are checked without hardware: `extras/test`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
