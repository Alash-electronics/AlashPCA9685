// AlashPCA9685: плавное свечение светодиодов. PCA9685 это не только сервоприводы: каждый канал даёт ШИМ 0-100 %.
// Светодиод через резистор 220 Ом между каналом и GND (или готовая светодиодная лента на 5 В через транзистор).
#include <AlashPCA9685.h>

AlashPCA9685 pca;

void setup() {
  if (!pca.begin()) {
    Serial.begin(115200);
    Serial.println(pca.errorText());
    while (true) delay(1000);
  }
  pca.setFrequency(1000);                 // для светодиодов выше частота, чем 50 Гц сервоприводов: нет мерцания
}

void loop() {
  for (int p = 0; p <= 100; p += 2) {     // нарастание
    for (uint8_t ch = 0; ch < 4; ch++) pca.setDuty(ch, p * (ch + 1) / 4.0);   // у каждого канала своя яркость
    delay(15);
  }
  for (int p = 100; p >= 0; p -= 2) {     // затухание
    for (uint8_t ch = 0; ch < 4; ch++) pca.setDuty(ch, p * (ch + 1) / 4.0);
    delay(15);
  }
}
