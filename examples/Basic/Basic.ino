// AlashPCA9685: самый простой пример. Сервопривод на канале 0 качается от 0 до 180 градусов.
//
// Подключение: модуль PCA9685 SDA/SCL к SDA/SCL платы (Uno: A4/A5, ESP32: GPIO21/22), VCC 5 В, GND общий.
// Сервопривод в разъём канала 0. На клемму V+ модуля подайте 5-6 В от отдельного источника (минус общий с Arduino).
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
