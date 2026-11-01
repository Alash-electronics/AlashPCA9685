// AlashPCA9685: калибровка сервопривода. У разных сервоприводов края хода разные: один идёт от 500 до 2500 мкс,
// другой от 700 до 2300. Если упирается, гудит или не доходит до края, подберите числа здесь.
//
// Через монитор порта (115200) отправляйте: число 500..2500 (импульс в микросекундах) или буквы
//   m  вывести найденные значения для вставки в скетч.
// Двигайте импульс по 50 мкс. Запомните число, где сервопривод только-только дошёл до края, не гудя.
#include <AlashPCA9685.h>

AlashPCA9685 pca;
const uint8_t CHANNEL = 0;
uint16_t lowUs = 600, highUs = 2400;

void setup() {
  Serial.begin(115200);
  if (!pca.begin()) {
    Serial.println(pca.errorText());
    while (true) delay(1000);
  }
  Serial.println("Отправьте импульс в мкс (например 1500). Подберите край 0 градусов, потом 180.");
  Serial.println("Команда: l = запомнить как 0 градусов, h = запомнить как 180, m = показать итог");
  pca.setPulseUs(CHANNEL, 1500);
}

uint16_t lastUs = 1500;

void loop() {
  if (!Serial.available()) return;
  char c = Serial.peek();
  if (c == 'l') { lowUs = lastUs; Serial.print("0 градусов = "); Serial.println(lowUs); Serial.read(); return; }
  if (c == 'h') { highUs = lastUs; Serial.print("180 градусов = "); Serial.println(highUs); Serial.read(); return; }
  if (c == 'm') {
    Serial.read();
    pca.setServoRange(CHANNEL, lowUs, highUs, 0);
    Serial.println("Вставьте в setup():");
    pca.printCalibration(Serial);         // готовая строка setServoRange(...)
    pca.servo(CHANNEL, 90);
    return;
  }
  int us = Serial.parseInt();
  if (us >= 400 && us <= 2600) {
    lastUs = us;
    pca.setPulseUs(CHANNEL, us);
    Serial.print("Импульс "); Serial.print(us); Serial.println(" мкс");
  }
}
