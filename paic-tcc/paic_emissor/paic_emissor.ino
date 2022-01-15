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

int cont = 0;

void envia_medicoes_serial(float temp_lida, float umid_lida, float f_latitude, float f_longitude, bool erro_dht, bool erro_gps);
void escreve_medicoes_display(float temp_lida, float umid_lida, float f_latitude, float f_longitude, bool erro_dht, bool erro_gps);

void escreve_medicoes_display(float temp_lida, float umid_lida, float f_latitude, float f_longitude, bool erro_dht, bool erro_gps)
{      
    display.clearDisplay();

    if (erro_dht) {
      display.setCursor(0, 0);
      display.print("Erro no DHT");
      display.println(DHTTYPE);
      
      display.setCursor(0, 10);
      display.println("");
    } else {
      char str_temp[10] = {0};
      char str_umid[10] = {0};
      char str_temp_max_min[20] = {0};
  
      sprintf(str_temp, "%.2fC", temp_lida);
      sprintf(str_umid, "%.2f", umid_lida);

      display.setCursor(0, 0);
      display.print("Temp: ");
      display.println(str_temp);
      
      display.setCursor(0, 10);
      display.print("Umid: ");
      display.print(str_umid);
      display.println("%");
    }

    if (erro_gps) {
      display.setCursor(0, 20);
      display.println("Erro no GPS");
  
      display.setCursor(0, 30);
      display.println("");
    } else {
      char str_flat[11] = {0};
      char str_flon[11] = {0};
      sprintf(str_flat, "%.6f", f_latitude);
      sprintf(str_flon, "%.6f", f_longitude);
      
      display.setCursor(0, 20);
      display.print("Lat: ");
      display.println(str_flat);
  
      display.setCursor(0, 30);
      display.print("Lon: ");
      display.println(str_flon);
    }
    
    display.setCursor(0, 40);
    display.print("Cont: ");
    display.print(cont);
    
    display.display();
}

void envia_medicoes_serial(float temp_lida, float umid_lida, float f_latitude, float f_longitude, bool erro_dht, bool erro_gps) 
{
  char mensagem[200];
  if (erro_dht) {
    Serial.print(" - Erro ao ler sensor DHT");
    Serial.println(DHTTYPE);
  } else {
    memset(mensagem,0,sizeof(mensagem));
    sprintf(mensagem,"Temperatura: %.2f °C", temp_lida);
    Serial.println(mensagem);
    
    memset(mensagem,0,sizeof(mensagem));
    sprintf(mensagem,"Umidade: %.2f",umid_lida);
    Serial.print(mensagem);
    Serial.println("%");
  }

  if (erro_gps) {
    Serial.println("Erro no GPS");
  } else {
    memset(mensagem,0,sizeof(mensagem));
    sprintf(mensagem,"Lat: %.6f", f_latitude);
    Serial.println(mensagem);
  
    memset(mensagem,0,sizeof(mensagem));
    sprintf(mensagem,"Lon: %.6f", f_longitude);
    Serial.println(mensagem);
  }

  Serial.print("Contador: ");
  Serial.println(cont);
  Serial.println(" ");
}

void setup() 
{
    Serial.begin(115200);
    Wire.begin(4, 15);
    dht.begin();
    ss.begin(BAUD_RATE);

    if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) 
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
    }
}

void loop()
{
  float f_latitude, f_longitude, temperatura_lida, umidade_lida;
  unsigned long idade;
  bool newData = false;
  bool erro_dht = false, erro_gps = false;

  temperatura_lida = dht.readTemperature(); /* Faz a leitura de temperatura do sensor */
  umidade_lida = dht.readHumidity(); /* Faz a leitura de umidade do sensor */

  delay(4000);

  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if ( isnan(temperatura_lida) || isnan(umidade_lida) )
    erro_dht = true;

  if (!newData)
    erro_gps = true;
  else
    gps.f_get_position(&f_latitude, &f_longitude, &idade);

  envia_medicoes_serial(temperatura_lida, umidade_lida, f_latitude, f_longitude, erro_dht, erro_gps);
  escreve_medicoes_display(temperatura_lida, umidade_lida, f_latitude, f_longitude, erro_dht, erro_gps);
  cont++;
}
