/* Display OLED ------------------------------------------------------------------------------------------------ */
#include <Wire.h>              /* Bibliotecas */
#include <Adafruit_GFX.h>      /* Bibliotecas */
#include <Adafruit_SSD1306.h>  /* Bibliotecas */
#define OLED_SDA 4       /* Pinagem para o Display Oled */
#define OLED_SCL 15      /* Pinagem para o Display Oled */
#define OLED_RST 16      /* Pinagem para o Display Oled */
#define SCREEN_WIDTH 128 /* Pinagem para o Display Oled */
#define SCREEN_HEIGHT 64 /* Pinagem para o Display Oled */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST); /* Definicões do Display OLED */
/* ------------------------------------------------------------------------------------------------------------- */

/* LoRA -------------------------------------------------------------------------------------------------------- */
#include <LoRa.h> /* Bibliotecas */
#include <SPI.h>  /* Bibliotecas */
#define SCK_LORA           5     /* Pinagem para comunicação LoRa */
#define MISO_LORA          19    /* Pinagem para comunicação LoRa */
#define MOSI_LORA          27    /* Pinagem para comunicação LoRa */
#define RESET_PIN_LORA     14    /* Pinagem para comunicação LoRa */
#define SS_PIN_LORA        18    /* Pinagem para comunicação LoRa */
#define HIGH_GAIN_LORA     20    /* Pinagem para comunicação LoRa */ /* dBm */
#define BAND               915E6 /* Pinagem para comunicação LoRa */ /* 915MHz de frequencia */
/* ------------------------------------------------------------------------------------------------------------- */

/* Fator de Espalhamento--------------------------------------------------------------------------------------- */
int fatorE = 7;     /* Valor do fator de espalhamento */
int fatorE_ant = 7; /* Valor do fator de espalhamento anterior */
int pinoPOT = 13;   /* Pinagem para o potenciômetro do fator de espalhamento */
/* ------------------------------------------------------------------------------------------------------------- */

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
void escreve_medicoes_display(TDadosLora dados_lora, int lora_rssi);
void envia_medicoes_serial(TDadosLora dados_lora, int lora_rssi, int tam_pacote);
bool init_comunicacao_lora(void);

void aguardando_dados_display() {
  /* Imprimir mensagem dizendo para esperar a chegada dos dados */
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Aguardando dados...");
  display.setCursor(0, 10);
  display.print("F.E.: ");
  display.println(fatorE);
  display.display();
  
  delay(200);
}

void escreve_medicoes_display(TDadosLora dados_lora, int lora_rssi)
{
  char str_rssi[11];
  char str_flat[11];
  char str_flon[11];
  char str_falt[11];

  memset(str_rssi,0,sizeof(str_rssi));
  memset(str_flat,0,sizeof(str_flat));
  memset(str_flon,0,sizeof(str_flon));
  memset(str_falt,0,sizeof(str_falt));
  
  sprintf(str_rssi, "%d", lora_rssi);
  sprintf(str_flat, "%.6f", dados_lora.f_latitude);
  sprintf(str_flon, "%.6f", dados_lora.f_longitude);
  sprintf(str_falt, "%.2f", dados_lora.f_aaltitude);
      
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print("Cont: ");
  display.println(dados_lora.contador);

  display.setCursor(0, 10);
  display.print("F.E.: ");
  display.println(fatorE);
  
  display.setCursor(0, 20);
  display.print("RSSI: ");
  display.println(str_rssi);
  
  display.setCursor(0, 30);
  display.print("Lat: ");
  display.println(str_flat);

  display.setCursor(0, 40);
  display.print("Lon: ");
  display.println(str_flon);

  display.setCursor(0, 50);
  display.print("Alt: ");
  display.println(str_falt);
  
  display.display();
}

void envia_medicoes_serial(TDadosLora dados_lora, int lora_rssi, int tam_pacote) 
{
  char mensagem[80];
  
  memset(mensagem,0,sizeof(mensagem));
  sprintf(mensagem,"%d;%02d:%02d:%02d;%d;%d;%d;%.6f;%.6f;%.2f", dados_lora.contador, dados_lora.hora, dados_lora.minuto, dados_lora.segundo, fatorE, lora_rssi, tam_pacote, dados_lora.f_latitude, dados_lora.f_longitude, dados_lora.f_aaltitude);
  Serial.println(mensagem);
}

