const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to

int sensorValue = 0;        // value read from the speaker

int mailState = LOW;
int lastMailnState = LOW;   // the previous reading from the input pin

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 30000;    // the debounce time; increase if the output flickers


void setup() {
  Serial.begin(9600);
}

void loop() {
  sensorValue = analogRead(analogInPin);

  if (sensorValue >= 500 && mailState == LOW) {
    lastDebounceTime = millis();
    Serial.print("1");
    mailState = HIGH;
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay && mailState == HIGH) {
    mailState = LOW;
  }
  delay(10); // wait 10 milliseconds before the next loop for the analog-to-digital converter to settle after the last reading:
}
