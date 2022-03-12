/* Bibliotecas para o Display OLED*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_TX "6d68efe5-04b6-4a85-abc4-c2670b7bf7fd"

/* Pinagem para o Display Oled */
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST); /* Definicões do Display OLED */

void setup() {
  Serial.begin(115200);
  
  /* Preparando a inicialização do display OLED */
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  /* Inicialização do display OLED */
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) /* Endereço 0x3C para 128 x 32 */
  {
    Serial.println(F("Falha no Display OLED"));
    for(;;); /* Loop infinito */
  } 
  else 
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Esperando conexao");
    display.display();
    delay(2000);
  }  

  /* Criar o BLE Device */
  BLEDevice::init("ESP32");

  /* Criar o BLE Server */
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  /* Criar o BLE Service */
  BLEService *pService = pServer->createService(SERVICE_UUID);

  /* Criar o BLE Characteristic */
  pCharacteristic =pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);

  /* BLE2902 precisa notificar */
  pCharacteristic->addDescriptor(new BLE2902());

  /* Iniciar o serviço */
  pService->start();

  /* Começar a anunciar */
  pServer->getAdvertising()->start();

  Serial.println("Esperando por uma conexão do cliente para notificar");
}

int cont = 0;

void loop() {
  if (deviceConnected) {

    /* Converter o array de caracter */
    char txString[200];
    sprintf(txString, "Cont: %d", cont);

    /* Definindo o valor para a característica */
    pCharacteristic->setValue(txString);

    /* Notificar o cliente conectado */
    pCharacteristic->notify();

    Serial.println(txString);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(txString);
    display.display();

    cont++;
    delay(1000);
  }
}
