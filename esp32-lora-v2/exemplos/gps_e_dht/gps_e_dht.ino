#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define OLED_ADDR   0x3c
#define SCREEN_WIDTH    128 
#define SCREEN_HEIGHT   64

#define DHTPIN    13
#define DHTTYPE DHT11

const int RX_PIN = 22; /* Ligar no TX do GPS */
const int TX_PIN = 23; /* Ligar no RX do GPS */
const int BAUD_RATE = 9600;

TinyGPS gps;
SoftwareSerial ss(RX_PIN, TX_PIN);

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, 16);

void setup()
{
  Serial.begin(115200);
  Wire.begin(4, 15);
  dht.begin();

  ss.begin(BAUD_RATE);

  if(display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) 
  {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
  }
}

void loop()
{
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
  float temperatura_lida = 0, umidade_lida = 0;
  float flat = 0, flon = 0;

  Serial.println("");
  
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.print(" PREC=");
    Serial.println(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
  } else {
    gps.stats(&chars, &sentences, &failed);
    Serial.print(" CHARS=");
    Serial.print(chars);
    Serial.print(" SENTENCES=");
    Serial.print(sentences);
    Serial.print(" CSUM ERR=");
    Serial.println(failed);
    if (chars == 0)
      Serial.println("** No characters received from GPS: check wiring **");
  }

  delay(4000);
  
  temperatura_lida = dht.readTemperature(); /* Faz a leitura de temperatura do sensor */
  umidade_lida = dht.readHumidity(); /* Faz a leitura de umidade do sensor */

  if ( isnan(temperatura_lida) || isnan(umidade_lida) )
    Serial.println("Erro no DHT");
  else {
    Serial.print("Temp=");
    Serial.print(temperatura_lida);
    Serial.print("Umid=");
    Serial.println(umidade_lida);
  }

  display.clearDisplay();
  
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.println(temperatura_lida);

  display.setCursor(0, 10);
  display.print("Umid: ");
  display.println(umidade_lida);

  char str_flat[11] = {0};
  char str_flon[11] = {0};
  sprintf(str_flat, "%.6f", flat);
  sprintf(str_flon, "%.6f", flon);

  display.setCursor(0, 20);
  display.print("Lat: ");
  display.println(str_flat);

  display.setCursor(0, 30);
  display.print("Lon: ");
  display.println(str_flon);
  
  display.display();
}
