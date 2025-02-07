int relayPin = 8;       // Pin controlling the relay
int switchPin = 7;      // Pin connected to the switch
int switchState = 0;    // Variable to store the switch state
 
void setup() {
  pinMode(relayPin, OUTPUT);        // Set relay pin as output
  pinMode(switchPin, INPUT_PULLUP); // Set switch pin as input with internal pull-up resistor
  digitalWrite(relayPin, LOW);      // Initially, the relay is off
}
 
void loop() {
  switchState = digitalRead(switchPin); // Read the state of the switch
  // Check if the switch is pressed (LOW because of internal pull-up)
  if (switchState == LOW) {
    digitalWrite(relayPin, HIGH);  // Turn on the relay (power the load)
    delay(10000);                  // Wait for 10 seconds
    digitalWrite(relayPin, LOW);   // Turn off the relay (cut power to load)
  }
}


int relayPin = 8;       // Pin controlling the relay
int switchPin = 7;      // Pin connected to the switch
int switchState = 0;    // Variable to store the switch state
const int debounceDelay = 50; // Debounce time in milliseconds

#define RELAY_ACTIVE_LOW  // Comment this line if your relay is active HIGH

void setup() {
  pinMode(relayPin, OUTPUT);        // Set relay pin as output
  pinMode(switchPin, INPUT_PULLUP); // Set switch pin as input with internal pull-up resistor

  #ifdef RELAY_ACTIVE_LOW
    digitalWrite(relayPin, HIGH);   // Ensure relay starts OFF if active LOW
  #else
    digitalWrite(relayPin, LOW);    // Ensure relay starts OFF if active HIGH
  #endif
}

void loop() {
  switchState = digitalRead(switchPin); // Read the state of the switch
  
  if (switchState == LOW) { // Button pressed (LOW due to pull-up)
    delay(debounceDelay); // Debounce delay
    if (digitalRead(switchPin) == LOW) { // Confirm press after debounce

      #ifdef RELAY_ACTIVE_LOW
        digitalWrite(relayPin, LOW);  // Activate relay (turn ON induction heater)
      #else
        digitalWrite(relayPin, HIGH); // Activate relay (turn ON induction heater)
      #endif

      delay(10000); // Keep the relay ON for 10 seconds

      #ifdef RELAY_ACTIVE_LOW
        digitalWrite(relayPin, HIGH); // Deactivate relay (turn OFF induction heater)
      #else
        digitalWrite(relayPin, LOW);  // Deactivate relay (turn OFF induction heater)
      #endif
    }
  }
}

