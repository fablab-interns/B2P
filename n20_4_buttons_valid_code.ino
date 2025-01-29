#include <Bounce2.h>

// Motor control pins
#define IN1 7
#define IN2 6
#define ENA 9

// Encoder pins
#define ENCODER_C1 2
#define ENCODER_C2 3

// Button pins
#define BUTTON_DECREASE_BACKWARD_PIN 4  // New button to decrease the motor position
#define BUTTON_DECREMENT_PIN 10
#define BUTTON_INCREMENT_PIN 11
#define BUTTON_TOGGLE_PIN 12  // The toggle button

// Variables
volatile long encoderCount = 0;  // Tracks the encoder count
const int CPR = 28;             // Counts per revolution (7 PPR * 4 quadrature steps)
const int incrementSteps = 1 * CPR;  // Encoder counts for 4 shaft rotations
const int motorSpeed = 150;     // Motor speed (0 to 255)
bool motorRunning = false;      // Tracks if the motor is running
long targetSteps = 0;           // Total encoder counts to reach (updated with each increment/decrement)

// Button instances
Bounce decreaseBackwardButton = Bounce();  // New button instance for decrease backward
Bounce incrementButton = Bounce();
Bounce decrementButton = Bounce();
Bounce toggleButton = Bounce();

void setup() {
  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  // Encoder pins
  pinMode(ENCODER_C1, INPUT);
  pinMode(ENCODER_C2, INPUT);

  // Button pins
  pinMode(BUTTON_DECREASE_BACKWARD_PIN, INPUT_PULLUP);  // Setup for the new backward button
  pinMode(BUTTON_INCREMENT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DECREMENT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_TOGGLE_PIN, INPUT_PULLUP);

  // Initialize the Bounce objects
  decreaseBackwardButton.attach(BUTTON_DECREASE_BACKWARD_PIN);  // Attach new backward button
  decreaseBackwardButton.interval(25); // Debounce interval for decrease backward button

  incrementButton.attach(BUTTON_INCREMENT_PIN);
  incrementButton.interval(25); // Debounce interval for increment button

  decrementButton.attach(BUTTON_DECREMENT_PIN);
  decrementButton.interval(25); // Debounce interval for decrement button

  toggleButton.attach(BUTTON_TOGGLE_PIN);
  toggleButton.interval(25); // Debounce interval for toggle button

  // Initially ensure motor is stopped
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  // Attach encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_C1), readEncoder, CHANGE);

  // Start serial communication
  Serial.begin(115200);
}

void loop() {
  // Update button states
  decreaseBackwardButton.update();  // Update the new backward button state
  incrementButton.update();
  decrementButton.update();
  toggleButton.update();

  // Increment button logic: Add 4 shaft rotations to the target
  if (incrementButton.fell() && !motorRunning) {
    Serial.println("Increment Button Pressed");

    // Add 4 rotations to the target
    targetSteps += incrementSteps;
    Serial.println("steps:");
    Serial.println(targetSteps);

    executeMotorMovement();
  }
  // Decrement button logic: Subtract 4 shaft rotations from the target or move backward
  if (decrementButton.fell() && !motorRunning) {
    Serial.println("Decrement Button Pressed");

    // Subtract 4 rotations, ensuring target doesn't go below 0
    if (targetSteps >= incrementSteps) {
      targetSteps -= incrementSteps;
      if (targetSteps > 0) {
        executeMotorMovement();
      } else {
        Serial.println("No Rotations Set (Target = 0)");
      }
    } else {
      Serial.println("Cannot Decrease Below Zero Rotations");
    }
  }

  // Toggle button logic: Start/stop the motor
  if (toggleButton.fell()) {
    if (motorRunning) {
      stopMotor();
      Serial.println("Motor Stopped via Toggle");
    } else if (targetSteps > 0) {
      executeMotorMovement();
    } else {
      Serial.println("No Rotations Set (Target = 0)");
    }
  }

  // Decrease backward button logic: Move motor backward while the button is pressed
  if (decreaseBackwardButton.read() == LOW) {  // If the button is pressed
    if (!motorRunning) {  // Only start the motor if it's not already running
      Serial.println("Decrease Backward Button Pressed - Moving Motor Backward");
      moveBackwardsContinuous();  // Start moving backward
    }
  } else {  // If the button is released
    if (motorRunning) {
      stopMotor();  // Stop the motor immediately after release
      Serial.println("Motor Stopped after Button Released");
    }
  }
}

// Function to execute motor movement to target steps
void executeMotorMovement() {
  encoderCount = 0;  // Reset encoder count
  digitalWrite(IN1, LOW);  // Forward direction
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, motorSpeed);  // Set motor speed
  motorRunning = true;

  // Wait until the target steps are reached
  while (encoderCount < targetSteps) {
    Serial.print("Encoder Count: ");
    Serial.println(encoderCount);
    delay(10);  // Small delay to allow encoder to update
  }

  // Stop the motor after completing the target
  stopMotor();
  Serial.println("Motor Stopped after Target Rotations");
}

// Function to move the motor backward continuously while the button is pressed
void moveBackwardsContinuous() {
  digitalWrite(IN1, HIGH);  // Reverse direction
  digitalWrite(IN2, LOW);
  analogWrite(ENA, motorSpeed);  // Set motor speed
  motorRunning = true;  // Indicate that the motor is running

  // Print encoder count while moving backward
  Serial.print("Reversing - Encoder Count: ");
  Serial.println(encoderCount);

  // Small delay to allow encoder to update
  delay(10);
}

// Function to stop the motor
void stopMotor() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  motorRunning = false;
}

// Encoder interrupt function to update encoder count
void readEncoder() {
  static int lastState = LOW;
  int stateC1 = digitalRead(ENCODER_C1);
  int stateC2 = digitalRead(ENCODER_C2);

  if (stateC1 != lastState) {
    if (stateC1 == stateC2) {
      encoderCount++;
    } else {
      encoderCount--;
    }
    Serial.print("Interrupt: Encoder Count = ");
    Serial.println(encoderCount);  // Debugging print
  }
  lastState = stateC1;
}


 
