const int buttonPin = 2;
const int buzzerPin = 3;

const int peopleGreenLedPin = 5;
const int peopleRedLedPin = 6;

const int carsGreenLedPin = 8;
const int carsYellowLedPin = 10;
const int carsRedLedPin = 12;

const int semaphoreState1Duration = 8000;
const int semaphoreState2Duration = 3000;
const int semaphoreState3Duration = 8000;
const int semaphoreState4Duration = 4000;

volatile unsigned long semaphoreState1StartTime = 0;
volatile unsigned long semaphoreState2StartTime = 0;
volatile unsigned long semaphoreState3StartTime = 0;
volatile unsigned long semaphoreState4StartTime = 0;

const int buzzerTone = 3000;
const int buzzState3Duration = 500;
const int buzzState4Duration = 200;

unsigned long lastLedBlinkReading = 0;
unsigned long ledBlinkReading = 0;
unsigned long lastDebounceTime = 0;
const long debounceInterval = 60;

const unsigned long peopleGreenLedBlinkDuration = 200;

byte buttonState = HIGH;
byte currentButtonState = LOW;
byte lastButtonState = LOW;

byte peopleGreenLedState = LOW;
byte peopleRedLedState = LOW;

byte carsGreenLedState = LOW;
byte carsYellowLedState = LOW;
byte carsRedLedState = LOW;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(peopleGreenLedPin, OUTPUT);
  pinMode(peopleRedLedPin, OUTPUT);

  pinMode(carsGreenLedPin, OUTPUT);
  pinMode(carsYellowLedPin, OUTPUT);
  pinMode(carsRedLedPin, OUTPUT);

  //attachInterrupt(digitalPinToInterrupt(buttonPin), activateSemaphore, RISING);

  Serial.begin(9600);
}

void loop() {
  originalState();
  currentButtonState = digitalRead(buttonPin);

  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceInterval) {
    if (currentButtonState != buttonState) {
      buttonState = currentButtonState;
      if (buttonState == LOW) {
        activateSemaphore();
      }
    }
  }

  lastButtonState = currentButtonState;
}

void activateSemaphore () {
  semaphoreState1StartTime = millis();

  Serial.println("state 1");

  // SEMAPHORE STATE 1
  while (millis() - semaphoreState1StartTime < semaphoreState1Duration);

  Serial.println("state 2");
  // SEMAPHORE STATE 2
  semaphoreState2StartTime = millis();

  carsGreenLedState = !carsGreenLedState;// hl
  digitalWrite(carsGreenLedPin, carsGreenLedState);
  carsYellowLedState = !carsYellowLedState;// lh
  digitalWrite(carsYellowLedPin, carsYellowLedState);

  while (millis() - semaphoreState2StartTime < semaphoreState2Duration);

  // SEMAPHORE STATE 3
  Serial.println("state 3");

  semaphoreState3StartTime = millis();


  carsYellowLedState = !carsYellowLedState; //hl
  digitalWrite(carsYellowLedPin, LOW);

  peopleRedLedState = !peopleRedLedState;   //hl
  digitalWrite(peopleRedLedPin, peopleRedLedState);
  peopleGreenLedState = !peopleGreenLedState; //lh
  digitalWrite(peopleGreenLedPin, peopleGreenLedState);

  while (millis() - semaphoreState3StartTime < semaphoreState3Duration) {
    buzz(buzzState3Duration);
  }

  // SEMAPHORE STATE 4
  Serial.println("state 4");

  semaphoreState4StartTime = millis();

  while (millis() - semaphoreState4StartTime < semaphoreState4Duration) {
    blink(peopleGreenLedState, peopleGreenLedPin, peopleGreenLedBlinkDuration);
    buzz(buzzState4Duration);
  }

  peopleGreenLedState = LOW;
  digitalWrite(peopleGreenLedPin, peopleGreenLedState);
}

void originalState() {
  carsGreenLedState = HIGH;
  digitalWrite(carsGreenLedPin, carsGreenLedState);
  peopleRedLedState = HIGH;
  digitalWrite(peopleRedLedPin, peopleRedLedState);
}

void blink (byte& ledState, const int ledPin, const int LedBlinkDuration) {
  ledBlinkReading = millis();

  if (ledBlinkReading - lastLedBlinkReading >= LedBlinkDuration) {
    lastLedBlinkReading = ledBlinkReading;

    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}

void buzz (const int buzzDuration) {
    tone(buzzerPin, buzzerTone, buzzDuration);
    delay(1000);
    noTone(buzzerPin);
}
