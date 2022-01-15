int botao = 13;
int fatorE = 7;
 
void setup () {
  Serial.begin(115200);
  pinMode(botao, INPUT);
}

void loop () {
  if (digitalRead(botao) == HIGH){
    fatorE = fatorE + 1;
    if (fatorE > 12) {
      fatorE = 7;
    }
  }

  Serial.println(fatorE);
  
  delay(1000);
}
