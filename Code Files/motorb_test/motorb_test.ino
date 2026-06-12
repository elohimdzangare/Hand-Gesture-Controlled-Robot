#define ENB 27
#define IN3 25
#define IN4 26

#define PWM_FREQ 1000
#define PWM_RES  8

void setup() {
  Serial.begin(115200);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  ledcAttach(ENB, PWM_FREQ, PWM_RES);

  Serial.println("Testing Motor B backward...");

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(ENB, 145);
}

void loop() {}