// [ru] AlashPCA9685: самый простой пример. Сервопривод на канале 0 качается от 0 до 180 градусов.
//
//      Подключение: модуль PCA9685 SDA/SCL к SDA/SCL платы (Uno: A4/A5, ESP32: GPIO21/22), VCC 5 В, GND общий.
//      Сервопривод в разъём канала 0. На клемму V+ модуля подайте 5-6 В от отдельного источника (минус общий с Arduino).
// [kk] AlashPCA9685: ең қарапайым мысал. 0 арнасындағы серво 0-ден 180 градусқа дейін тербеледі.
//
//      Қосу: PCA9685 модулінің SDA/SCL тақтаның SDA/SCL-іне (Uno: A4/A5, ESP32: GPIO21/22), VCC 5 В, GND ортақ.
//      Серво 0 арна қосқышына. Модульдің V+ қыстырғышына бөлек көзден 5-6 В беріңіз (минус Arduino-мен ортақ).
// [en] AlashPCA9685: the simplest example. A servo on channel 0 sweeps from 0 to 180 degrees.
//
//      Wiring: the PCA9685 module SDA/SCL to the board SDA/SCL (Uno: A4/A5, ESP32: GPIO21/22), VCC 5 V, GND common.
//      The servo into the channel 0 connector. Give the module's V+ terminal 5-6 V from a separate supply (minus common with the Arduino).
#include <AlashPCA9685.h>

AlashPCA9685 pca;

void setup() {
  Serial.begin(115200);
  if (!pca.begin()) {                     // сам найдёт модуль и поставит 50 Гц
    Serial.println(pca.errorText());      // объяснит, что проверить
    while (true) delay(1000);
  }
}

void loop() {
  pca.servo(0, 0);                        // канал 0, угол 0 градусов
  delay(1000);
  pca.servo(0, 90);
  delay(1000);
  pca.servo(0, 180);
  delay(1000);
}
