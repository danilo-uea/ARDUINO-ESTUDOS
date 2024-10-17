const int ledPin = 25;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT); /* Led */
}

void loop() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');

    if (comando == "ligar") {
      digitalWrite(ledPin, HIGH);
    } else if (comando == "desligar") {
      digitalWrite(ledPin, LOW);
    }
  }
}
