/* Bibliotecas para o Display OLED*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* Bibliotecas para comunicação LoRa */
#include <LoRa.h>
#include <SPI.h>

/* Bibliotecas para o módulo de GPS */
#include <SoftwareSerial.h>
#include <TinyGPS.h>

/* Biblioteca para o sensor DHT */
#include <DHT.h>

/* Pinagem para o Display Oled */
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/* Pinagem para comunicação com radio LoRa */
#define SCK_LORA           5
#define MISO_LORA          19
#define MOSI_LORA          27
#define RESET_PIN_LORA     14
#define SS_PIN_LORA        18
#define HIGH_GAIN_LORA     20  /* dBm */
#define BAND               915E6  /* 915MHz de frequencia */

/* Pinagem para o módulo de GPS */
const int RX_PIN = 22; /* Ligar no TX do GPS */
const int TX_PIN = 23; /* Ligar no RX do GPS */
const int BAUD_RATE = 9600;

/* Pinagem para o botão do fator de espalhamento */
int fatorE = 7;
int botao =13;

/* Variáveis globais para o módulo de GPS */
TinyGPS gps;
SoftwareSerial ss(RX_PIN, TX_PIN);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST); /* Definicões do Display OLED */

/* Estrutura do pacote de dados */
typedef struct __attribute__((__packed__))  
{
  int contador;
  byte hora;
  byte minuto;
  byte segundo;
  float f_latitude;
  float f_longitude;
  float f_aaltitude;
} TDadosLora;

void aguardando_dados_display();
void envia_medicoes_serial(TDadosLora dados_lora);
void escreve_medicoes_display(TDadosLora dados_lora);
void envia_informacoes_lora(TDadosLora dados_lora);
bool init_comunicacao_lora(void);

void aguardando_dados_display() {
  /* Imprimir mensagem dizendo que está aguardando o funcionamento do GPS */
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Aguardando GPS...");
  display.setCursor(0, 10);
  display.print("F.E.: ");
  display.println(fatorE);
  display.display();
  
  delay(200);
}

void escreve_medicoes_display(TDadosLora dados_lora)
{      
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("Cont: ");
    display.println(dados_lora.contador);

    display.setCursor(0, 10);
    display.print("F.E.: ");
    display.println(fatorE);

    char str_flat[11] = {0};
    char str_flon[11] = {0};
    char str_falt[11] = {0};
    sprintf(str_flat, "%.6f", dados_lora.f_latitude);
    sprintf(str_flon, "%.6f", dados_lora.f_longitude);
    sprintf(str_falt, "%.2f", dados_lora.f_aaltitude);
    
    display.setCursor(0, 20);
    display.print("Lat: ");
    display.println(str_flat);

    display.setCursor(0, 30);
    display.print("Lon: ");
    display.println(str_flon);

    display.setCursor(0, 40);
    display.print("Alt: ");
    display.println(str_falt);
    
    display.display();
}

void envia_medicoes_serial(TDadosLora dados_lora) 
{
  char mensagem[200];

  Serial.print("Contador: ");
  Serial.println(dados_lora.contador);

  memset(mensagem,0,sizeof(mensagem));
  sprintf(mensagem,"Hora: %d:%d:%d", dados_lora.hora, dados_lora.minuto, dados_lora.segundo);
  Serial.println(mensagem);

  Serial.print("Fator de Espalhamento: ");
  Serial.println(fatorE);
  
  memset(mensagem,0,sizeof(mensagem));
  sprintf(mensagem,"Lat: %.6f", dados_lora.f_latitude);
  Serial.println(mensagem);

  memset(mensagem,0,sizeof(mensagem));
  sprintf(mensagem,"Lon: %.6f", dados_lora.f_longitude);
  Serial.println(mensagem);

  memset(mensagem,0,sizeof(mensagem));
  sprintf(mensagem,"Altitude: %.2f", dados_lora.f_aaltitude);
  Serial.println(mensagem);
  
  Serial.println(" ");
}