bool init_comunicacao_lora(void)
{
    bool status_init = false;
    //Serial.println("[LoRa Receptor] Tentando iniciar comunicacao com o radio LoRa...");
    SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
    LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);
        
    if (!LoRa.begin(BAND)) {
      status_init = false;
      
      //Serial.println("[LoRa Receptor] Comunicacao com o radio LoRa falhou. Nova tentativa em 1 segundo...");        
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Radio LoRa");
      display.setCursor(0, 10);
      display.println("Status: Conectando...");
      display.setCursor(0, 20);
      display.println("Tentativas: Cada 1s");
      display.display();

      delay(1000);
    } else {
      status_init = true;
      LoRa.setTxPower(HIGH_GAIN_LORA); /* Configura o ganho do receptor LoRa para 20dBm, o maior ganho possível (visando maior alcance possível) */ 
      LoRa.setSignalBandwidth(125E3);  /* Largura de banda fixa de 125 kHz *//* Suporta valores: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3 e 500E3 */
      LoRa.setCodingRate4(5);          /* Taxa de código - Suporta valores entre 5 e 8 */

      //Serial.println("[LoRa Receptor] Comunicacao com o radio LoRa ok");
      
      //display.setCursor(0, 0);
      //display.println("Radio LoRa");
      //display.setCursor(0, 10);
      //display.println("Status: Ok");
      //display.display();
    }

    return status_init;
}

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
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { /* Endereço 0x3C para 128 x 32 */
    Serial.println(F("Falha no Display OLED"));
    for(;;); /* Loop infinito */
  } else {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
  }

  while(init_comunicacao_lora() == false); /* Tenta, até obter sucesso na comunicacao com o chip LoRa */
  //delay(2000);
    
  //init_wifi(); /* Inicialização do WI-FI */
  //delay(2000);
  
  //init_MQTT(); /* Inicialização do MQTT */
  //delay(2000);

  /* Imprimir mensagem dizendo para esperar a chegada dos dados */
  //Serial.println("Aguardando dados...");
  aguardando_dados_display();
  
  /* Pino de entrada do potenciômetro */
  pinMode(pinoPOT, INPUT);
}

void loop()
{ 
  TDadosLora dados_lora;
  char byte_recebido;
  int tam_pacote = 0;
  int lora_rssi = 0;
  char * ptInformaraoRecebida = NULL;
  unsigned long idade;
  bool newData = false;

  /* Fator de Espalhamento */
  fatorE = analogRead(pinoPOT);
  fatorE = map(fatorE, 0, 4095, 7, 12);
  if (fatorE != fatorE_ant) {
    aguardando_dados_display();
  }
  fatorE_ant =  fatorE;

  /* Fator de Espalhamento */
  LoRa.setSpreadingFactor(fatorE);

  tam_pacote = LoRa.parsePacket(); /* Verifica se chegou alguma informação do tamanho esperado */

  if (tam_pacote == sizeof(TDadosLora)) {               
    ptInformaraoRecebida = (char *)&dados_lora; /* Recebe os dados conforme protocolo */
    while (LoRa.available()) 
    {
        byte_recebido = (char)LoRa.read();
        *ptInformaraoRecebida = byte_recebido;
        ptInformaraoRecebida++;
    }
    
    lora_rssi = LoRa.packetRssi(); /* Escreve RSSI de recepção e informação recebida */

    envia_medicoes_serial(dados_lora, lora_rssi, tam_pacote);
    escreve_medicoes_display(dados_lora, lora_rssi);
    //envia_informacoes_por_mqtt(dados_lora, lora_rssi, tam_pacote);
  }
  
  //MQTT.loop();                     /* Faz o keep-alive do MQTT */
  //verifica_conexoes_wifi_e_MQTT(); /* Verifica se as conexões MQTT e wi-fi estão ativas. Se alguma delas não estiver ativa, a reconexão é feita */
}
