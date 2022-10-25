#define MIN_ANALOG_INPUT 0
#define MAX_ANALOG_INPUT 1023
#define MIN_ANALOG_OUTPUT 0
#define MAX_ANALOG_OUTPUT 255

const int analogInPinRed = A0;
const int analogInPinGreen = A1;
const int analogInPinBlue = A2;

const int analogOutPinRed = 11;
const int analogOutPinGreen = 10;
const int analogOutPinBlue = 9;

int sensorValueRed = 0;
int sensorValueGreen = 0;
int sensorValueBlue = 0;

int outputValueRed = 0;
int outputValueGreen = 0;
int outputValueBlue = 0;

void setup () {
  pinMode(analogInPinRed, INPUT);
  pinMode(analogInPinGreen, INPUT);
  pinMode(analogInPinBlue, INPUT);

  pinMode(analogOutPinRed, OUTPUT);
  pinMode(analogOutPinGreen, OUTPUT);
  pinMode(analogOutPinBlue, OUTPUT);
}

/* the values read from the sensors (potentiometer in our case) are mapped in an adequate interval
that can be interpreted by the RGB led, then are passed to their corresponding color
the input values are between 0 and 1023, while the output values are between 0 and 255*/
void loop () {
  sensorValueRed = analogRead(analogInPinRed);
  sensorValueGreen = analogRead(analogInPinGreen);
  sensorValueBlue = analogRead(analogInPinBlue);

  outputValueRed = map(sensorValueRed, MIN_ANALOG_INPUT, MAX_ANALOG_INPUT, MIN_ANALOG_OUTPUT, MAX_ANALOG_OUTPUT);
  outputValueGreen = map(sensorValueGreen, MIN_ANALOG_INPUT, MAX_ANALOG_INPUT, MIN_ANALOG_OUTPUT, MAX_ANALOG_OUTPUT);
  outputValueBlue = map(sensorValueBlue, MIN_ANALOG_INPUT, MAX_ANALOG_INPUT, MIN_ANALOG_OUTPUT, MAX_ANALOG_OUTPUT);

  analogWrite(analogOutPinRed, outputValueRed);
  analogWrite(analogOutPinGreen, outputValueGreen);
  analogWrite(analogOutPinBlue, outputValueBlue);
}