void envia_informacoes_lora(TDadosLora dados_lora) 
{
  LoRa.setSpreadingFactor(fatorE);
  LoRa.beginPacket();
  LoRa.write((unsigned char *)&dados_lora, sizeof(TDadosLora));
  LoRa.endPacket();
}

bool init_comunicacao_lora(void)
{
    bool status_init = false;
    Serial.println("[LoRa Emissor] Tentando iniciar comunicacao com o radio LoRa...");
    SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
    LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);

    display.clearDisplay();
    
    if (!LoRa.begin(BAND)) 
    {
      Serial.println("[LoRa Emissor] Comunicacao com o radio LoRa falhou. Nova tentativa em 1 segundo...");        
      status_init = false;

      display.setCursor(0, 0);
      display.println("Radio LoRa");
      display.setCursor(0, 10);
      display.println("Status: Conectando...");
      display.setCursor(0, 20);
      display.println("Tentativas: Cada 1s");
      display.display();

      delay(1000);
    }
    else
    {
      LoRa.setTxPower(HIGH_GAIN_LORA); /* Configura o ganho do receptor LoRa para 20dBm, o maior ganho possível (visando maior alcance possível) */ 
      LoRa.setSignalBandwidth(125E3);  /* Largura de banda fixa de 125 kHz - Suporta valores: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3 e 500E3 */
      LoRa.setCodingRate4(5);          /* Taxa de código - Suporta valores entre 5 e 8 */
      
      Serial.println("[LoRa Emissor] Comunicacao com o radio LoRa ok");
      status_init = true;

      display.setCursor(0, 0);
      display.println("Radio LoRa");
      display.setCursor(0, 10);
      display.println("Status: Ok");
      display.display();
    }

    return status_init;
}

void setup() 
{
/* Inicialização do módulo GPS */
  ss.begin(BAUD_RATE);
  
  /* Monitor Serial */
  Serial.begin(115200);

  /* Preparando a inicialização do display OLED */
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  /* Inicialização do display OLED */
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("Falha no Display Oled"));
    for(;;); // Don't proceed, loop forever
  }

  /* Mensagem inicial */
  Serial.println("EMISSOR LORA");
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Emissor LoRa");
  display.display();
  delay(2000); 

  while(init_comunicacao_lora() == false); /* Tenta, até obter sucesso na comunicacao com o chip LoRa */

  delay(2000);

  /* Imprimir mensagem dizendo que está aguardando o funcionamento do GPS */
  aguardando_dados_display();

  /* Pino de entrada do potenciômetro */
  pinMode(botao, INPUT);
}

int cont = 0;

void loop()
{
  TDadosLora dados_lora;
  dados_lora.contador = cont;
  unsigned long idade;
  bool newData = false;
       
  /* Realiza a leitura do módulo GPS */
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c)) {
        newData = true;
        break;
      }
    }

    /* Fator de Espalhamento */
    if (digitalRead(botao) == HIGH){
      fatorE = fatorE + 1;
      if (fatorE > 12) {
        fatorE = 7;
      }
      aguardando_dados_display();
    }
  }

  /* Verifica se a leitura do módulo de GPS foi bem sucedida */
  if (newData)
  { 
    gps.f_get_position(&dados_lora.f_latitude, &dados_lora.f_longitude, &idade);
    dados_lora.f_aaltitude = gps.f_altitude();

    //Dia e Hora
     int ano;
     byte mes, dia, centesimo;
     gps.crack_datetime(&ano, &mes, &dia, &dados_lora.hora, &dados_lora.minuto, &dados_lora.segundo, &centesimo, &idade);

    envia_medicoes_serial(dados_lora);
    escreve_medicoes_display(dados_lora);
    envia_informacoes_lora(dados_lora);
    
    cont++;
  }

  //delay(1000);
}
