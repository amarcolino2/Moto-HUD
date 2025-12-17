#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ArduinoJson.h>

TFT_eSPI tft;

#define SERVICE_UUID "0000feed-0000-1000-8000-00805f9b34fb"
#define WRITE_UUID   "0000beef-0000-1000-8000-00805f9b34fb"
#define NOTIFY_UUID  "0000ack0-0000-1000-8000-00805f9b34fb"

BLECharacteristic *notifyChar;

void drawHUD(char dir, int dist, int speed, const char* rua, int head) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(4);
  tft.drawCentreString(dir == 'L' ? "<" : dir == 'R' ? ">" : "^", 120, 20, 4);
  tft.drawCentreString(String(dist).c_str(), 120, 80, 4);
  tft.setTextFont(2);
  tft.drawCentreString(rua, 120, 140, 2);
  tft.drawString(String(speed).c_str(), 10, 200, 4);
  tft.drawRightString(String(head).c_str(), 230, 200, 2);
}

class WriteCB : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, c->getValue());
    drawHUD(
      doc["i"].as<const char*>()[0],
      doc["d"],
      doc["s"],
      doc["r"],
      doc["h"]
    );
    notifyChar->setValue("{\"ok\":1}");
    notifyChar->notify();
  }
};

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  BLEDevice::init("ESP32_HUD");
  BLEServer *server = BLEDevice::createServer();
  BLEService *service = server->createService(SERVICE_UUID);

  BLECharacteristic *writeChar = service->createCharacteristic(
    WRITE_UUID, BLECharacteristic::PROPERTY_WRITE
  );
  notifyChar = service->createCharacteristic(
    NOTIFY_UUID, BLECharacteristic::PROPERTY_NOTIFY
  );
  writeChar->setCallbacks(new WriteCB());

  service->start();
  server->getAdvertising()->start();
}

void loop() {}
