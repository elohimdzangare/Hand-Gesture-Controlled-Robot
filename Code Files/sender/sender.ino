#include <Wire.h>
#include <MPU6050.h>
#include <esp_now.h>
#include <WiFi.h>

MPU6050 mpu;

uint8_t motorMAC[] = {0x00, 0x70, 0x07, 0xA3, 0x73, 0x10};

typedef struct {
  int command;  // 0=stop, 1=forward, 2=backward, 3=left, 4=right, 99=heartbeat
} GestureData;

GestureData gesture;

#define TILT_THRESHOLD 8000
#define HEARTBEAT_INTERVAL 2000  // send heartbeat every 2 seconds

esp_now_peer_info_t peerInfo;
unsigned long lastHeartbeat = 0;

void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(1000);

  mpu.initialize();
  if (mpu.testConnection())
    Serial.println("MPU6050 connected!");
  else
    Serial.println("MPU6050 failed!");

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(onSent);

  memcpy(peerInfo.peer_addr, motorMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
    Serial.println("Failed to add peer");
  else
    Serial.println("Peer added successfully!");
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  int prevCommand = gesture.command;

  if (ay > TILT_THRESHOLD)
    gesture.command = 1;
  else if (ay < -TILT_THRESHOLD)
    gesture.command = 2;
  else if (ax > TILT_THRESHOLD)
    gesture.command = 3;
  else if (ax < -TILT_THRESHOLD)
    gesture.command = 4;
  else
    gesture.command = 0;

  // Send if command changed
  if (gesture.command != prevCommand) {
    esp_now_send(motorMAC, (uint8_t *)&gesture, sizeof(gesture));

    Serial.print("Sent command: ");
    switch (gesture.command) {
      case 0: Serial.println("STOP");     break;
      case 1: Serial.println("FORWARD");  break;
      case 2: Serial.println("BACKWARD"); break;
      case 3: Serial.println("LEFT");     break;
      case 4: Serial.println("RIGHT");    break;
    }

    lastHeartbeat = millis();  // reset heartbeat timer after a real command
  }

  // Send heartbeat if no command has been sent recently
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    GestureData heartbeat;
    heartbeat.command = 99;  // 99 = heartbeat, not a movement command
    esp_now_send(motorMAC, (uint8_t *)&heartbeat, sizeof(heartbeat));
    Serial.println("Heartbeat sent");
    lastHeartbeat = millis();
  }

  delay(100);
}