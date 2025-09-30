#include <Servo.h>
#include <math.h>
#include <Arduino.h>

// --- Servo objects (axes) ---
Servo ServoY;   // yaw (left/right) - torso
Servo ServoX1;  // pitch (up/down)  - torso
Servo ServoX2;  // pitch (up/down)  - elbow
// Servo ServoX3; // yaw (left/right) - head (unused)
Servo ServoX4;  // surge (forward/back) - head

// --- Pins ---
const uint8_t PIN_SERVO_Y  = 12;
const uint8_t PIN_SERVO_X1 = 11;
const uint8_t PIN_SERVO_X2 = 10;
// const uint8_t PIN_SERVO_X3 = 9;   // unused
const uint8_t PIN_SERVO_X4 = 8;
const uint8_t PIN_PUMP     = 4;

// --- Timings ---
const int SERVO_STEP_DELAY_MS = 500;   // wait between individual servo moves

// --- Home position (camera sees entire scene) ---
const int HOME_Y  = 90;   // larger angle -> left turn (arm perspective)
const int HOME_X1 = 110;  // smaller angle -> move downward
const int HOME_X2 = 115;  // smaller angle -> move upward
// const int HOME_X3 = 5;  // larger angle -> right turn (arm perspective)
const int HOME_X4 = 85;   // larger angle -> move upward, hose side

// --- Base position (arm aligned to shot glass height at image center) ---
const int BASE_Y  = 90;
const int BASE_X1 = 80;
const int BASE_X2 = 140;
//const int BASE_X3 = 5;
const int BASE_X4 = 145;  // used as base for X4

// --- Geometry / kinematics (image <-> world) ---
const float BASE_X1_ABS_ANGLE_DEG = 53.0f; // absolute X1 angle to ground when BASE_X1 = 80
const float IMG_WIDTH_Y_CM = 22.5f;        // image width in Y direction (cm)
const float IMG_WIDTH_X_CM = 16.5f;        // image width in X direction (cm)
const float CAM_CENTER_Y_CM = 11.25f;
const float CAM_CENTER_X_CM = 8.25f;
const float PX_PER_CM_Y = 28.44f;          // 480 px over 16.5 cm -> 480/16.5 = 28.44
const float PX_PER_CM_X = 29.09f;          // 640 px over 22.5 cm -> 640/22.5 = 29.09
const int   ARM_LINK_LEN_CM = 12;          // both arm links equal length
const float X1_TO_IMG_EDGE_CM = 6.0f;      // ~6 cm from X1 servo to image edge
const int   HOSE_TO_CAM_CORR_DEG = 8;      // correction (deg) to compensate hose vs camera offset (~2.5 cm)

// --- Prototypes ---
void attachServos();                         // attach servos to pins
void moveToHomePosition();                   // go to home (overview) position
void detachServos();                         // detach all servos
void processData();                          // handle 4-byte UART payload (val1,val2)
void computeAngles(int16_t val1, int16_t val2);
void activatePump();                         // simple pump pulse on PIN 4
void sendAck(uint8_t statusCode);            // 1-byte ACK (0x01 = OK)
void sendAckText(uint8_t statusCode);        // optional text ACK: "OK:<n>\n"

// --- Setup / Loop ---
void setup() {
  Serial.begin(9600);
  pinMode(PIN_PUMP, OUTPUT);
  digitalWrite(PIN_PUMP, LOW);

  attachServos();
  moveToHomePosition();
  detachServos();
}

void loop() {
  while (Serial.available() > 0) {
    processData();
  }
}

// --- Servo helpers ---
void attachServos() {
  ServoY.attach(PIN_SERVO_Y);
  ServoX1.attach(PIN_SERVO_X1);
  ServoX2.attach(PIN_SERVO_X2);
  // ServoX3.attach(PIN_SERVO_X3);
  ServoX4.attach(PIN_SERVO_X4);
}

void moveToHomePosition() {
  ServoY.write(HOME_Y);
  delay(SERVO_STEP_DELAY_MS);

  ServoX2.write(HOME_X2);
  delay(SERVO_STEP_DELAY_MS);

  ServoX1.write(HOME_X1);
  delay(SERVO_STEP_DELAY_MS);
  // ServoX3.write(HOME_X3);
  // delay(SERVO_STEP_DELAY_MS);

  ServoX4.write(HOME_X4);
  delay(SERVO_STEP_DELAY_MS);
  delay(SERVO_STEP_DELAY_MS);
}

