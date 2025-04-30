#include <Keyboard.h>
#include <Mouse.h>
#include "Wire.h"
#include <Arduino_LSM6DS3.h>

// Digital pin definitions
const int escapeButtonPin = 3;
const int switchInputsButtonPin = 13;
const int rightMouseButtonPin = 10; 
const int crouchButtonPin = 6;
const int selPin = 2;
const int spaceButtonPin = 7;
const int leftMouseButtonPin = 9;
const int reloadButtonPin = 4;
const int swapWeaponButtonPin = 5;
const int grenadeButtonPin = 8;
const int interactButtonPin = 11;

// Analog pin definitions
const int vertPin = A0; 
const int horzPin = A1; 
const int potPin = A2;

// Joystick variables
int vertZero, horzZero;
int vertValue, horzValue;
int sensitivity, potValue;
int invertMouse = 1;
const int joystickDeadzone = 5;

// Accelerometer variables
float x, y, z;

// Button states
int spaceButtonState = 0, lastSpaceButtonState = 0;
int leftMouseButtonState = 0, lastLeftMouseButtonState = 0;
int rightMouseButtonState = 0, lastRightMouseButtonState = 0;
int crouchButtonState = 0, lastCrouchButtonState = 0;
int reloadButtonState = 0, lastReloadButtonState = 0;
int escapeButtonState = 0, lastEscapeButtonState = 0;
int grenadeButtonState = 0, lastGrenadeButtonState = 0;
int interactButtonState = 0, lastInteractButtonState = 0;
int swapWeaponButtonState = 0, lastSwapWeaponButtonState = 0;
int switchInputsButtonState = 0, lastSwitchInputsButtonState = 0;

int joystickButtonState;

// Key definitions
#define KEY_SPACE 0x20
#define KEY_C 0x63
#define KEY_W 0x77
#define KEY_S 0x73
#define KEY_A 0x61
#define KEY_D 0x64
#define KEY_G 0x67
#define KEY_Q 0x71
#define KEY_R 0x72
#define KEY_ESC 0xB1
#define KEY_TAB 0xB3
#define KEY_F 0x66
#define KEY_V 0x76
#define KEY_B 0x62
#define KEY_4 0x34
#define KEY_5 0x35
#define KEY_6 0x36 


// Mapping state
bool alternateMapping = false;

void setup() {
  // Set all button inputs with pull-up resistors
  pinMode(escapeButtonPin, INPUT_PULLUP);
  pinMode(switchInputsButtonPin, INPUT_PULLUP);
  pinMode(rightMouseButtonPin, INPUT_PULLUP);
  pinMode(crouchButtonPin, INPUT_PULLUP);
  pinMode(spaceButtonPin, INPUT_PULLUP);
  pinMode(leftMouseButtonPin, INPUT_PULLUP);
  pinMode(reloadButtonPin, INPUT_PULLUP);
  pinMode(swapWeaponButtonPin, INPUT_PULLUP);
  pinMode(grenadeButtonPin, INPUT_PULLUP);
  pinMode(interactButtonPin, INPUT_PULLUP);
  pinMode(selPin, INPUT_PULLUP);

  pinMode(horzPin, INPUT);
  pinMode(vertPin, INPUT);
  pinMode(potPin, INPUT);

  Wire.begin();
  Mouse.begin();
  Keyboard.begin();
  Serial.begin(1000);

  vertZero = analogRead(vertPin);
  horzZero = analogRead(horzPin);

  while (!Serial);
  Serial.println("Started");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println("Hz");
}


void handleButton(int buttonPin, int &buttonState, int &lastButtonState,
                  void (*pressActionPrimary)(), void (*releaseActionPrimary)(),
                  void (*pressActionAlternate)(), void (*releaseActionAlternate)()) {
  buttonState = digitalRead(buttonPin);

  if (alternateMapping) {
    if (buttonState == LOW && lastButtonState == HIGH) {
      pressActionAlternate();
    } else if (buttonState == HIGH && lastButtonState == LOW) {
      releaseActionAlternate();
    }
  } else {
    if (buttonState == LOW && lastButtonState == HIGH) {
      pressActionPrimary();
    } else if (buttonState == HIGH && lastButtonState == LOW) {
      releaseActionPrimary();
    }
  }

  lastButtonState = buttonState;
}


void handleJoystickAndMouse() {
  potValue = analogRead(potPin);
  sensitivity = map(potValue, 0, 1023, 25, 400);

  horzValue = analogRead(vertPin) - vertZero;  // read vertical offset
  vertValue = analogRead(horzPin) - horzZero;  // read horizontal offset

  if (vertValue != 0)
    Mouse.move(0, (invertMouse * -(vertValue / sensitivity)), 0); // move mouse on y axis
  if (horzValue != 0)
    Mouse.move((invertMouse * (horzValue / sensitivity)), 0, 0); // move mouse on x axis

  // Joystick button press (clicks SHIFT while held)
  joystickButtonState = digitalRead(selPin);
  if (joystickButtonState == LOW) {
    Keyboard.press(KEY_LEFT_SHIFT);
  } else {
    Keyboard.release(KEY_LEFT_SHIFT);
  }
}



