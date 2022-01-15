#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>

#define DHTPIN    13 /* (GPIO 13) */

#define OLED_ADDR 0x3c /* Endereço I2C do display */

/* distancia, em pixels, de cada linha em relacao ao topo do display */
#define OLED_LINE1 0
#define OLED_LINE2 10
#define OLED_LINE3 20
#define OLED_LINE4 30
#define OLED_LINE5 40
#define OLED_LINE6 50

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)


#define TOPICO_PUBLISH   "/paic/uea/danilo/tempumid"    /* tópico MQTT de envio de informações para Broker */
#define ID_MQTT  "lens_Ar5FqZmhWgZOgO5RgT7NGkZTp9P"     /* id mqtt */

const char* SSID = "Lula 2022"; // coloque aqui o SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "&zTXJEYL"; // coloque aqui a senha da rede WI-FI que deseja se conectar

const char* BROKER_MQTT = "broker.hivemq.com"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT
  
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

DHT dht(DHTPIN, DHTTYPE); /* objeto para comunicação com sensor DHT */

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, 16); /* objeto do display */

float temperatura_max;
float temperatura_min;
int cont = 0;

void atualiza_temperatura_max_e_minima(float temp_lida);
void envia_medicoes_para_serial(float temp_lida, float umid_lida);
void escreve_temperatura_umidade_display(float temp_lida, float umid_lida);
void init_wifi(void);
void init_MQTT(void);
void reconnect_wifi(void); 
void reconnect_MQTT(void);
void verifica_conexoes_wifi_e_MQTT(void);
void envia_informacoes_por_mqtt(float temp_lida, float umid_lida);

void init_wifi(void) /* Inicializa e conecta-se na rede WI-FI desejada */
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");    
    reconnect_wifi();
}

void init_MQTT(void)
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
}

void reconnect_MQTT(void)
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
            Serial.println("Conectado com sucesso ao broker MQTT!");
        else 
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

void reconnect_wifi(void)
{
    if (WiFi.status() == WL_CONNECTED)
        return;
        
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
    
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
  
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void verifica_conexoes_wifi_e_MQTT(void)
{    
    reconnect_wifi(); /* se não há conexão com o WiFI, a conexão é refeita */ 
    
    if (!MQTT.connected()) /* se não há conexão com o Broker, a conexão é refeita  */ 
        reconnect_MQTT(); 
}

void atualiza_temperatura_max_e_minima(float temp_lida)
{
  if (!isnan(temp_lida) && (isnan(temperatura_max) || isnan(temperatura_min))){
    temperatura_max = temp_lida;
    temperatura_min = temp_lida;
  } else {
    if (temp_lida > temperatura_max)
      temperatura_max = temp_lida;

    if (temp_lida < temperatura_min)
      temperatura_min = temp_lida;  
  }
}

void envia_medicoes_para_serial(float temp_lida, float umid_lida)
{
  char mensagem[200];
  char i;
  
  Serial.println(" ");
  
  memset(mensagem,0,sizeof(mensagem));
  sprintf(mensagem,"- Temperatura: %.2f °C", temp_lida);
  Serial.println(mensagem);
  
  memset(mensagem,0,sizeof(mensagem));
  sprintf(mensagem,"- Umidade atual: %.2f",umid_lida);
  Serial.print(mensagem);
  Serial.println("%");
  
  memset(mensagem,0,sizeof(mensagem));
  sprintf(mensagem,"- Temperatura maxima: %.2f °C", temperatura_max);
  Serial.println(mensagem); 
  
  memset(mensagem,0,sizeof(mensagem));
  sprintf(mensagem,"- Temperatura minima: %.2f °C", temperatura_min);
  Serial.println(mensagem);
}

void escreve_temperatura_umidade_display(float temp_lida, float umid_lida)
{
    char str_temp[10] = {0};
    char str_umid[10] = {0};
    char str_temp_max_min[20] = {0};

    sprintf(str_temp, "%.2fC", temp_lida);
    sprintf(str_umid, "%.2f", umid_lida);
    sprintf(str_temp_max_min, "%.2fC / %.2fC", temperatura_min, temperatura_max);
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Temp: ");
    display.println(str_temp);
    display.setCursor(0, 10);
    display.print("Umid: ");
    display.print(str_umid);
    display.println("%");
    display.setCursor(0, 20);
    display.println(" ");
    display.setCursor(0, 30);
    display.println("Temp. min / max:");
    display.setCursor(0, 40);
    display.print(str_temp_max_min);
    display.display();
}

void envia_informacoes_por_mqtt(float temp_lida, float umid_lida)
{
  char mensagem_MQTT[200] = {0};
  sprintf(mensagem_MQTT,"Temperatura: %.2f C, Umidade atual: %.2f/100, Temperatura maxima: %.2f C, Temperatura minima: %.2f C", temp_lida, umid_lida, temperatura_max, temperatura_min);
  MQTT.publish(TOPICO_PUBLISH, mensagem_MQTT);

}

void setup() {
  Serial.begin(115200);  
  dht.begin();

  temperatura_max = dht.readTemperature();
  temperatura_min = temperatura_max;
   
   Wire.begin(4, 15); /* inicializa display OLED */
   
   if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
       Serial.println("Display OLED: falha ao inicializar");
   else
   {
       Serial.println("Display OLED: inicializacao ok");
       display.clearDisplay(); /* Limpa display */
       display.setTextSize(1); /* Configura tamanho de fonte */
       display.setTextColor(WHITE); /* Configura cor */
   }

  init_wifi();
  init_MQTT();
}

void loop() {
  float temperatura_lida;
  float umidade_lida;

  verifica_conexoes_wifi_e_MQTT();

  temperatura_lida = dht.readTemperature();
  umidade_lida = dht.readHumidity();

  /* se houve falha na leitura do sensor, escreve mensagem de erro na serial */
  if ( isnan(temperatura_lida) || isnan(umidade_lida) ) {
    cont++;    
    Serial.print(cont);
    Serial.print(" - Erro ao ler sensor DHT");
    Serial.println(DHTTYPE);
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("DHT");
    display.println(DHTTYPE);
    display.setCursor(0, 10);
    display.print("Erro ao ler sensor");
    display.setCursor(0, 20);
    display.print("Tentativa ");
    display.println(cont);
    display.display();
  } else {
    atualiza_temperatura_max_e_minima(temperatura_lida);
    envia_medicoes_para_serial(temperatura_lida, umidade_lida);
    escreve_temperatura_umidade_display(temperatura_lida, umidade_lida);
    envia_informacoes_por_mqtt(temperatura_lida, umidade_lida);
  }  
  
  MQTT.loop();
  delay(5000);
}
