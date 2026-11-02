// [ru] AlashPCA9685: калибровка сервопривода. У разных сервоприводов края хода разные: один идёт от 500 до 2500 мкс,
//      другой от 700 до 2300. Если упирается, гудит или не доходит до края, подберите числа здесь.
//
//      Через монитор порта (115200) отправляйте: число 500..2500 (импульс в микросекундах) или буквы
//        m  вывести найденные значения для вставки в скетч.
//      Двигайте импульс по 50 мкс. Запомните число, где сервопривод только-только дошёл до края, не гудя.
// [kk] AlashPCA9685: серваны калибрлеу. Әртүрлі серволарда жүріс ұштары әртүрлі: біреуі 500-ден 2500 мкс-қа дейін,
//      екіншісі 700-ден 2300-ге дейін. Тірелсе, ызыңдаса немесе шетіне жетпесе, сандарды осы жерде таңдаңыз.
//
//      Порт монитор арқылы (115200) жіберіңіз: 500..2500 саны (микросекундпен импульс) немесе әріп
//        m  табылған мәндерді скетчке қою үшін шығару.
//      Импульсті 50 мкс қадаммен жылжытыңыз. Серво ызыңдамай шетіне жаңа ғана жеткен санды есте сақтаңыз.
// [en] AlashPCA9685: servo calibration. Different servos have different travel ends: one goes from 500 to 2500 us,
//      another from 700 to 2300. If it hits the stop, hums or does not reach the end, find the numbers here.
//
//      Through the serial monitor (115200) send: a number 500..2500 (the pulse in microseconds) or the letter
//        m  print the found values to paste into the sketch.
//      Move the pulse in steps of 50 us. Remember the number where the servo has just reached the end without humming.
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
  Serial.println(F("Отправьте импульс в мкс (например 1500). Подберите край 0 градусов, потом 180."));
  Serial.println(F("Команда: l = запомнить как 0 градусов, h = запомнить как 180, m = показать итог"));
  pca.setPulseUs(CHANNEL, 1500);
}

uint16_t lastUs = 1500;

void loop() {
  if (!Serial.available()) return;
  char c = Serial.peek();
  if (c == 'l') { lowUs = lastUs; Serial.print(F("0 градусов = ")); Serial.println(lowUs); Serial.read(); return; }
  if (c == 'h') { highUs = lastUs; Serial.print(F("180 градусов = ")); Serial.println(highUs); Serial.read(); return; }
  if (c == 'm') {
    Serial.read();
    pca.setServoRange(CHANNEL, lowUs, highUs, 0);
    Serial.println(F("Вставьте в setup():"));
    pca.printCalibration(Serial);         // готовая строка setServoRange(...)
    pca.servo(CHANNEL, 90);
    return;
  }
  int us = Serial.parseInt();
  if (us >= 400 && us <= 2600) {
    lastUs = us;
    pca.setPulseUs(CHANNEL, us);
    Serial.print(F("Импульс ")); Serial.print(us); Serial.println(F(" мкс"));
  }
}
