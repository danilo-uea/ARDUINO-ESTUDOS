/*
* Projeto: emissor - comunicação LoRa ponto-a-ponto
* Autor: Pedro Bertoleti
*/
#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* Definicoes para comunicação com radio LoRa */
#define SCK_LORA           5
#define MISO_LORA          19
#define MOSI_LORA          27
#define RESET_PIN_LORA     14
#define SS_PIN_LORA        18

#define HIGH_GAIN_LORA     20  /* dBm */
#define BAND               915E6  /* 915MHz de frequencia */

/* Definicoes do OLED */
#define OLED_SDA_PIN    4
#define OLED_SCL_PIN    15
#define SCREEN_WIDTH    128 
#define SCREEN_HEIGHT   64  
#define OLED_ADDR       0x3C 
#define OLED_RESET      16

/* Offset de linhas no display OLED */
#define OLED_LINE1     0
#define OLED_LINE2     10
#define OLED_LINE3     20
#define OLED_LINE4     30
#define OLED_LINE5     40
#define OLED_LINE6     50

/* Definicoes gerais */
#define DEBUG_SERIAL_BAUDRATE    115200

/* Variaveis e objetos globais */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* Variaveis globais */
long informacao_a_ser_enviada = 0;

/* Local prototypes */
bool init_comunicacao_lora(void);

/* Funcao: inicia comunicação com chip LoRa
 * Parametros: nenhum
 * Retorno: true: comunicacao ok
 *          false: falha na comunicacao
*/
bool init_comunicacao_lora(void)
{
    bool status_init = false;
    Serial.println("[LoRa Sender] Tentando iniciar comunicacao com o radio LoRa...");
    SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
    LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);
    
    if (!LoRa.begin(BAND)) 
    {
        Serial.println("[LoRa Sender] Comunicacao com o radio LoRa falhou. Nova tentativa em 1 segundo...");        
        delay(1000);
        status_init = false;
    }
    else
    {
        /* Configura o ganho do receptor LoRa para 20dBm, o maior ganho possível (visando maior alcance possível) */ 
        LoRa.setTxPower(HIGH_GAIN_LORA); 
        Serial.println("[LoRa Sender] Comunicacao com o radio LoRa ok");
        status_init = true;
    }

    return status_init;
}

void display_init(void);

/* Funcao: inicializa comunicacao com o display OLED
 * Parametros: nenhnum
 * Retorno: nenhnum
*/ 
void display_init(void)
{
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) 
    {
        Serial.println("[LoRa Receiver] Falha ao inicializar comunicacao com OLED");        
    }
    else
    {
        Serial.println("[LoRa Receiver] Comunicacao com OLED inicializada com sucesso");
    
        /* Limpa display e configura tamanho de fonte */
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
    }
}

/* Funcao de setup */
void setup() 
{
    /* Preparando a inicialização do display OLED */
    pinMode(OLED_RESET, OUTPUT);
    digitalWrite(OLED_RESET, LOW);
    delay(20);
    digitalWrite(OLED_RESET, HIGH);
    /* Configuracao para o display OLED */
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    display_init();
    display.clearDisplay();    
    display.setCursor(0, OLED_LINE1);
    display.print("Pronto para enviar");
    display.display();
    delay(1000);
    
    Serial.begin(DEBUG_SERIAL_BAUDRATE);
    while (!Serial);

    /* Tenta, até obter sucesso, comunicacao com o chip LoRa */
    while(init_comunicacao_lora() == false);       
}

/* Programa principal */
void loop() 
{
    /* Envia a informação */
    LoRa.beginPacket();
    LoRa.write((unsigned char *)&informacao_a_ser_enviada, sizeof(informacao_a_ser_enviada));
    LoRa.endPacket();
    Serial.println(informacao_a_ser_enviada);

    display.clearDisplay();   
    display.setCursor(0, OLED_LINE1);
    display.print("Enviando ");
    display.println(informacao_a_ser_enviada);
    display.display();
    
    /* Incrementa a informação para o próximo envio e aguarda 
       1 segundo até enviar a próxima informação */
    informacao_a_ser_enviada++;
    delay(1000);
    
}
