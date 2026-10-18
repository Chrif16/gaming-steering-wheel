#include <Arduino.h>
#include <BleGamepad.h>

BleGamepad bleGamepad("Steering wheel", "chrif", 100);

// Define which physical ESP32 GPIO pins are wired to your buttons
const int BUTTON_A_PIN = 21; // Wired to GPIO 21
const int BUTTON_B_PIN = 22; // Wired to GPIO 22
const int CLK_PIN = 23;  // Wired to GPIO 23
const int DT_PIN = 7;  // Wired to GPIO 7

int lastButtonAState = HIGH;
int lastButtonBState = HIGH;
volatile int encoderPosition = 0;
volatile int lastClkState;
volatile int encoderDelta = 0;       
unsigned long cwTimer = 0;            
unsigned long ccwTimer = 0;           
bool cwActive = false;
bool ccwActive = false;
int32_t wheelPosition = 0;   
const int32_t maxTurn = 500; 

void IRAM_ATTR handleRotation() {
  int clkState = digitalRead(CLK_PIN);
  
  
  if (clkState != lastClkState) {
    if (digitalRead(DT_PIN) != clkState) {
      encoderPosition++;
    } else {
      encoderPosition--;
    }
  }
  lastClkState = clkState;
}

void setup() {
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  lastClkState = digitalRead(CLK_PIN);
  attachInterrupt(digitalPinToInterrupt(CLK_PIN), handleRotation, CHANGE);

  bleGamepad.begin();
}

void loop() {
  if (bleGamepad.isConnected()) {
    
    // --- MAP BUTTON A ---
    int currentButtonAState = digitalRead(BUTTON_A_PIN);
    if (currentButtonAState != lastButtonAState) {
      if (currentButtonAState == LOW) {
        bleGamepad.press(BUTTON_1);
      } else {
        bleGamepad.release(BUTTON_1);
      }
      lastButtonAState = currentButtonAState;
    }

    // --- MAP BUTTON B ---
    int currentButtonBState = digitalRead(BUTTON_B_PIN);
    if (currentButtonBState != lastButtonBState) {
      if (currentButtonBState == LOW) {
        bleGamepad.press(BUTTON_2);
      } else {
        bleGamepad.release(BUTTON_2);
      }
      lastButtonBState = currentButtonBState;
    }

    noInterrupts();
    int movement = encoderDelta;
    encoderDelta = 0; 
    interrupts();
    if (movement != 0) {
            wheelPosition += movement;

            wheelPosition = constrain(wheelPosition, -maxTurn, maxTurn);

            int16_t gamepadX = map(wheelPosition, -maxTurn, maxTurn, -32767, 32767);

            bleGamepad.setX(gamepadX);
            bleGamepad.sendReport();
    }


    delay(10); 
  }
}
