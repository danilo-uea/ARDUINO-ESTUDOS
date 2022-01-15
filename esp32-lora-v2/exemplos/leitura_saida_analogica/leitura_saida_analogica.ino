#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_ADDR 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, 16);

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 15);
   
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
      Serial.println("Display OLED: falha ao inicializar");
  else
  {  
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
  }
}

void loop() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Ola mundo");
    display.println("Teste 1");
    display.println("Teste 2");
    display.display();
}
