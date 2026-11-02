// [ru] AlashPCA9685: позы. Четыре сервопривода (каналы 0-3) переходят из одной позы в другую одновременно и плавно,
//      как ноги робота: "сесть" и "встать". Позу задают массивом углов.
// [kk] AlashPCA9685: қалыптар. Төрт серво (0-3 арналар) бір қалыптан екіншісіне бір мезгілде және баяу ауысады,
//      робот аяқтары сияқты: «отыру» және «тұру». Қалып бұрыштар массивімен беріледі.
// [en] AlashPCA9685: poses. Four servos (channels 0-3) go from one pose to another at the same time and smoothly,
//      like the legs of a robot: "sit" and "stand". A pose is set by an array of angles.
#include <AlashPCA9685.h>

AlashPCA9685 pca;
const uint8_t channels[4] = {0, 1, 2, 3};
const float stand[4] = {90, 90, 90, 90};
const float sit[4] = {60, 120, 60, 120};

void setup() {
  if (!pca.begin()) {
    Serial.begin(115200);
    Serial.println(pca.errorText());
    while (true) delay(1000);
  }
  for (uint8_t i = 0; i < 4; i++) pca.servo(channels[i], stand[i]);   // стартовая поза
  delay(1000);
}

void loop() {
  pca.pose(channels, sit, 4, 1200);       // за 1.2 с все ноги приходят одновременно
  pca.wait();                             // wait() дожидается конца; можно вместо него вызывать run() в loop()
  delay(1000);
  pca.pose(channels, stand, 4, 1200);
  pca.wait();
  delay(1000);
}
