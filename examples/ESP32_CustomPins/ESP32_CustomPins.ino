// AlashPCA9685 на ESP32 со своими пинами I2C: begin(SDA, SCL).
#include <AlashPCA9685.h>

AlashPCA9685 pca;

void setup() {
  Serial.begin(115200);
  if (!pca.begin(21, 22)) {               // SDA = GPIO21, SCL = GPIO22 (можно другие)
    Serial.println(pca.errorText());
    while (true) delay(1000);
  }
  pca.setClock(400000);                   // быстрее I2C: плавнее движения многих сервоприводов
}

void loop() {
  pca.servoMove(0, 180, 1000);
  pca.wait();
  pca.servoMove(0, 0, 1000);
  pca.wait();
}
