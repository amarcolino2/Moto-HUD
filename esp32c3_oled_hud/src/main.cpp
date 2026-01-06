#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

// ESP32-C3 USB CDC Serial
#if ARDUINO_USB_CDC_ON_BOOT
#define HWSerial Serial
#else
#define HWSerial Serial0
#endif

// Display OLED 72x40 - SSD1306 via I2C
// GPIO5 = SDA, GPIO6 = SCL
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);

#define SERVICE_UUID "0000feed-0000-1000-8000-00805f9b34fb"
#define WRITE_UUID   "0000beef-0000-1000-8000-00805f9b34fb"
#define NOTIFY_UUID  "0000ack0-0000-1000-8000-00805f9b34fb"

BLECharacteristic *notifyChar;

// Estado do alerta de radar
unsigned long lastBlinkTime = 0;
bool radarBlinkState = false;

// Estado da conexão BLE
bool bleConnected = false;
bool dataReceived = false;

/* ================= BLE SERVER CALLBACKS ================= */

class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    bleConnected = true;
    dataReceived = false;
    HWSerial.println("✅ [BLE] Cliente conectado!");
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(12, 10, "CONECTADO");
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(6, 22, "Aguardando");
    u8g2.drawStr(15, 32, "dados...");
    u8g2.sendBuffer();
  }

  void onDisconnect(BLEServer* pServer) override {
    bleConnected = false;
    HWSerial.println("❌ [BLE] Cliente desconectado!");
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(6, 15, "ESP32-C3");
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(3, 28, "Aguardando");
    u8g2.drawStr(18, 38, "BLE...");
    u8g2.sendBuffer();
    
    pServer->getAdvertising()->start();
    HWSerial.println("📡 Aguardando nova conexão...");
  }
};

/* ================= DESENHO DO HUD ================= */

// Desenha seta ESQUERDA compacta (6x8px)
void drawArrowLeft(int x, int y) {
  // Linha vertical
  u8g2.drawVLine(x+4, y, 6);
  // Linha horizontal
  u8g2.drawHLine(x, y+3, 5);
  // Ponta
  u8g2.drawPixel(x+1, y+2);
  u8g2.drawPixel(x+1, y+4);
  u8g2.drawPixel(x, y+3);
}

// Desenha seta DIREITA compacta (6x8px)
void drawArrowRight(int x, int y) {
  // Linha vertical
  u8g2.drawVLine(x+1, y, 6);
  // Linha horizontal
  u8g2.drawHLine(x+1, y+3, 5);
  // Ponta
  u8g2.drawPixel(x+4, y+2);
  u8g2.drawPixel(x+4, y+4);
  u8g2.drawPixel(x+5, y+3);
}

// Desenha seta CIMA compacta (6x8px)
void drawArrowUp(int x, int y) {
  // Linha vertical
  u8g2.drawVLine(x+3, y, 8);
  // Ponta
  u8g2.drawPixel(x+1, y+2);
  u8g2.drawPixel(x+2, y+1);
  u8g2.drawPixel(x+4, y+1);
  u8g2.drawPixel(x+5, y+2);
  u8g2.drawPixel(x+3, y);
}

// Converte graus para pontos cardeais
String degreesToCardinal(int degrees) {
  degrees = degrees % 360;
  if (degrees < 0) degrees += 360;
  
  if (degrees >= 337 || degrees < 23) return "N";
  else if (degrees >= 23 && degrees < 68) return "NE";
  else if (degrees >= 68 && degrees < 113) return "E";
  else if (degrees >= 113 && degrees < 158) return "SE";
  else if (degrees >= 158 && degrees < 203) return "S";
  else if (degrees >= 203 && degrees < 248) return "SW";
  else if (degrees >= 248 && degrees < 293) return "W";
  else return "NW";
}

// Formata distância (m ou km)
String formatDistance(int meters) {
  if (meters >= 1000) {
    float km = meters / 1000.0;
    return String(km, 1) + "k";
  }
  return String(meters) + "m";
}

