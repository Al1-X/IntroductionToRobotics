const int up = 0;
const int down = 1;
const int left = 2;
const int right = 3;

const int pinA = 4;
const int pinB = 5;
const int pinC = 6;
const int pinD = 7;
const int pinE = 8;
const int pinF = 9;
const int pinG = 10;
const int pinDP = 11;

const int pinXAxis = A0;
const int pinYAxis = A1;
const int pinSwitchOutput = 2;

const int numberOfSegments = 8;
const int segmentBlinkDuration = 500;

unsigned long lastLedBlinkReading = 0;

const int minThreshold = 400;
const int maxThreshold = 600;
bool joyMoved = false;

int xAxisValue = 0;
int yAxisValue = 0;

int switchState;
int lastSwitchState = LOW;
const int switchPressThreshold = 1500;
const int debounceDuration = 60;

volatile unsigned long switchPressedTime;
volatile unsigned long switchReleasedTime;
volatile unsigned long lastInterruptTime = 0;
volatile unsigned long interruptTime;

int backToState1 = 0;
int systemState = 1;

int currentPositionPin = 7;
byte currentPositionState = LOW;

int segments[numberOfSegments] = {
// 0     1      2     3     4     5     6     7
	pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

byte segmentsStates[numberOfSegments] = {
  LOW
};

int segmentMovementMatrix[numberOfSegments][4] = {
// UP  DOWN  LEFT  RIGHT
  {-1,   6,    5,    1 },   // a 0
  { 0,   6,    5,   -1 },   // b 1
  { 6,   3,    4,    7 },   // c 2
  { 6,  -1,    4,    2 },   // d 3
  { 6,   3,   -1,    2 },   // e 4
  { 0,   6,   -1,    1 },   // f 5
  { 0,   3,   -1,   -1 },   // g 6
  {-1,  -1,    2,   -1 },   // dp 7
};

// ----------------------------------------------------------------------------
void setup() {
  pinMode(pinSwitchOutput, INPUT_PULLUP);
  for (int i = 0; i < numberOfSegments; i++) {
    pinMode(segments[i], OUTPUT);
  }

  attachInterrupt(digitalPinToInterrupt(pinSwitchOutput), switchPress, CHANGE);

  Serial.begin(9600);
}


void loop() {
  if (systemState == 1) {
    firstState();
  }
  else {
    secondState();
  }
}


// The function for the first state keeps track of the segment movement, by registering the analog output 
// given by the switch and mapping it to the corresponding axis.
void firstState() {
  blink(segments[currentPositionPin], segmentBlinkDuration);

  xAxisValue = analogRead(pinXAxis);

  // X axis maps to horizontal movement
  // RIGHT
  if (xAxisValue > maxThreshold && joyMoved == false && 
      segmentMovementMatrix[currentPositionPin][3] != -1) {
    joyMoved = true;
    digitalWrite(segments[currentPositionPin], segmentsStates[currentPositionPin]);
    currentPositionPin = segmentMovementMatrix[currentPositionPin][right];

    Serial.print("RIGHT -> Moved to pin ");
    Serial.println(currentPositionPin);
    currentPositionState = segmentsStates[currentPositionPin];
  }

  // LEFT
  if (xAxisValue < minThreshold && joyMoved == false &&
      segmentMovementMatrix[currentPositionPin][2] != -1) {
    joyMoved = true;
    digitalWrite(segments[currentPositionPin], segmentsStates[currentPositionPin]);
    currentPositionPin = segmentMovementMatrix[currentPositionPin][left];

    Serial.print("LEFT -> Moved to pin ");
    Serial.println(currentPositionPin);
    currentPositionState = segmentsStates[currentPositionPin];
  }


  yAxisValue = analogRead(pinYAxis);

  // Y axis maps to vertical movement
  // UP
  if (yAxisValue > maxThreshold && joyMoved == false &&
      segmentMovementMatrix[currentPositionPin][0] != -1) {
    joyMoved = true;
    digitalWrite(segments[currentPositionPin], segmentsStates[currentPositionPin]);
    currentPositionPin = segmentMovementMatrix[currentPositionPin][up];

    Serial.print("UP -> Moved to pin ");
    Serial.println(currentPositionPin);
    currentPositionState = segmentsStates[currentPositionPin];
  }
  
  // DOWN
  if (yAxisValue < minThreshold && joyMoved == false &&
      segmentMovementMatrix[currentPositionPin][1] != -1) {
    joyMoved = true;
    digitalWrite(segments[currentPositionPin], segmentsStates[currentPositionPin]);
    currentPositionPin = segmentMovementMatrix[currentPositionPin][down];

    Serial.print("DOWN -> Moved to pin ");
    Serial.println(currentPositionPin);
    currentPositionState = segmentsStates[currentPositionPin];
  }

  if (yAxisValue >= minThreshold && yAxisValue <= maxThreshold &&
      xAxisValue >= minThreshold && xAxisValue <= maxThreshold) {
    joyMoved = false;
  }
}


// The function for the second state handles the changing of the "true" state of the current
// segment, using movement on the X axis. In case the button is pressed while in state 2,
// the state changes to 1.
void secondState() {
  digitalWrite(segments[currentPositionPin], segmentsStates[currentPositionPin]);

  xAxisValue = analogRead(pinXAxis);

  if ((xAxisValue < minThreshold || xAxisValue > maxThreshold) && joyMoved == false) {
    joyMoved = true;
    segmentsStates[currentPositionPin] = !segmentsStates[currentPositionPin];
  }

  if (xAxisValue > minThreshold && xAxisValue < maxThreshold) {
    joyMoved = false;
  }

  switchState = digitalRead(pinSwitchOutput);

  if (switchState != lastSwitchState) {
    if (switchState == LOW) {
      Serial.println("Changed state to 1");
      systemState = 1;
      backToState1 = 1;
    }
  }
  lastSwitchState = switchState;
}


// The IRS() manages state change between 1 and 2, taking in account the duration for which the switch is pressed.
// For a short press, the state will change to 2.
// For a long press, the display will reset and the state will remain 1.
// The long press is only available in state 1.
void switchPress() {
  if (systemState == 1) {
    interruptTime = millis();

    if (interruptTime - lastInterruptTime > debounceDuration) {
      if (digitalRead(pinSwitchOutput) == LOW) {
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
              for (int i = 0; i < numberOfSegments; i++) {
                segmentsStates[i] = LOW;
                digitalWrite(segments[i], segmentsStates[i]);
              }
              currentPositionPin = 7;
              systemState = 1;
          }
        }
      }
    }
    lastInterruptTime = interruptTime;
  }
}


// Simple function for segment blink
void blink (int ledPin, const int LedBlinkDuration) {
  unsigned long ledBlinkReading = millis();

  if (ledBlinkReading - lastLedBlinkReading >= LedBlinkDuration) {
    lastLedBlinkReading = ledBlinkReading;
    currentPositionState = !currentPositionState;
    digitalWrite(ledPin, currentPositionState);
  }
}