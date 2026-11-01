# AlashPCA9685: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для модуля **PCA9685** (16 каналов ШИМ по I2C): сервоприводы по **углам**, калибровка каждого сервопривода, плавные движения и позы целиком для роботов (например, четвероногий AlashSpot). Работает и со светодиодами. Только встроенная `Wire`, **Adafruit PWM Servo Driver не нужен**.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashPCA9685** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Подключение
| Модуль PCA9685 | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| V+ (клемма питания сервоприводов) | 5-6 В отдельный источник | 5-6 В отдельный источник |

**Сервоприводы питайте отдельно:** 5-6 В на клемму V+, минус общий с Arduino. От пина 5V платы сервоприводы не потянет: плата будет перезагружаться, сервоприводы дёргаться.

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `begin()` | Запустить модуль, найти адрес, поставить 50 Гц |
| `servo(ch, deg)` | Сразу установить угол 0-180 |
| `servoMove(ch, deg, ms)` | Плавно прийти в угол за мс (нужен run()) |
| `run() / wait()` | Обновить плавные движения / дождаться конца |
| `setServoRange(ch, min, max, off)` | Калибровка: импульсы для 0° и 180° в мкс, смещение центра |
| `setServoLimits(ch, min, max)` | Ограничить углы (защита механики) |
| `invertServo(ch)` | Обратное направление |
| `pose(chs, angles, n, ms)` | Несколько каналов сразу к целевым углам |
| `setDuty(ch, %) / setLevel(ch, hi)` | ШИМ для светодиодов и моторов |
| `setFrequency(hz)` | Частота ШИМ 24-1526 Гц |
| `servoOff(ch) / allOff()` | Снять управление (серво отдыхает) |
| `printCalibration(Serial)` | Вывести строки калибровки для скетча |
| `test(ch) / diagnose(Serial)` | Покачать серво / самопроверка |

### Если что-то не работает
**Сервоприводы дёргаются, плата перезагружается**

Не хватает питания. Подайте 5-6 В отдельным блоком на клемму V+ модуля, общий минус с Arduino, конденсатор 470-1000 мкФ на V+.

**Модуль не найден**

Проверьте SDA/SCL, VCC и GND. Адрес меняется перемычками A0-A5; библиотека ищет 0x40-0x47. Запустите пример Diagnose.

**Сервопривод не доходит до края или упирается**

Подберите импульсы: `setServoRange(канал, 500, 2500)` или уже. Пример Calibrate помогает подобрать значения.

**Сервопривод вращается в другую сторону**

Вызовите `invertServo(канал)`.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `SmoothMove`, `Calibrate`, `Pose`, `LedFade`, `Diagnose`, `ESP32_CustomPins`

---

## Қазақша

**PCA9685** модуліне (I2C арқылы 16 ШИМ арнасы) арналған қарапайым кітапхана: сервоприводтар **бұрыш** бойынша, әр сервоприводты калибрлеу, жайлап қозғалу және роботтарға арналған толық позалар. Жарықдиодтармен де жұмыс істейді. Тек кірістірілген `Wire`, **Adafruit PWM Servo Driver қажет емес**.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashPCA9685** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Қосу
| PCA9685 модулі | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| V+ (клемма питания сервоприводов) | 5-6 В отдельный источник | 5-6 В отдельный источник |

