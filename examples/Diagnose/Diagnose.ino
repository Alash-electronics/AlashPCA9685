// [ru] AlashPCA9685: модуль не работает? Скетч покажет все устройства на шине I2C и подскажет, что проверить
//      (в том числе про питание сервоприводов).
// [kk] AlashPCA9685: модуль жұмыс істемей ме? Скетч I2C шинасындағы барлық құрылғыны көрсетеді және нені тексеру керегін айтады
//      (серво қуатын қоса алғанда).
// [en] AlashPCA9685: the module does not work? The sketch shows all the devices on the I2C bus and tells what to check
//      (including the servo power).
#include <AlashPCA9685.h>

AlashPCA9685 pca;

void setup() {
  Serial.begin(115200);
  delay(500);
  pca.setLanguage(AlashPCA9685::LANG_RU);   // LANG_KK: қазақша, LANG_EN: English
  bool ok = pca.begin();
  Serial.println(ok ? "Модуль найден" : "Модуль НЕ найден");
  pca.diagnose(Serial);
  // Закомментируйте строку ниже, чтобы не двигать сервопривод:
  if (ok) pca.test(0);                    // качнёт сервопривод на канале 0
}

void loop() {}
