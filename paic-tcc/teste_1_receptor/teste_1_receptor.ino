/* Bibliotecas para o Display OLED*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* Bibliotecas para comunicação LoRa */
#include <LoRa.h>
#include <SPI.h>

/* Biblioteca para comunicação Wifi */
#include <WiFi.h>

/* Biblioteca para comunicação MQTT */
#include <PubSubClient.h>

/* Pinagem para o Display Oled */
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64

/* Pinagem para comunicação LoRa */
#define SCK_LORA           5
#define MISO_LORA          19
#define MOSI_LORA          27
#define RESET_PIN_LORA     14
#define SS_PIN_LORA        18
#define HIGH_GAIN_LORA     20  /* dBm */
#define BAND               915E6  /* 915MHz de frequencia */

/* Defines de id_mqtt e tópicos para publicação e subscribe */
#define TOPICO_PUBLISH   "/uea/paic/danilo/dados"
#define ID_MQTT  "UeaPaicDaniloReceptor"

/* Nome e senha da rede Wifi */
const char* SSID = "Danilo";    //Lula 2022 //Danilo
const char* PASSWORD = "danilo2020"; //&zTXJEYL  //danilo2020

/* URL do broker MQTT e a porta */
const char* BROKER_MQTT = "broker.hivemq.com";
int BROKER_PORT = 1883;

/* Variáveis e objetos globais*/
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST); /* Definicões do Display OLED */
WiFiClient espClient; /* Cria o objeto espClient */
PubSubClient MQTT(espClient); /* Instancia o Cliente MQTT passando o objeto espClient */

/* Estrutura do pacote de dados */
typedef struct __attribute__((__packed__))  
{
  int contador;
  float temp_lida;
  float umid_lida;
  float f_latitude;
  float f_longitude;
  bool erro_dht;
  bool erro_gps;
} TDadosLora;

void escreve_medicoes_display(TDadosLora dados_lora, int lora_rssi, int tam_pacote);
void envia_medicoes_serial(TDadosLora dados_lora, int lora_rssi, int tam_pacote);
bool init_comunicacao_lora(void);
void init_wifi(void);
void init_MQTT(void);
void reconnect_wifi(void); 
void reconnect_MQTT(void);
void verifica_conexoes_wifi_e_MQTT(void);
void envia_informacoes_por_mqtt(TDadosLora dados_lora, int lora_rssi, int tam_pacote);

void escreve_medicoes_display(TDadosLora dados_lora, int lora_rssi, int tam_pacote)
{      
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("Cont: ");
    display.println(dados_lora.contador);
    
    display.setCursor(0, 10);
    char str_rssi_tam_pac[10] = {0};
    sprintf(str_rssi_tam_pac, "RSSI: %ddBm %dBytes", lora_rssi, tam_pacote);
    display.println(str_rssi_tam_pac);

    if (dados_lora.erro_dht) {
      display.setCursor(0, 20);
      display.println("Erro no DHT");
      
      display.setCursor(0, 30);
      display.println("");
    } else {
      char str_temp[10] = {0};
      char str_umid[10] = {0};
  
      sprintf(str_temp, "%.2fC", dados_lora.temp_lida);
      sprintf(str_umid, "%.2f", dados_lora.umid_lida);

      display.setCursor(0, 20);
      display.print("Temp: ");
      display.println(str_temp);
      
      display.setCursor(0, 30);
      display.print("Umid: ");
      display.print(str_umid);
      display.println("%");
    }

    if (dados_lora.erro_gps) {
      display.setCursor(0, 40);
      display.println("Erro no GPS");
  
      display.setCursor(0, 50);
      display.println("");
    } else {
      char str_flat[11] = {0};
      char str_flon[11] = {0};
      sprintf(str_flat, "%.6f", dados_lora.f_latitude);
      sprintf(str_flon, "%.6f", dados_lora.f_longitude);
      
      display.setCursor(0, 40);
      display.print("Lat: ");
      display.println(str_flat);
  
      display.setCursor(0, 50);
      display.print("Lon: ");
      display.println(str_flon);
    }
    
    display.display();
}

void envia_medicoes_serial(TDadosLora dados_lora, int lora_rssi, int tam_pacote) 
{
  char mensagem[200];
  
  Serial.print(dados_lora.contador);
  Serial.print(";");
  
  Serial.print(lora_rssi);
  Serial.print(";");

  Serial.print(tam_pacote);
  Serial.print(";");
  
  if (dados_lora.erro_dht) {
    Serial.print(";;");
  } else {
    memset(mensagem,0,sizeof(mensagem));
    sprintf(mensagem,"%.2f", dados_lora.temp_lida);
    Serial.print(mensagem);
    Serial.print(";");
    
    memset(mensagem,0,sizeof(mensagem));
    sprintf(mensagem,"%.2f", dados_lora.umid_lida);
    Serial.print(mensagem);
    Serial.print(";");
  }

  if (dados_lora.erro_gps) {
    Serial.print(";");
  } else {
    memset(mensagem,0,sizeof(mensagem));
    sprintf(mensagem,"%.6f", dados_lora.f_latitude);
    Serial.print(mensagem);
    Serial.print(";");
  
    memset(mensagem,0,sizeof(mensagem));
    sprintf(mensagem,"%.6f", dados_lora.f_longitude);
    Serial.print(mensagem);
  }
  
  Serial.println("");
}

