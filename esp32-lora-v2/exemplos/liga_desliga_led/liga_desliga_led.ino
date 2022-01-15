int pinoLED = 27;

void setup() {
  pinMode(pinoLED, OUTPUT);
}

void loop() {
  digitalWrite(pinoLED, HIGH);
  delay(600);
  digitalWrite(pinoLED, LOW);
  delay(600);
}

//26, 18, 19, 15