void detachServos() {
  ServoY.detach();
  ServoX1.detach();
  ServoX2.detach();
  // ServoX3.detach();
  ServoX4.detach();
}

// --- Serial I/O ---
void processData() {
  // Expect exactly 4 bytes: val1 (LE), val2 (LE)
  if (Serial.available() >= 4) {
    int16_t val1 = (int16_t)(Serial.read() | (Serial.read() << 8)); // little-endian
    int16_t val2 = (int16_t)(Serial.read() | (Serial.read() << 8)); // little-endian

    computeAngles(val1, val2);
    activatePump();
    moveToHomePosition();
    detachServos();

    sendAck(0x01);    // Binary ACK: 0x01 = done/OK
  }
}

// --- Kinematics / motion ---
void computeAngles(int16_t val1, int16_t val2) {
  // val1: pixel Y (0..480), val2: pixel X (0..640)
  // Convert to cm relative to arm
  float x_cm, y_cm;
  int   y_cmd, x1_cmd, x2_cmd, x4_cmd;
  float yaw_deg, pitch_deg, hyp_cm;

  // Distance X from arm to target (cm)
  x_cm = IMG_WIDTH_X_CM + X1_TO_IMG_EDGE_CM - (float(val2) / PX_PER_CM_X);

  // Y sign: right of arm vs left of arm (arm perspective)
  if (float(val1) / PX_PER_CM_Y >= CAM_CENTER_Y_CM) {
    // object right of arm
    y_cm   = (float(val1) / PX_PER_CM_Y) - CAM_CENTER_Y_CM;
    yaw_deg = atan(y_cm / x_cm) * (180.0f / PI);
    y_cmd  = HOME_Y - (int)ceilf(yaw_deg);
  } else {
    // object left of arm
    y_cm   = CAM_CENTER_Y_CM - (float(val1) / PX_PER_CM_Y);
    yaw_deg = atan(y_cm / x_cm) * (180.0f / PI);
    y_cmd  = HOME_Y + (int)ceilf(yaw_deg);
  }

  // Hypotenuse from arm base to target
  hyp_cm   = sqrtf(y_cm * y_cm + x_cm * x_cm);

  // Elbow/shoulder pitch using law of cosines (links equal length)
  // cos(theta) = (d^2 + L^2 - L^2) / (2 L d) = d / (2L)
  pitch_deg = acosf((hyp_cm * hyp_cm + ARM_LINK_LEN_CM * ARM_LINK_LEN_CM - ARM_LINK_LEN_CM * ARM_LINK_LEN_CM)
                    / (2.0f * ARM_LINK_LEN_CM * hyp_cm)) * (180.0f / PI);

  int delta = (int)fabsf(ceilf(pitch_deg) - BASE_X1_ABS_ANGLE_DEG);

  if (pitch_deg >= BASE_X1_ABS_ANGLE_DEG) {
    // target between image center and arm
    x1_cmd = BASE_X1 + delta;
    x2_cmd = BASE_X2 + 2 * delta;
    x4_cmd = BASE_X4 + delta;
  } else {
    // target beyond image center (farther away)
    x1_cmd = BASE_X1 - delta;
    x2_cmd = BASE_X2 - 2 * delta;
    x4_cmd = BASE_X4 - delta;
  }

  //additional correction for hose vs camera offset:
  x1_cmd += HOSE_TO_CAM_CORR_DEG;
  x2_cmd += 2 * HOSE_TO_CAM_CORR_DEG;
  x4_cmd += HOSE_TO_CAM_CORR_DEG;

  // Execute motion
  attachServos();
  ServoY.write(y_cmd);
  delay(SERVO_STEP_DELAY_MS);

  ServoX1.write(x1_cmd);
  delay(SERVO_STEP_DELAY_MS);

  ServoX2.write(x2_cmd);
  delay(SERVO_STEP_DELAY_MS);

  ServoX4.write(x4_cmd);
  delay(3000);
}


// --- Actuators ---
void activatePump() {
  digitalWrite(PIN_PUMP, HIGH);
  delay(2000);
  digitalWrite(PIN_PUMP, LOW);
  delay(2000);
}

// --- Acknowledgements back to host ---
void sendAck(uint8_t statusCode) {
  Serial.write(statusCode);
  Serial.flush();
}

void sendAckText(uint8_t statusCode) {
  Serial.print("OK:");
  Serial.print(statusCode);
  Serial.print('\n');
  Serial.flush();
}