bool init_comunicacao_lora(void)
{
    bool status_init = false;
    Serial.println("[LoRa Receptor] Tentando iniciar comunicacao com o radio LoRa...");
    SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
    LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);

    display.clearDisplay();
    
    if (!LoRa.begin(BAND)) {
      Serial.println("[LoRa Receptor] Comunicacao com o radio LoRa falhou. Nova tentativa em 1 segundo...");        
      status_init = false;

      display.setCursor(0, 0);
      display.println("Radio LoRa");
      display.setCursor(0, 10);
      display.println("Status: Conectando...");
      display.setCursor(0, 20);
      display.println("Tentativas: Cada 1s");
      display.display();

      delay(1000);
    } else {
      LoRa.setTxPower(HIGH_GAIN_LORA); /* Configura o ganho do receptor LoRa para 20dBm, o maior ganho possível (visando maior alcance possível) */ 
      Serial.println("[LoRa Receptor] Comunicacao com o radio LoRa ok");
      status_init = true;

      display.setCursor(0, 0);
      display.println("Radio LoRa");
      display.setCursor(0, 10);
      display.println("Status: Ok");
      display.display();
    }

    return status_init;
}

/* Inicializa e conecta-se na rede WI-FI desejada */
void init_wifi(void) 
{
  delay(10);
  Serial.print("Conectando-se na rede Wifi: ");
  Serial.println(SSID);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("WI-FI: ");
  display.println(SSID);
  display.setCursor(0, 10);
  display.println("Status: Conectando...");
  display.display();
  
  reconnect_wifi();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("WI-FI: ");
  display.println(SSID);
  display.setCursor(0, 10);
  display.println("Status: Ok");
  display.setCursor(0, 20);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
}

/* Inicializa parâmetros de conexão MQTT(endereço do broker e porta) */
void init_MQTT(void) 
{
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Broker MQTT");
  display.setCursor(0, 10);
  display.print("Status: ");
  display.println("Conectando...");
  display.setCursor(0, 20);
  display.print("URL: ");
  display.println(BROKER_MQTT);
  display.display();
  
  reconnect_MQTT();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Broker MQTT");
  display.setCursor(0, 10);
  display.print("Status: ");
  display.println("Ok");
  display.setCursor(0, 20);
  display.print("URL: ");
  display.println(BROKER_MQTT);
  display.display();
}

/* Reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair) */
void reconnect_MQTT(void) 
{  
  while (!MQTT.connected()) 
  {
      Serial.print("Tentando se conectar ao Broker MQTT: ");
      Serial.println(BROKER_MQTT);
      if (MQTT.connect(ID_MQTT)) {
          Serial.println("Conectado com sucesso ao broker MQTT!");
      } else {
          Serial.println("Falha ao reconectar no broker. Nova tentatica de conexao em 2s");
          delay(2000);
      }
  }
}

/* Reconecta-se ao WiFi */
void reconnect_wifi(void) 
{
  /* Se já está conectado a rede WI-FI, nada é feito. Caso contrário, são efetuadas tentativas de conexão */
  if (WiFi.status() == WL_CONNECTED)
      return;
      
  WiFi.begin(SSID, PASSWORD); /* Conecta na rede WI-FI */

  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(100);
      Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede: ");
  Serial.println(SSID);
  Serial.print("IP obtido: ");
  Serial.println(WiFi.localIP());
}

/* Verifica o estado das conexões WiFI e ao broker MQTT. Em caso de desconexão (qualquer uma das duas), a conexão é refeita. */
void verifica_conexoes_wifi_e_MQTT(void)
{
  reconnect_wifi(); /* Se não há conexão com o WiFI, a conexão é refeita */ 

  if (!MQTT.connected()) /* Se não há conexão com o Broker, a conexão é refeita  */ 
      reconnect_MQTT(); 
}

void envia_informacoes_por_mqtt(TDadosLora dados_lora, int lora_rssi)
{
  char mensagem_MQTT[200] = {0};

  sprintf(mensagem_MQTT, "Cont: %d, RSSI: %d, Temp: %.2f, Umid: %.2f, Lat: %.6f, Long: %.6f", dados_lora.contador, lora_rssi, dados_lora.temp_lida, dados_lora.umid_lida, dados_lora.f_latitude, dados_lora.f_longitude);
  MQTT.publish(TOPICO_PUBLISH, mensagem_MQTT);
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

  /* Mensagem inicial */
  Serial.println("RECEPTOR LORA");
  display.setCursor(0,0);
  display.print("Receptor LoRa");
  display.display();
  delay(2000);

  while(init_comunicacao_lora() == false); /* Tenta, até obter sucesso na comunicacao com o chip LoRa */
  delay(2000);
    
  //init_wifi(); /* Inicialização do WI-FI */
  //delay(2000);
  
  //init_MQTT(); /* Inicialização do MQTT */
  //delay(2000);

  /* Imprimir mensagem dizendo para esperar a chegada dos dados */
  Serial.println("Aguardando dados...");
  display.clearDisplay();    
  display.setCursor(0, 0);
  display.print("Aguardando dados...");
  display.display();
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
    escreve_medicoes_display(dados_lora, lora_rssi, tam_pacote);
    //envia_informacoes_por_mqtt(dados_lora, lora_rssi);
  }

  
  //MQTT.loop(); /* Faz o keep-alive do MQTT */
  //verifica_conexoes_wifi_e_MQTT(); /* Verifica se as conexões MQTT e wi-fi estão ativas. Se alguma delas não estiver ativa, a reconexão é feita */
}
