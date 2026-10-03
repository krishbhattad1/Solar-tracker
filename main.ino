#include <Servo.h>

// ---------------- Pins ----------------
const int LDR_LEFT  = A0;   // LDR 1
const int LDR_RIGHT = A1;   // LDR 2
const int SERVO_PIN = 9;

// ---------------- Servo ----------------
Servo tracker;
int angle = 90;
int lastAngle = 90;

// ---------------- Limits ----------------
const int MIN_ANGLE = 5;
const int MAX_ANGLE = 175;

// ---------------- Control ----------------
int offset = 0;              // sensor bias
const int SMALL_ERR = 40;    // ignore tiny error (NO movement)
const int UPDATE_TIME = 300; // ms between decisions

unsigned long lastUpdate = 0;

// ---------------- LDR Filter ----------------
int readLDR(int pin) {
  long sum = 0;
  for (int i = 0; i < 15; i++) {
    sum += analogRead(pin);
    delay(3);
  }
  return sum / 15;
}

void setup() {
  tracker.attach(SERVO_PIN);
  tracker.write(angle);
  Serial.begin(9600);

  // -------- Auto calibration --------
  delay(2000); // keep both LDRs under same light
  int left  = readLDR(LDR_LEFT);
  int right = readLDR(LDR_RIGHT);
  offset = right - left;   // compensate LDR mismatch

  Serial.print("Offset = ");
  Serial.println(offset);
}

void loop() {

  if (millis() - lastUpdate < UPDATE_TIME) return;
  lastUpdate = millis();

  int left  = readLDR(LDR_LEFT);
  int right = readLDR(LDR_RIGHT);

  // -------- Error (DIRECTION CAN BE FLIPPED LATER) --------
  int error = (right - left) - offset;
  // If direction is wrong later, change to:
  // int error = (left - right) - offset;

  Serial.print("L=");
  Serial.print(left);
  Serial.print(" R=");
  Serial.print(right);
  Serial.print(" Err=");
  Serial.print(error);
  Serial.print(" Ang=");
  Serial.println(angle);

  // -------- Small error zone (NO shaking) --------
  if (abs(error) < SMALL_ERR) {
    tracker.detach();   // stop PWM → NO jitter
    return;
  }

  // ensure servo is active when movement is needed
  if (!tracker.attached()) {
    tracker.attach(SERVO_PIN);
  }

  // -------- Variable step (smooth & precise) --------
  int step;
  int absErr = abs(error);

  if (absErr > 300) step = 3;
  else if (absErr > 150) step = 2;
  else step = 1;

  if (error > 0 && angle < MAX_ANGLE) {
    angle += step;
  }
  else if (error < 0 && angle > MIN_ANGLE) {
    angle -= step;
  }

  angle = constrain(angle, MIN_ANGLE, MAX_ANGLE);

  // -------- Write ONLY if changed --------
  if (angle != lastAngle) {
    tracker.write(angle);
    lastAngle = angle;
  }
}