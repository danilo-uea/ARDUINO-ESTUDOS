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

/* Definicões do Display OLED */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST); 

/* Bibliotecas para o módulo de GPS */
#include <SoftwareSerial.h> 
//#include <HardwareSerial.h>
#include <TinyGPS.h>

/* Pinagem para o módulo de GPS */
const int RX_PIN = 22; /* Ligar no TX do GPS */
const int TX_PIN = 23; /* Ligar no RX do GPS */
const int BAUD_RATE = 9600;

/* Variáveis globais para o módulo de GPS */
TinyGPS gps;
SoftwareSerial ss(RX_PIN, TX_PIN);
//HardwareSerial ss(1);

/* Estrutura de dados */
typedef struct __attribute__((__packed__))  
{
  float f_latitude;
  float f_longitude;
  int i_sat;
  int i_prec;
} TDadosGPS;

void envia_medicoes_serial(TDadosGPS dados_gps);
void envia_medicoes_display(TDadosGPS dados_gps);

void envia_medicoes_serial(TDadosGPS dados_gps){
  Serial.print("LAT=");
  Serial.print(dados_gps.f_latitude, 6);
  Serial.print(" LON=");
  Serial.print(dados_gps.f_longitude, 6);
  Serial.print(" SAT=");
  Serial.print(dados_gps.i_sat);
  Serial.print(" PREC=");
  Serial.println(dados_gps.i_prec);
}

bool verifica = true;

void envia_medicoes_display(TDadosGPS dados_gps){
  display.clearDisplay();

  if(verifica)
    verifica = false;
  else
    verifica = true;

  display.setCursor(0, 0);
  display.print("Loop: ");
  display.println(verifica);
  
  char str_flat[11] = {0};
  sprintf(str_flat, "%.6f", dados_gps.f_latitude);
  display.setCursor(0, 10);
  display.print("Lat: ");
  display.println(str_flat);

  char str_flon[11] = {0};
  sprintf(str_flon, "%.6f", dados_gps.f_longitude);
  display.setCursor(0, 20);
  display.print("Lon: ");
  display.println(str_flon);

  display.setCursor(0, 30);
  display.print("Sat: ");
  display.println(dados_gps.i_sat);

  display.setCursor(0, 40);
  display.print("Prec: ");
  display.println(dados_gps.i_prec);

  display.display();
}

void setup()
{
  /* Monitor Serial */
  Serial.begin(115200);
  
  /* Inicialização do módulo GPS */
  ss.begin(BAUD_RATE); 
  //ss.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);

  /* Preparando a inicialização do display OLED */
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  /* Inicialização do display OLED */
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) /* Address 0x3C for 128x32 */
  { 
    Serial.println(F("Falha no Display Oled"));
    for(;;); // Don't proceed, loop forever
  }
  else /* Se der tudo certo */
  {
    Serial.print("Simple TinyGPS library v. "); 
    Serial.println(TinyGPS::library_version());
    Serial.println("by Mikal Hart");
    Serial.println();

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Display Ok");
    display.display();
    delay(2000);
  }
}

void loop()
{
  TDadosGPS dados_gps;
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
  float f_latitude, f_longitude;
  unsigned long idade;

  /* Por um segundo, analisamos os dados do GPS e relatamos alguns valores-chave */
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c)) /* Uma nova frase válida entrou? */
        newData = true;
    }
  }

  if (newData)
  {
    gps.f_get_position(&f_latitude, &f_longitude, &idade);
    dados_gps.f_latitude = f_latitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : f_latitude;
    dados_gps.f_longitude = f_longitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : f_longitude;
    dados_gps.i_sat = gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites();
    dados_gps.i_prec = gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop();
    
    envia_medicoes_serial(dados_gps);
    envia_medicoes_display(dados_gps);
  } 
  else 
  {
    gps.stats(&chars, &sentences, &failed);
    Serial.print(" CHARS=");
    Serial.print(chars);
    Serial.print(" SENTENCES=");
    Serial.print(sentences);
    Serial.print(" CSUM ERR=");
    Serial.println(failed);
    if (chars == 0)
      Serial.println("** Nenhum caractere recebido do GPS: verifique a fiação **");
  }
}
