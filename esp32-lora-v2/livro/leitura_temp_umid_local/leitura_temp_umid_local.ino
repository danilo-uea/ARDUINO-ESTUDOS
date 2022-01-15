#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN    13 /* (GPIO 13) */
#define OLED_ADDR 0x3c /* Endereço I2C do display */

#define SCREEN_WIDTH 128 /* Configuração da resolucao do display (este modulo possui display 128x64) */
#define SCREEN_HEIGHT 64

#define DHTTYPE DHT11   // DHT 11
/* #define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321 */
/* #define DHTTYPE DHT21   // DHT 21 (AM2301) */

DHT dht(DHTPIN, DHTTYPE); /* objeto para comunicação com sensor DHT */

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, 16); /* objeto do display */

float temperatura_max;
float temperatura_min;

void atualiza_temperatura_max_e_minima(float temp_lida);
void envia_medicoes_para_serial(float temp_lida, float umid_lida);
void escreve_temperatura_umidade_display(float temp_lida, float umid_lida);

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

int cont = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  temperatura_max = dht.readTemperature(); /* inicializa temperaturas máxima */
  temperatura_min = temperatura_max; /* inicializa temperaturas máxima e mínima */
    
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
}

void loop() {
  float temperatura_lida;
  float umidade_lida;
  
  temperatura_lida = dht.readTemperature(); /* Faz a leitura de temperatura do sensor */
  umidade_lida = dht.readHumidity(); /* Faz a leitura de umidade do sensor */

  /* Se houver falha na leitura do sensor, escreve mensagem de erro */
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
    cont = 0;
  }  
  
  delay(5000); /* espera cinco segundos até a próxima leitura  */
}