void handleAccelerometer() {

    if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    // Debug print to see the values
    Serial.print("X: "); Serial.print(x);
    Serial.print(" Y: "); Serial.print(y);
    Serial.print(" Z: "); Serial.println(z);
  
  // Forward and backward movement
  if (x > -0.15) {
    Keyboard.press(KEY_W);
  } else if (x < -0.80) {
    Keyboard.press(KEY_S);
  }

  // Side to Side movement
  if (z > 0.58) {
    Keyboard.press(KEY_D);
  } else if (z < -0.42) {
    Keyboard.press(KEY_A);
  } else {
    Keyboard.release(KEY_W);
    Keyboard.release(KEY_S);
    Keyboard.release(KEY_A);
    Keyboard.release(KEY_D);
  }
}
}

void loop() {
  if (alternateMapping) {
  Serial.println("ALTERNATE mapping active");
}
  switchInputsButtonState = digitalRead(switchInputsButtonPin);
  alternateMapping = (switchInputsButtonState == LOW);

  // Reload Button
  handleButton(reloadButtonPin, reloadButtonState, lastReloadButtonState,
               []() { Keyboard.press(KEY_R); },  // Primary action
               []() { Keyboard.release(KEY_R); },
               []() { Keyboard.press(KEY_B); },  // Alternate action
               []() { Keyboard.release(KEY_B); });

  // Jump Button
  handleButton(spaceButtonPin, spaceButtonState, lastSpaceButtonState,
               []() { Keyboard.press(KEY_SPACE); },  // Primary action
               []() { Keyboard.release(KEY_SPACE); },
               []() { Keyboard.press(KEY_6); },  // Alternate action
               []() { Keyboard.release(KEY_6); });

  // Crouch Button
  handleButton(crouchButtonPin, crouchButtonState, lastCrouchButtonState,
               []() { Keyboard.press(KEY_C); },  // Primary action
               []() { Keyboard.release(KEY_C); },
               []() { Keyboard.press(KEY_5); },  // Alternate action
               []() { Keyboard.release(KEY_5); });
  
  // Swap Weapons Button
  handleButton(swapWeaponButtonPin, swapWeaponButtonState, lastSwapWeaponButtonState,
             []() { Mouse.move(0, 0, 1); },    // Primary action
             []() {},
             []() { Keyboard.press(KEY_4); }, // Alternate action
             []() { Keyboard.release(KEY_4); }); 

  // Menu Button
  handleButton(escapeButtonPin, escapeButtonState, lastEscapeButtonState,
               []() { Keyboard.press(KEY_TAB); },  // Primary action
               []() { Keyboard.release(KEY_TAB); },
               []() { Keyboard.press(KEY_ESC); },  // Alternate action
               []() { Keyboard.release(KEY_ESC); });

  // Bumper Button
  handleButton(grenadeButtonPin, grenadeButtonState, lastGrenadeButtonState,
               []() { Keyboard.press(KEY_G); },  // Primary action
               []() { Keyboard.release(KEY_G); },
               []() { Keyboard.press(KEY_Q); },  // Alternate action
               []() { Keyboard.release(KEY_Q); });

  // Ring Finger Button
  handleButton(interactButtonPin, interactButtonState, lastInteractButtonState,
               []() { Keyboard.press(KEY_F); },  // Primary action
               []() { Keyboard.release(KEY_F); },
               []() { Keyboard.press(KEY_V); },  // Alternate action
               []() { Keyboard.release(KEY_V); });

  // Middle Finger Button
  handleButton(rightMouseButtonPin, rightMouseButtonState, lastRightMouseButtonState,
               []() { Mouse.press(MOUSE_RIGHT); },  // Primary action
               []() { Mouse.release(MOUSE_RIGHT); },
               []() { Mouse.press(MOUSE_RIGHT);},                             // No alternate action
               []() { Mouse.release(MOUSE_RIGHT);});                            

  // Trigger Buttonb
  handleButton(leftMouseButtonPin, leftMouseButtonState, lastLeftMouseButtonState,
               []() { Mouse.press(); },            // Primary action
               []() { Mouse.release(); },
               []() { Mouse.press();},             // No alternate action
               []() { Mouse.release();}); 
  

  // Handle joystick and accelerometer
  handleJoystickAndMouse();
  handleAccelerometer();
}