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

// Constants
const int CPR = 28;                     // Encoder counts per revolution
const int STEPS_PER_ROTATION = 4 * CPR;  // Steps for 4 rotations
const int FIXED_STEPS = 380;             // Predefined forward steps
int motorSpeed = 150;                    // Motor speed (0-255)

// Variables
volatile long encoderCount = 0;
long targetSteps = 0;
bool motorRunning = false;
bool motorDirection = true;  // true = forward, false = backward

// Button instances
Bounce decrementButton = Bounce();
Bounce incrementButton = Bounce();
Bounce toggleButton = Bounce();
Bounce backwardButton = Bounce();

void setup() {
  Serial.begin(115200);

  // Motor setup
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  stopMotor();  // Ensure motor is stopped at start

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
  backwardButton.attach(BUTTON_DECREASE_BACKWARD_PIN);

  incrementButton.interval(50);
  decrementButton.interval(50);
  toggleButton.interval(50);
  backwardButton.interval(50);
}

void loop() {
  incrementButton.update();
  decrementButton.update();
  toggleButton.update();
  backwardButton.update();

  // Increment button: increase steps forward
  if (incrementButton.fell() && !motorRunning) {
    targetSteps += STEPS_PER_ROTATION;
    Serial.println("Increment Button Pressed");
    startMotor(true);
  }

  // Decrement button: decrease forward steps
  if (decrementButton.fell() && !motorRunning) {
    if (targetSteps >= STEPS_PER_ROTATION) {
      targetSteps -= STEPS_PER_ROTATION;
      Serial.println("Decrement Button Pressed");
      if (targetSteps > 0) startMotor(true);
    }
  }

  // Toggle button: run a fixed number of steps
  if (toggleButton.fell()) {
    if (motorRunning) {
      stopMotor();
      Serial.println("Motor Stopped via Toggle");
    } else {
      targetSteps = FIXED_STEPS;
      Serial.println("Toggle Button Pressed: Moving Forward");
      startMotor(true);
    }
  }

  // Backward button: move in reverse while pressed
  if (backwardButton.read() == LOW && !motorRunning) {
    Serial.println("Backward Button Pressed - Moving Backward");
    startMotor(false);
  } else if (motorRunning && backwardButton.read() == HIGH) {
    stopMotor();
    Serial.println("Motor Stopped after Backward Release");
  }
}

// Function to start motor movement
void startMotor(bool forward) {
  encoderCount = 0;
  motorDirection = forward;

  digitalWrite(IN1, forward ? LOW : HIGH);
  digitalWrite(IN2, forward ? HIGH : LOW);
  analogWrite(ENA, motorSpeed);
  motorRunning = true;

  while (encoderCount < targetSteps) {
    Serial.print("Encoder Count: ");
    Serial.println(encoderCount);
    delay(10);
  }

  stopMotor();
}

// Function to stop the motor
void stopMotor() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  motorRunning = false;
}

// Encoder interrupt function
void readEncoder() {
  static int lastState = LOW;
  int stateC1 = digitalRead(ENCODER_C1);
  int stateC2 = digitalRead(ENCODER_C2);

  if (stateC1 != lastState) {
    encoderCount += (stateC1 == stateC2) ? 1 : -1;
  }
  lastState = stateC1;
}
