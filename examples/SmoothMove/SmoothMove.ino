// [ru] AlashPCA9685: плавное движение без остановки программы. Сервопривод идёт за 1.5 секунды с мягким стартом и торможением,
//      а светодиод всё это время мигает. run() нужно вызывать в loop() как можно чаще.
// [kk] AlashPCA9685: бағдарламаны тоқтатпай баяу қозғалыс. Серво 1.5 секундта жұмсақ бастап, жұмсақ тоқтайды,
//      осы уақытта жарықдиод жыпылықтап тұрады. run() функциясын loop() ішінде мүмкіндігінше жиі шақыру керек.
// [en] AlashPCA9685: smooth motion without stopping the program. The servo moves in 1.5 seconds with a soft start and stop,
//      while an LED blinks all that time. run() must be called in loop() as often as possible.
#include <AlashPCA9685.h>

AlashPCA9685 pca;
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif
uint32_t lastBlink = 0;
bool toEnd = true;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  if (!pca.begin()) {
    Serial.begin(115200);
    Serial.println(pca.errorText());
    while (true) delay(1000);
  }
  pca.servo(0, 0);                        // начальное положение (плавно можно двигаться из известного положения)
  delay(500);
}

void loop() {
  pca.run();                              // двигает сервопривод по таймеру

  if (!pca.isMoving()) {                  // дошёл: едем обратно
    delay(500);
    pca.servoMove(0, toEnd ? 180 : 0, 1500);
    toEnd = !toEnd;
  }

  if (millis() - lastBlink > 200) {       // параллельно мигаем
    lastBlink = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
