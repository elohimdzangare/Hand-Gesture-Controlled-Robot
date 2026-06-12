#include <esp_now.h>
#include <WiFi.h>

#define ENA 18
#define IN1 19
#define IN2 21

#define ENB 27
#define IN3 33
#define IN4 26

#define TRIG 17
#define ECHO 16

#define PWM_FREQ 1000
#define PWM_RES  8

#define SPEED 145
#define TIMEOUT_MS 10000
#define STOP_DISTANCE 20

typedef struct {
  int command;
} GestureData;

GestureData gesture;
unsigned long lastReceived = 0;
bool connectionLost = false;

float getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  return duration * 0.034 / 2;
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
}

void forward() {
  float distance = getDistance();
  Serial.print("Distance: "); Serial.print(distance); Serial.println("cm");

  if (distance > 0 && distance < STOP_DISTANCE) {
    Serial.println("Obstacle detected — stopping!");
    stopMotors();
    return;
  }

  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  ledcWrite(ENA, SPEED);
  ledcWrite(ENB, SPEED);
}

void backward() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(ENA, SPEED);
  ledcWrite(ENB, SPEED);
}

void turnLeft() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  ledcWrite(ENA, SPEED);
  ledcWrite(ENB, SPEED);
}

void turnRight() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  ledcWrite(ENA, SPEED);
  ledcWrite(ENB, SPEED);
}

void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memcpy(&gesture, data, sizeof(gesture));
  lastReceived = millis();

  if (connectionLost) {
    connectionLost = false;
    Serial.println("Connection restored!");
  }

  if (gesture.command == 99) {
    Serial.println("Heartbeat received");
    return;
  }

  Serial.print("Received command: ");
  switch (gesture.command) {
    case 0: Serial.println("STOP");     stopMotors(); break;
    case 1: Serial.println("FORWARD");  forward();    break;
    case 2: Serial.println("BACKWARD"); backward();   break;
    case 3: Serial.println("LEFT");     turnLeft();   break;
    case 4: Serial.println("RIGHT");    turnRight();  break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  ledcAttach(ENA, PWM_FREQ, PWM_RES);
  ledcAttach(ENB, PWM_FREQ, PWM_RES);
  stopMotors();

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);
  lastReceived = millis();
  Serial.println("Motor ESP32 ready, waiting for commands...");
}

void loop() {
  if (millis() - lastReceived > TIMEOUT_MS && !connectionLost) {
    connectionLost = true;
    stopMotors();
    Serial.println("Connection lost — motors stopped!");
  }
}