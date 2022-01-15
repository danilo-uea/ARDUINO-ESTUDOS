/* Bibliotecas para o Display OLED*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* Pinagem para o Display Oled */
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST); /* Definicões do Display OLED */

void setup()
{
  /* Monitor Serial */
  Serial.begin(115200);

  /* Preparando a inicialização do display OLED */
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  /* Inicialização do display OLED */
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) /* Endereço 0x3C para 128 x 32 */
  {
    Serial.println(F("Falha no Display OLED"));
    for(;;); /* Loop infinito */
  } 
  else 
  {
    Serial.println("Display OLED Ok");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Display OLED Ok");
    display.display();
    delay(2000);
  }
}

int cont = 0;

void loop()
{
  Serial.println(cont);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Contador: ");
  display.println(cont);
  display.display();
  cont++;
  delay(1000);
}
