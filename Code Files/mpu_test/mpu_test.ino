#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  Wire.begin();  // SDA = D21, SCL = D22
  delay(1000);

  Serial.println("Scanning I2C bus...");
  bool found = false;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Device found at 0x");
      Serial.println(addr, HEX);
      found = true;
    }
  }
  if (!found) {
    Serial.println("No I2C devices found — check wiring!");
    while (true);  // halt
  }

  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 connected successfully!");
  } else {
    Serial.println("MPU6050 connection failed — check wiring!");
    while (true);  // halt
  }
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  Serial.println("--- MPU6050 Readings ---");
  Serial.print("Accel  X: "); Serial.print(ax);
  Serial.print("  Y: ");      Serial.print(ay);
  Serial.print("  Z: ");      Serial.println(az);

  Serial.print("Gyro   X: "); Serial.print(gx);
  Serial.print("  Y: ");      Serial.print(gy);
  Serial.print("  Z: ");      Serial.println(gz);

  Serial.println();
  delay(500);
}