#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);

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

    Serial.println("Valor: " + String(txString));

    cont++;
    delay(1000);
  }
}