void drawHUD(char dir, int dist, int speed, int heading, bool radarActive, int radarDist) {
  u8g2.clearBuffer();
  
  if (!radarActive) {
    // ========== MODO NORMAL (SEM RADAR) ==========
    
    // Linha 1: Seta direção (esquerda) + Distância (direita)
    // Seta (6x8px no canto esquerdo)
    if (dir == 'L') {
      drawArrowLeft(1, 1);
    } else if (dir == 'R') {
      drawArrowRight(1, 1);
    } else {
      drawArrowUp(1, 1);
    }
    
    // Distância (fonte média, alinhada à direita)
    u8g2.setFont(u8g2_font_7x13B_tr);  // Bold, maior
    String distStr = formatDistance(dist);
    int distWidth = u8g2.getStrWidth(distStr.c_str());
    u8g2.drawStr(72 - distWidth - 2, 10, distStr.c_str());
    
    // Linha 2: Velocidade (esquerda) + Heading (direita)
    u8g2.setFont(u8g2_font_6x12_tr);
    
    // Velocidade
    String speedStr = String(speed);
    u8g2.drawStr(2, 25, speedStr.c_str());
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(2, 33, "km/h");
    
    // Heading (ponto cardeal)
    String cardinal = degreesToCardinal(heading);
    u8g2.setFont(u8g2_font_6x12_tr);
    int cardWidth = u8g2.getStrWidth(cardinal.c_str());
    u8g2.drawStr(72 - cardWidth - 2, 25, cardinal.c_str());
    
    // Graus (menor, embaixo)
    u8g2.setFont(u8g2_font_4x6_tr);
    String degStr = String(heading) + "°";
    int degWidth = u8g2.getStrWidth(degStr.c_str());
    u8g2.drawStr(72 - degWidth - 2, 33, degStr.c_str());
    
  } else {
    // ========== MODO ALERTA RADAR (PRIORIDADE) ==========
    
    // Piscar a cada 400ms
    if (millis() - lastBlinkTime > 400) {
      radarBlinkState = !radarBlinkState;
      lastBlinkTime = millis();
    }
    
    if (radarBlinkState) {
      // Tela cheia invertida (fundo branco, texto preto)
      u8g2.drawBox(0, 0, 72, 40);  // Fundo branco
      u8g2.setDrawColor(0);  // Desenhar em preto
      
      // Texto "RADAR" grande centralizado
      u8g2.setFont(u8g2_font_7x13B_tr);
      String radarStr = "RADAR";
      int radarWidth = u8g2.getStrWidth(radarStr.c_str());
      u8g2.drawStr((72 - radarWidth) / 2, 15, radarStr.c_str());
      
      // Distância do radar
      u8g2.setFont(u8g2_font_6x12_tr);
      String radarDistStr = formatDistance(radarDist);
      int radarDistWidth = u8g2.getStrWidth(radarDistStr.c_str());
      u8g2.drawStr((72 - radarDistWidth) / 2, 30, radarDistStr.c_str());
      
      u8g2.setDrawColor(1);  // Restaurar cor normal
    } else {
      // Frame escuro - mostrar navegação reduzida
      
      // Seta pequena no topo
      if (dir == 'L') {
        drawArrowLeft(2, 2);
      } else if (dir == 'R') {
        drawArrowRight(2, 2);
      } else {
        drawArrowUp(2, 2);
      }
      
      // Distância navegação (pequena)
      u8g2.setFont(u8g2_font_4x6_tr);
      String distStr = formatDistance(dist);
      u8g2.drawStr(10, 8, distStr.c_str());
      
      // Velocidade compacta no canto
      u8g2.setFont(u8g2_font_5x7_tr);
      String speedStr = String(speed);
      u8g2.drawStr(2, 38, speedStr.c_str());
    }
  }
  
  u8g2.sendBuffer();
}

/* ================= BLE CALLBACK ================= */

class WriteCB : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    HWSerial.println("🔔 [DEBUG] onWrite() chamado!");
    
    std::string value = c->getValue();
    int len = value.length();
    
    HWSerial.printf("📊 [DEBUG] Tamanho: %d bytes\n", len);
    
    if (len == 0) {
      HWSerial.println("⚠️ [DEBUG] Valor vazio recebido!");
      return;
    }
    
    if (!dataReceived) {
      dataReceived = true;
      HWSerial.println("🎯 [BLE] Primeiros dados recebidos! Iniciando HUD...");
    }
    
    HWSerial.println("📥 [BLE] Recebido:");
    HWSerial.println(value.c_str());

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, value);

    if (err) {
      HWSerial.print("❌ [JSON] Erro: ");
      HWSerial.println(err.c_str());
      return;
    }

    // Extrair campos do JSON
    String dirStr = doc["i"] | "S";
    char dir = dirStr[0];
    int dist = doc["d"] | 0;
    int speed = doc["s"] | 0;
    int heading = doc["h"] | 0;
    int radarDist = doc["rd"] | 9999;
    int radarBearing = doc["rb"] | 0;

    // ========== LÓGICA DO RADAR ==========
    bool radarActive = false;
    
    if (radarDist < 300) {
      int angleDiff = abs(heading - radarBearing);
      
      if (angleDiff > 180) {
        angleDiff = 360 - angleDiff;
      }
      
      if (angleDiff <= 45) {
        radarActive = true;
        HWSerial.printf("🚨 [RADAR] ATIVO! Dist=%dm, Diff=%d°\n", radarDist, angleDiff);
      } else {
        HWSerial.printf("✅ [RADAR] Ignorado (sentido contrário). Diff=%d°\n", angleDiff);
      }
    }

    HWSerial.printf("✅ [JSON] dir=%c, dist=%d, speed=%d, heading=%d\n", 
                  dir, dist, speed, heading);

    // Atualizar display
    drawHUD(dir, dist, speed, heading, radarActive, radarDist);

    // Enviar ACK
    notifyChar->setValue("{\"ok\":1}");
    notifyChar->notify();
  }
};

/* ================= SETUP ================= */

void setup() {
  HWSerial.begin(115200);
  delay(1000); // Aguardar USB CDC inicializar
  HWSerial.println("🚀 ESP32-C3 OLED HUD Iniciando...");

  // Inicializar display OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(6, 15, "ESP32-C3");
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(3, 28, "Aguardando");
  u8g2.drawStr(18, 38, "BLE...");
  u8g2.sendBuffer();

  HWSerial.println("📶 Inicializando BLE...");
  BLEDevice::init("ESP32_HUD");
  BLEServer *server = BLEDevice::createServer();
  
  server->setCallbacks(new ServerCB());
  
  BLEService *service = server->createService(SERVICE_UUID);

  BLECharacteristic *writeChar = service->createCharacteristic(
    WRITE_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );

  notifyChar = service->createCharacteristic(
    NOTIFY_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  writeChar->setCallbacks(new WriteCB());
  
  HWSerial.println("📋 [DEBUG] Característica Write criada:");
  HWSerial.print("  UUID: ");
  HWSerial.println(WRITE_UUID);
  HWSerial.println("  Propriedades: WRITE");
  HWSerial.println("  Callback registrado: WriteCB");

  service->start();
  server->getAdvertising()->start();
  
  HWSerial.println("✅ BLE Ativo! Nome: ESP32_HUD");
  HWSerial.println("📡 Aguardando conexão...");
}

void loop() {
  delay(10);
}
