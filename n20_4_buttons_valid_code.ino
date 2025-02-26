#include <Bounce2.h>

// Motor control pins
#define IN1 7
#define IN2 6
#define ENA 9

// Encoder pins
#define ENCODER_C1 2
#define ENCODER_C2 3

// Button pins
#define BUTTON_DECREASE_BACKWARD_PIN 4
#define BUTTON_DECREMENT_PIN 10
#define BUTTON_INCREMENT_PIN 11
#define BUTTON_TOGGLE_PIN 12

// Variables
volatile long encoderCount = 0;
const int CPR = 28;                   // Counts per revolution (7 PPR * 4 quadrature steps)
const int incrementSteps = 1 * CPR;    // Steps per increment
int motorSpeed = 150;                  // Motor speed (0 to 255)
bool motorRunning = false;              // Tracks if the motor is running
long targetSteps = 0;                   // Total steps updated when incrementing or decrementing
long lastExecutedSteps = 0;             // Stores last executed step count

// Button instances
Bounce decreaseBackwardButton = Bounce();
Bounce incrementButton = Bounce();
Bounce decrementButton = Bounce();
Bounce toggleButton = Bounce();

void setup() {
  Serial.begin(115200);

  // Motor setup
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  stopMotor();

  // Encoder setup
  pinMode(ENCODER_C1, INPUT);
  pinMode(ENCODER_C2, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODER_C1), readEncoder, CHANGE);

  // Button setup
  pinMode(BUTTON_INCREMENT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DECREMENT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_TOGGLE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DECREASE_BACKWARD_PIN, INPUT_PULLUP);

  incrementButton.attach(BUTTON_INCREMENT_PIN);
  decrementButton.attach(BUTTON_DECREMENT_PIN);
  toggleButton.attach(BUTTON_TOGGLE_PIN);
  decreaseBackwardButton.attach(BUTTON_DECREASE_BACKWARD_PIN);

  incrementButton.interval(25);
  decrementButton.interval(25);
  toggleButton.interval(25);
  decreaseBackwardButton.interval(25);
}

void loop() {
  incrementButton.update();
  decrementButton.update();
  toggleButton.update();
  decreaseBackwardButton.update();

  // ✅ When Increment is Pressed → Increase Steps and Save Last Executed Steps
  if (incrementButton.fell() && !motorRunning) {
    Serial.println("Increment Button Pressed");
    targetSteps += incrementSteps;
    lastExecutedSteps = targetSteps;  // Store last valid step count
    Serial.print("Updated Steps: ");
    Serial.println(targetSteps);
    executeMotorMovement();
  }

  // ✅ When Decrement is Pressed → Decrease Steps and Save Last Executed Steps
  if (decrementButton.fell() && !motorRunning) {
    Serial.println("Decrement Button Pressed");

    if (targetSteps >= incrementSteps) {
      targetSteps -= incrementSteps;
      lastExecutedSteps = targetSteps;  // Store last valid step count
      Serial.print("Updated Steps: ");
      Serial.println(targetSteps);
      executeMotorMovement();
    } else {
      Serial.println("Cannot Decrease Below Zero");
    }
  }

  // ✅ Toggle Button → Repeat Last Executed Step Count
  if (toggleButton.fell()) {
    if (!motorRunning) {
      Serial.println("Toggle Button Pressed: Repeating Last Movement");
      targetSteps = lastExecutedSteps;  // Use last executed steps
      executeMotorMovement();
    } else {
      stopMotor();
      Serial.println("Motor Stopped via Toggle");
    }
  }

  // ✅ Decrease Backward Button: Move Motor Backward While Pressed
  if (decreaseBackwardButton.read() == LOW && !motorRunning) {
    Serial.println("Backward Button Pressed - Moving Backward");
    moveBackwardsContinuous();
  } else if (motorRunning && decreaseBackwardButton.read() == HIGH) {
    stopMotor();
    Serial.println("Motor Stopped after Backward Release");
  }
}

// ✅ Function to Execute Motor Movement
void executeMotorMovement() {
  encoderCount = 0;
  digitalWrite(IN1, LOW);  // Forward direction
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, motorSpeed);
  motorRunning = true;

  while (encoderCount < targetSteps) {
    Serial.print("Encoder Count: ");
    Serial.println(encoderCount);
    delay(10);
  }

  stopMotor();
  Serial.println("Motor Stopped after Target Rotations");
}

// ✅ Function to Move Motor Backward While Button is Held
void moveBackwardsContinuous() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, motorSpeed);
  motorRunning = true;
}

// ✅ Function to Stop the Motor
void stopMotor() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  motorRunning = false;
}

// ✅ Encoder Interrupt to Track Steps
void readEncoder() {
  static int lastState = LOW;
  int stateC1 = digitalRead(ENCODER_C1);
  int stateC2 = digitalRead(ENCODER_C2);

  if (stateC1 != lastState) {
    encoderCount += (stateC1 == stateC2) ? 1 : -1;
  }
  lastState = stateC1;
}