**Сервоприводтарды бөлек қоректендіріңіз:** V+ қысқышына 5-6 В, минус Arduino-мен ортақ. Тақтаның 5V пинінен сервоприводтар тартпайды.

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `begin()` | Модульді іске қосу, мекенжайды табу, 50 Гц қою |
| `servo(ch, deg)` | Бұрышты бірден қою 0-180 |
| `servoMove(ch, deg, ms)` | мс ішінде жайлап бұрышқа келу (run() керек) |
| `run() / wait()` | Жайлап қозғалысты жаңарту / аяқталуын күту |
| `setServoRange(ch, min, max, off)` | Калибрлеу: 0° және 180° импульстері мкс-пен |
| `setServoLimits(ch, min, max)` | Бұрыштарды шектеу |
| `invertServo(ch)` | Кері бағыт |
| `pose(chs, angles, n, ms)` | Бірнеше арна бірден мақсатты бұрышқа |
| `setDuty(ch, %) / setLevel(ch, hi)` | Жарықдиодтарға ШИМ |
| `setFrequency(hz)` | ШИМ жиілігі |
| `servoOff(ch) / allOff()` | Басқаруды алу |
| `printCalibration(Serial)` | Скетчке арналған калибрлеу жолдары |
| `test(ch) / diagnose(Serial)` | Серво шайқау / өзін тексеру |

### Бірдеңе жұмыс істемесе
**Сервоприводтар тартылады, тақта қайта қосылады**

Қуат жетпейді. Модульдің V+ қысқышына бөлек блоктан 5-6 В беріңіз, Arduino-мен ортақ минус, V+ бойынша 470-1000 мкФ конденсатор.

**Модуль табылмады**

SDA/SCL, VCC және GND тексеріңіз. Мекенжай A0-A5 перемычкаларымен өзгереді. Diagnose мысалын іске қосыңыз.

**Сервопривод шетіне жетпейді немесе тіреледі**

Импульстерді таңдаңыз: `setServoRange(арна, 500, 2500)`. Calibrate мысалы көмектеседі.

**Сервопривод басқа жаққа айналады**

`invertServo(арна)` шақырыңыз.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `SmoothMove`, `Calibrate`, `Pose`, `LedFade`, `Diagnose`, `ESP32_CustomPins`

---

## English

A simple library for the **PCA9685** module (16 PWM channels over I2C): servos by **angle**, per-servo calibration, smooth moves and whole robot poses (for example the four-legged AlashSpot). Works with LEDs too. Only the built-in `Wire`, **no Adafruit PWM Servo Driver needed**.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashPCA9685** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Wiring
| PCA9685 module | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| V+ (клемма питания сервоприводов) | 5-6 В отдельный источник | 5-6 В отдельный источник |

**Power servos separately:** 5-6 V on the V+ terminal, ground shared with the Arduino. The board 5V pin cannot feed servos: the board will reset and servos will jitter.

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `begin()` | Start the module, find the address, set 50 Hz |
| `servo(ch, deg)` | Set an angle 0-180 immediately |
| `servoMove(ch, deg, ms)` | Move smoothly in ms (needs run()) |
| `run() / wait()` | Update smooth moves / wait until done |
| `setServoRange(ch, min, max, off)` | Calibration: pulses for 0° and 180° in us, center offset |
| `setServoLimits(ch, min, max)` | Limit the angles (protects the mechanics) |
| `invertServo(ch)` | Reverse direction |
| `pose(chs, angles, n, ms)` | Several channels to target angles at once |
| `setDuty(ch, %) / setLevel(ch, hi)` | PWM for LEDs and motors |
| `setFrequency(hz)` | PWM frequency 24-1526 Hz |
| `servoOff(ch) / allOff()` | Release control (servo relaxes) |
| `printCalibration(Serial)` | Print calibration lines for your sketch |
| `test(ch) / diagnose(Serial)` | Wiggle a servo / self-check |

### If something does not work
**Servos jitter, the board resets**

Not enough power. Feed 5-6 V from a separate supply to the V+ terminal, common ground with the Arduino, 470-1000 uF capacitor on V+.

**The module is not found**

Check SDA/SCL, VCC and GND. The address is changed with A0-A5 jumpers; the library searches 0x40-0x47. Run the Diagnose example.

**The servo does not reach the end or hits it**

Tune pulses: `setServoRange(channel, 500, 2500)` or narrower. The Calibrate example helps.

**The servo turns the other way**

Call `invertServo(channel)`.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `SmoothMove`, `Calibrate`, `Pose`, `LedFade`, `Diagnose`, `ESP32_CustomPins`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashPCA9685
