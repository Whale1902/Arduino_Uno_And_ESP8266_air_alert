#include <SoftwareSerial.h>

const int PIN_RED = 10;
const int PIN_GREEN = 11;
const int PIN_BLUE = 9;

const int BUZZER = 6;

#define rxPin 5
#define txPin 3

SoftwareSerial mySerial(rxPin, txPin);

int attentionFlashesTimes = 5;

void setup() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  mySerial.begin(115200);

  Serial.begin(9600);

  greenState();
  // cancel();
}

char alarmRef[3] = "11";
char cancelRef[3] = "22";
char duringAlarmRef[3] = "33";
char durinCancelRef[3] = "44";

const byte numChars = 100;
char receivedChars[numChars];  // an array to store the received data

bool newData = false;

void loop() {
  recvWithEndMarker();
  showNewData();
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (mySerial.available() > 0 && newData == false) {
    rc = mySerial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0';  // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void showNewData() {
  if (newData == true) {
    Serial.println(receivedChars);
    newData = false;

    if (receivedChars[0] == alarmRef[0] && receivedChars[1] == alarmRef[1]) {
      alarm();
    } else if (receivedChars[0] == cancelRef[0] && receivedChars[1] == cancelRef[1]) {
      cancel();
    } else if (receivedChars[0] == duringAlarmRef[0] && receivedChars[1] == duringAlarmRef[1]) {
      redState();
    } else if (receivedChars[0] == durinCancelRef[0] && receivedChars[1] == durinCancelRef[1]) {
      greenState();
    }
  }
}

// Alarm and Cancel functuion flashes blue X times and then go to Green or Red state
void alarm() {
  warningState();
  redState();
}

void cancel() {
  warningState();
  greenState();
}

// Blinking blue light
void warningState() {
  for (int i = 0; i < attentionFlashesTimes; i++) {
    blueState();
    tone(BUZZER, 50);
    delay(250);
    idle();
    noTone(BUZZER);
    delay(100);
  }
}

// Red, Green and Blue states are just static color functions
void redState() {
  analogWrite(PIN_RED, 255);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 0);
}

void greenState() {
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 255);
  analogWrite(PIN_BLUE, 0);
}

void blueState() {
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 255);
}

// Turn LED off
void idle() {
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GREEN, 0);
  analogWrite(PIN_BLUE, 0);
}
