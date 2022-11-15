const int storageClockLatchPin = 11;
const int shiftRegisterClockPin = 10;
const int dataStoragePin = 12;

const int segmentD1Pin = 7;
const int segmentD2Pin = 6;
const int segmentD3Pin = 5;
const int segmentD4Pin = 4;

const byte segmentCount = 8;    // per display

const int displayCount = 4;
const int encodingsNumber = 16;

int displayPins[] = {
  //    0            1              2             3
  segmentD1Pin, segmentD2Pin, segmentD3Pin, segmentD4Pin
};

int switchState;
int lastSwitchState = LOW;
const int switchPressThreshold = 1000;
const int debounceDuration = 60;

volatile unsigned long switchPressedTime;
volatile unsigned long switchReleasedTime;
volatile unsigned long lastInterruptTime = 0;
volatile unsigned long interruptTime;

int backToState1 = 0;
int systemState = 1;

int currentDisplay = 0;

const int pointBlinkDuration = 500;

// each hex number is encoded in binary
int byteEncodings[encodingsNumber] = {
//A B C D E F G DP 
  B11111100, // 0 
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
  B11101110, // A
  B00111110, // b
  B10011100, // C
  B01111010, // d
  B10011110, // E
  B10001110  // F
};

int displayDigits[displayCount] = {
  //  0          1          2          3
  B11111100, B11111100, B11111100, B11111100
};

int digits[displayCount] = {0};

const int xAxisPin = A0;
const int yAxisPin = A1;
const int switchOutputPin = 2;

int xAxisValue = 0;
int yAxisValue = 0;

const int minThreshold = 400;
const int maxThreshold = 600;
bool joyMoved = false;

void setup() {
  pinMode(dataStoragePin, OUTPUT);
  pinMode(shiftRegisterClockPin, OUTPUT);
  pinMode(storageClockLatchPin, OUTPUT);

  for (int i = 0; i < displayCount; i++) {
    pinMode(displayPins[i], OUTPUT);
    digitalWrite(displayPins[i], LOW);
  }

  pinMode(switchOutputPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(switchOutputPin), switchPress, CHANGE);

  Serial.begin(9600);
}

void loop() {
  display();

  if (systemState == 1) {
    firstState();
  }
  else {
    secondState();
  }
}

// The switchPress() function controlls the state in with the prototype is in.
// The default state is 1. In case of a short button press, the state will be changed to 2.
// For a long button press, the displays will be reset and the current position will be changed to the first display
// (this is only posible from state 1).
void switchPress() {
  if (systemState == 1) {
    Serial.println("Switch was pressed");
    interruptTime = millis();

    if (interruptTime - lastInterruptTime > debounceDuration) {
      if (digitalRead(switchOutputPin) == LOW) {
        switchPressedTime = millis();
        backToState1 = 0;
      }
      else {
        if (backToState1 == 0) {
          switchReleasedTime = millis();

          Serial.print("Press time: ");
          Serial.println(switchPressedTime);
          Serial.print("Release time: ");
          Serial.println(switchReleasedTime);

          if(switchReleasedTime - switchPressedTime < switchPressThreshold) {
            Serial.println("Changed to state 2");
            systemState = 2;
          }
          else {
              Serial.println("The display has been RESET");
              for (int i = 0; i < displayCount; i++) {
                displayDigits[i] = B11111100;
                digits[i] = 0;
              }
              systemState = 1;
              currentDisplay = 0;
          }
        }
      }
    }

    lastInterruptTime = interruptTime;
  }
}

// The display() function shows the digit of each individual display, one at the time.
// Using the process of multiplexing, it creates the illusion that all displays are
// lit at the same time.
void display() {
  for (int i = 0; i < displayCount; i++) {
    delay(2);
    writeOnShiftReg(displayDigits[i]);
    activateDisplay(i);
  }
}

// The firstState() function handles the joystick movement on the X axis, which changes displays.
// The decimal point is blinking for the current display.
void firstState() {
  static unsigned long lastJoyMovedTime = 0;
  static unsigned long joyMoveDebounceTime = 350;

  int oldDisplay = currentDisplay;
  int temporaryDisplay = currentDisplay;

  int displayChange = 0;

  blink(pointBlinkDuration);

  unsigned long joyMovedTime = millis();
  xAxisValue = analogRead(xAxisPin);

  if (xAxisValue > maxThreshold && joyMoved == false) {
    joyMoved = true;
    temporaryDisplay++;
    if (temporaryDisplay > 3) {
      temporaryDisplay = 0;
    }
    displayChange = 1;
  }

  if (xAxisValue < minThreshold && joyMoved == false) {
    joyMoved = true;
    temporaryDisplay--;
    if (temporaryDisplay < 0) {
      temporaryDisplay = 3;
    }
    displayChange = 1;
  }

  if (xAxisValue > minThreshold && xAxisValue < maxThreshold) {
      joyMoved = false;
  }

  if (displayChange == 1 && joyMovedTime - lastJoyMovedTime > joyMoveDebounceTime) {
    
    displayDigits[oldDisplay] = byteEncodings[digits[oldDisplay]];
    currentDisplay = temporaryDisplay;

    lastJoyMovedTime = joyMovedTime;
  }
}

// The secondState() function handles the digit change of an individual display.
// The change can be achieved by moving the joystick on the Y axis.
// The decimal point remains lit during this state.
// With a button press, the state changes to 1 again.
void secondState() {
  yAxisValue = analogRead(yAxisPin);

  if (yAxisValue > maxThreshold && joyMoved == false) {
    joyMoved = true;
    digits[currentDisplay]++;
    if (digits[currentDisplay] > 15) {
      digits[currentDisplay] = 0;
    }
  }

  if (yAxisValue < minThreshold && joyMoved == false) {
    joyMoved = true;
    digits[currentDisplay]--;
    if (digits[currentDisplay] < 0) {
      digits[currentDisplay] = 15;
    }
  }

  if (yAxisValue > minThreshold && yAxisValue < maxThreshold) {
      joyMoved = false;
  }

  displayDigits[currentDisplay] = byteEncodings[digits[currentDisplay]] ^ 1;

  switchState = digitalRead(switchOutputPin);

  if (switchState != lastSwitchState) {
    if (switchState == LOW) {
      Serial.println("Changed state to 1");
      systemState = 1;
      backToState1 = 1;

      displayDigits[currentDisplay] = displayDigits[currentDisplay] ^ 1;
    }
  }

  lastSwitchState = switchState;
};

// Simple function for decimal point blink
void blink (const int LedBlinkDuration) {
  static unsigned long lastLedBlinkReading = 0;
  unsigned long ledBlinkReading = millis();

  if (ledBlinkReading - lastLedBlinkReading >= LedBlinkDuration) {
    lastLedBlinkReading = ledBlinkReading;
    displayDigits[currentDisplay] = displayDigits[currentDisplay] ^ 1;
  }
}

void writeOnShiftReg(byte encoding) {
  digitalWrite(storageClockLatchPin, LOW);
  shiftOut(dataStoragePin, shiftRegisterClockPin, MSBFIRST, encoding);
  digitalWrite(storageClockLatchPin, HIGH);
}

void activateDisplay(byte displayNumber) {
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayPins[i], HIGH);
  }
  digitalWrite(displayPins[displayNumber], LOW);
}
