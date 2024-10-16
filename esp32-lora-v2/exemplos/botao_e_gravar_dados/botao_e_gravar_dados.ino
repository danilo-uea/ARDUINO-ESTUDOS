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

/* Header-file com as funções utilizadas para 
   manipulação da partição NVS */
#include "nvs_flash.h"  
 
/* Chave atribuida ao valor a ser escrito e lido
   da partição NVS */
#define CHAVE_NVS  "teste"
 
/* Protótipos */
void grava_dado_nvs(uint32_t dado);
uint32_t le_dado_nvs(void);
 
/* Função: grava na NVS um dado do tipo interio 32-bits
 *         sem sinal, na chave definida em CHAVE_NVS
 * Parâmetros: dado a ser gravado
 * Retorno: nenhum
 */
void grava_dado_nvs(uint32_t dado)
{
    nvs_handle handler_particao_nvs;
    esp_err_t err;
    
    err = nvs_flash_init_partition("nvs");
     
    if (err != ESP_OK)
    {
        Serial.println("[ERRO] Falha ao iniciar partição NVS.");           
        return;
    }
 
    err = nvs_open_from_partition("nvs", "ns_nvs", NVS_READWRITE, &handler_particao_nvs);
    if (err != ESP_OK)
    {
        Serial.println("[ERRO] Falha ao abrir NVS como escrita/leitura"); 
        return;
    }
 
    /* Atualiza valor do horimetro total */
    err = nvs_set_u32(handler_particao_nvs, CHAVE_NVS, dado);
 
    if (err != ESP_OK)
    {
        Serial.println("[ERRO] Erro ao gravar horimetro");                   
        nvs_close(handler_particao_nvs);
        return;
    }
    else
    {
        Serial.println("Dado gravado com sucesso!");     
        nvs_commit(handler_particao_nvs);    
        nvs_close(handler_particao_nvs);      
    }
}
 
/* Função: le da NVS um dado do tipo interio 32-bits
 *         sem sinal, contido na chave definida em CHAVE_NVS
 * Parâmetros: nenhum
 * Retorno: dado lido
 */
uint32_t le_dado_nvs(void)
{
    nvs_handle handler_particao_nvs;
    esp_err_t err;
    uint32_t dado_lido;
     
    err = nvs_flash_init_partition("nvs");
     
    if (err != ESP_OK)
    {
        Serial.println("[ERRO] Falha ao iniciar partição NVS.");         
        return 0;
    }
 
    err = nvs_open_from_partition("nvs", "ns_nvs", NVS_READWRITE, &handler_particao_nvs);
    if (err != ESP_OK)
    {
        Serial.println("[ERRO] Falha ao abrir NVS como escrita/leitura");         
        return 0;
    }
 
    /* Faz a leitura do dado associado a chave definida em CHAVE_NVS */
    err = nvs_get_u32(handler_particao_nvs, CHAVE_NVS, &dado_lido);
     
    if (err != ESP_OK)
    {
        Serial.println("[ERRO] Falha ao fazer leitura do dado");
        return 0;
    }
    else
    {
        Serial.println("Dado lido com sucesso!");  
        nvs_close(handler_particao_nvs);   
        return dado_lido;
    }
}

uint32_t fatorE = 7;
int botao = 0;

void setup () {
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
    Serial.println("Display OLED Ok teste");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
  }

  uint32_t dado_lido;
  
  /* Le da NVS (na chave definida em CHAVE_NVS)
  o valor escrito e compara com o que foi escrito */
  dado_lido = le_dado_nvs();

  if (dado_lido == 0){
      Serial.println("Erro: dado lido nao eh igual ao escrito");
      
      /* Escreve na NVS (na chave definida em CHAVE_NVS)
      o valor da variável "dado_a_ser_escrito" */
      grava_dado_nvs(fatorE);
      Serial.println("Dado gravado pela primeira vez na placa!");
  } else
    fatorE = dado_lido;

  pinMode(botao, INPUT);
  
  Serial.print("FE: ");
  Serial.println(fatorE);

  display.setCursor(0, 0);
  display.print("FE: ");
  display.println(fatorE);
  display.display();
}

void loop () {

  delay(20);
  if (digitalRead(botao) == LOW){
    fatorE = fatorE +1;
    if (fatorE > 12 || fatorE < 7) {
      fatorE = 7;
    }

    grava_dado_nvs(fatorE);

    Serial.print("FE: ");
    Serial.println(fatorE);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("FE: ");
    display.println(fatorE);
    display.display();
    
    delay(500);
  }
}
