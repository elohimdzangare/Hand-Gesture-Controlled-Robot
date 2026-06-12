#include <esp_now.h>
#include <WiFi.h>

#define ENA 18
#define IN1 19
#define IN2 21

#define ENB 27
#define IN3 33
#define IN4 26

#define PWM_FREQ 1000
#define PWM_RES  8

#define SPEED 145
#define TIMEOUT_MS 5000  // stop motors if no message for 10 seconds

typedef struct {
  int command;
} GestureData;

GestureData gesture;
unsigned long lastReceived = 0;
bool connectionLost = false;

void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // Motor A forward
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);  // Motor B forward
  ledcWrite(ENA, SPEED);
  ledcWrite(ENB, SPEED);
}

void backward() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);  // Motor A backward
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // Motor B backward
  ledcWrite(ENA, SPEED);
  ledcWrite(ENB, SPEED);
}

// The following code was used to test the backward code
//void backward() {
//  Serial.println("backward() called");
//  Serial.print("IN3: HIGH, IN4: LOW, ENB: "); Serial.println(SPEED);
//  
//  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
//  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
//  ledcWrite(ENA, SPEED);
//  ledcWrite(ENB, SPEED);
//}

void turnLeft() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  // Motor A forward
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  // Motor B backwards
  ledcWrite(ENA, SPEED);
  ledcWrite(ENB, SPEED);
}

void turnRight() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); // Motor A backwards
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); // Motor B forward
  ledcWrite(ENA, SPEED); 
  ledcWrite(ENB, SPEED);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
}

void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memcpy(&gesture, data, sizeof(gesture));
  lastReceived = millis();  // update timestamp on every message including heartbeat

  if (connectionLost) {
    connectionLost = false;
    Serial.println("Connection restored!");
  }

  if (gesture.command == 99) {
    Serial.println("Heartbeat received");
    return;  // don't execute any motor command for heartbeat
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

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);
  lastReceived = millis();  // initialise so it doesn't trigger immediately on boot
  Serial.println("Motor ESP32 ready, waiting for commands...");
}

void loop() {
  if (millis() - lastReceived > TIMEOUT_MS && !connectionLost) {
    connectionLost = true;
    stopMotors();
    Serial.println("Connection lost — motors stopped!");
  }
}