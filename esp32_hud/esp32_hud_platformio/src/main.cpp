#include <Arduino.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ArduinoJson.h>
#include <algorithm>

using std::min;

TFT_eSPI tft;

#define SERVICE_UUID "0000feed-0000-1000-8000-00805f9b34fb"
#define WRITE_UUID   "0000beef-0000-1000-8000-00805f9b34fb"
#define NOTIFY_UUID  "0000ack0-0000-1000-8000-00805f9b34fb"

BLECharacteristic *notifyChar;

// Estado do alerta de radar
unsigned long lastBlinkTime = 0;
bool radarBlinkState = false;

// Estado da conexão BLE
bool bleConnected = false;
bool dataReceived = false;  // Flag para saber se já recebeu dados

/* ================= BLE SERVER CALLBACKS ================= */

class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    bleConnected = true;
    dataReceived = false;
    Serial.println("✅ [BLE] Cliente conectado!");
    
    // Atualizar display - tela de aguardo
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("CONECTADO", 120, 35, 4);
    
    tft.setTextColor(TFT_YELLOW);
    tft.setTextFont(2);
    tft.drawString("Aguardando dados...", 120, 70, 2);
    
    tft.setTextColor(TFT_CYAN);
    tft.setTextFont(1);
    tft.drawString("1. Obtenha sinal GPS", 120, 95, 1);
    tft.drawString("2. Digite destino", 120, 105, 1);
    tft.drawString("3. Inicie navegacao", 120, 115, 1);
  }

  void onDisconnect(BLEServer* pServer) override {
    bleConnected = false;
    Serial.println("❌ [BLE] Cliente desconectado!");
    
    // Atualizar display
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN);
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("ESP32 HUD", 120, 40, 4);
    tft.setTextFont(2);
    tft.drawString("Aguardando BLE", 120, 90, 2);
    
    // Reiniciar advertising
    pServer->getAdvertising()->start();
    Serial.println("📡 Aguardando nova conexão...");
  }
};

/* ================= DESENHO DE VIA E VEÍCULO (ESTILO AUTOMOTIVO) ================= */

// Desenha o veículo (triângulo ciano sólido fixo no centro)
void drawVehicle(int centerX, int centerY) {
  // Asa delta com geometria precisa - aumentado e esticado
  // Ângulo no nariz: ~35° (agudo)
  // Ângulos nas asas: ~65-80° (obtusos)
  
  // Definição dos três pontos (aumentados em ~30% e esticados):
  // Ponto A (nariz): à frente, ângulo agudo
  int noseX = centerX;
  int noseY = centerY - 22;  // Esticado: -16 → -22 (38% mais longo)
  
  // Ponto B (asa esquerda): recuado e à esquerda
  int leftWingX = centerX - 18;  // Aumentado: -14 → -18 (29% maior)
  int leftWingY = centerY + 16;   // Esticado: +12 → +16 (33% mais longo)
  
  // Ponto C (asa direita): recuado e à direita
  int rightWingX = centerX + 18;  // Aumentado: +14 → +18 (29% maior)
  int rightWingY = centerY + 16;   // Esticado: +12 → +16 (33% mais longo)
  
  // Triângulo principal (asa delta) - COR CIANO (mesma do HEADING)
  tft.fillTriangle(
    noseX, noseY,              // Ponto A (nariz, ângulo ~35°)
    leftWingX, leftWingY,      // Ponto B (asa esquerda, ângulo ~70°)
    rightWingX, rightWingY,    // Ponto C (asa direita, ângulo ~70°)
    TFT_CYAN                   // COR ALTERADA: TFT_BLUE → TFT_CYAN
  );
  
  // BORDA REMOVIDA - sem drawTriangle
}

// Desenha via RETA (linha branca vertical suave)
void drawStraightRoad(int centerX, int centerY) {
  int roadWidth = 12;  // 1/3 da largura original (35/3 ≈ 12px)
  int roadHeight = 100;
  
  // Via principal (retângulo vertical)
  tft.fillRect(centerX - roadWidth/2, centerY - roadHeight/2, 
               roadWidth, roadHeight, TFT_DARKGREY);
  
  // Linhas laterais brancas
  for (int i = 0; i < roadHeight; i += 2) {
    tft.drawPixel(centerX - roadWidth/2, centerY - roadHeight/2 + i, TFT_WHITE);
    tft.drawPixel(centerX + roadWidth/2, centerY - roadHeight/2 + i, TFT_WHITE);
  }
  
  // Linha central tracejada
  for (int i = 0; i < roadHeight; i += 8) {
    tft.drawFastVLine(centerX, centerY - roadHeight/2 + i, 4, TFT_YELLOW);
  }
}

// Desenha via com CURVA À ESQUERDA (via curva fluida)
void drawLeftCurve(int centerX, int centerY) {
  // Via curvando para esquerda - desenhar com curvas Bezier simplificadas
  int roadWidth = 12;  // 1/3 da largura original (muito estreita)
  
  // Desenhar a via como uma série de linhas curvas
  for (int y = -50; y <= 50; y += 2) {
    // Curva suave à esquerda usando função quadrática
    int xOffset = -(y * y) / 100; // Curva mais acentuada no topo
    int leftEdge = centerX - roadWidth/2 + xOffset;
    int rightEdge = centerX + roadWidth/2 + xOffset;
    
    // Fundo da via
    tft.drawFastHLine(leftEdge, centerY + y, rightEdge - leftEdge, TFT_DARKGREY);
    
    // Bordas brancas
    if (y % 4 == 0) {
      tft.drawPixel(leftEdge, centerY + y, TFT_WHITE);
      tft.drawPixel(rightEdge, centerY + y, TFT_WHITE);
    }
    
    // Linha central tracejada
    if (y % 12 < 6 && y % 2 == 0) {
      tft.drawPixel(centerX + xOffset, centerY + y, TFT_YELLOW);
    }
  }
}

// Desenha via com CURVA À DIREITA (via curva fluida)
void drawRightCurve(int centerX, int centerY) {
  int roadWidth = 12;  // 1/3 da largura original (muito estreita)
  
  for (int y = -50; y <= 50; y += 2) {
    // Curva suave à direita
    int xOffset = (y * y) / 100;
    int leftEdge = centerX - roadWidth/2 + xOffset;
    int rightEdge = centerX + roadWidth/2 + xOffset;
    
    // Fundo da via
    tft.drawFastHLine(leftEdge, centerY + y, rightEdge - leftEdge, TFT_DARKGREY);
    
    // Bordas brancas
    if (y % 4 == 0) {
      tft.drawPixel(leftEdge, centerY + y, TFT_WHITE);
      tft.drawPixel(rightEdge, centerY + y, TFT_WHITE);
    }
    
    // Linha central tracejada
    if (y % 12 < 6 && y % 2 == 0) {
      tft.drawPixel(centerX + xOffset, centerY + y, TFT_YELLOW);
    }
  }
}

// MANTIDO PARA COMPATIBILIDADE - mas usa novo estilo
void drawLeftArrow(int centerX, int centerY) {
  drawLeftCurve(centerX, centerY - 10);
  drawVehicle(centerX, centerY + 10);  // Subido: +20 → +10 (mais próximo da via)
}

// Seta para DIREITA
void drawRightArrow(int centerX, int centerY) {
  drawRightCurve(centerX, centerY - 10);
  drawVehicle(centerX, centerY + 10);  // Subido: +20 → +10 (mais próximo da via)
}

// Seta para FRENTE (via reta)
void drawStraightArrow(int centerX, int centerY) {
  drawStraightRoad(centerX, centerY);
  drawVehicle(centerX, centerY + 20);  // Subido: +30 → +20 (mais próximo da via)
}

// Converte graus (0-360) para pontos cardeais (N, NE, E, SE, S, SW, W, NW)
String degreesToCardinal(int degrees) {
  // Normalizar para 0-360
  degrees = degrees % 360;
  if (degrees < 0) degrees += 360;
  
  // Dividir em 8 setores de 45° cada (com offset de 22.5° para centralizar)
  if (degrees >= 337.5 || degrees < 22.5) return "N";
  else if (degrees >= 22.5 && degrees < 67.5) return "NE";
  else if (degrees >= 67.5 && degrees < 112.5) return "E";
  else if (degrees >= 112.5 && degrees < 157.5) return "SE";
  else if (degrees >= 157.5 && degrees < 202.5) return "S";
  else if (degrees >= 202.5 && degrees < 247.5) return "SW";
  else if (degrees >= 247.5 && degrees < 292.5) return "W";
  else return "NW";
}

// Desenha indicador de direção no canto esquerdo (seta branca estilo GPS profissional)
void drawDirectionIndicator(char dir) {
  int x = 20;   // Posição X no canto esquerdo
  int y = 30;   // Posição Y superior - subida de 40 para 30 (10px mais alto)
  
  if (dir == 'L') {
    // Seta para ESQUERDA (←) - Tamanho profissional 25px
    // Linha horizontal principal (mais grossa - 5 linhas paralelas)
    tft.drawFastHLine(x, y - 2, 25, TFT_WHITE);
    tft.drawFastHLine(x, y - 1, 25, TFT_WHITE);
    tft.drawFastHLine(x, y, 25, TFT_WHITE);
    tft.drawFastHLine(x, y + 1, 25, TFT_WHITE);
    tft.drawFastHLine(x, y + 2, 25, TFT_WHITE);
    
    // Ponta da seta (mais pronunciada e grossa)
    for (int i = 0; i < 8; i++) {
      tft.drawLine(x, y, x + 8, y - i, TFT_WHITE);
      tft.drawLine(x, y, x + 8, y + i, TFT_WHITE);
    }
  } else if (dir == 'R') {
    // Seta para DIREITA (→) - Tamanho profissional 25px
    // Linha horizontal principal (mais grossa - 5 linhas paralelas)
    tft.drawFastHLine(x, y - 2, 25, TFT_WHITE);
    tft.drawFastHLine(x, y - 1, 25, TFT_WHITE);
    tft.drawFastHLine(x, y, 25, TFT_WHITE);
    tft.drawFastHLine(x, y + 1, 25, TFT_WHITE);
    tft.drawFastHLine(x, y + 2, 25, TFT_WHITE);
    
    // Ponta da seta (mais pronunciada e grossa)
    for (int i = 0; i < 8; i++) {
      tft.drawLine(x + 24, y, x + 16, y - i, TFT_WHITE);
      tft.drawLine(x + 24, y, x + 16, y + i, TFT_WHITE);
    }
  } else {
    // Seta para FRENTE (↑) - Tamanho profissional 25px
    // Linha vertical principal (mais grossa - 5 linhas paralelas)
    tft.drawFastVLine(x + 10, y, 25, TFT_WHITE);
    tft.drawFastVLine(x + 11, y, 25, TFT_WHITE);
    tft.drawFastVLine(x + 12, y, 25, TFT_WHITE);
    tft.drawFastVLine(x + 13, y, 25, TFT_WHITE);
    tft.drawFastVLine(x + 14, y, 25, TFT_WHITE);
    
    // Ponta da seta (mais pronunciada e grossa)
    for (int i = 0; i < 8; i++) {
      tft.drawLine(x + 12, y, x + 12 - i, y + 8, TFT_WHITE);
      tft.drawLine(x + 12, y, x + 12 + i, y + 8, TFT_WHITE);
    }
  }
}

/* ================= DISPLAY HUD ================= */

void drawHUD(char dir, int dist, int speed, int heading, bool radarActive, int radarDist) {
  tft.fillScreen(TFT_BLACK);
  
  // ========== INDICADOR DE DIREÇÃO (canto esquerdo superior) ==========
  drawDirectionIndicator(dir);
  
  // ========== REGIÃO CENTRAL: SETA DE NAVEGAÇÃO ==========
  int centerX = 120; // 240/2
  int centerY = 50;  // Parte superior
  
  if (!radarActive) {
    // Seta normal
    if (dir == 'L') {
      drawLeftArrow(centerX, centerY);
    } else if (dir == 'R') {
      drawRightArrow(centerX, centerY);
    } else {
      drawStraightArrow(centerX, centerY);
    }
  } else {
    // Seta reduzida quando alerta ativo
    if (dir == 'L') {
      drawLeftArrow(centerX, 30);
    } else if (dir == 'R') {
      drawRightArrow(centerX, 30);
    } else {
      drawStraightArrow(centerX, 30);
    }
  }
  
  // ========== DISTÂNCIA ATÉ MANOBRA ==========
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  
  if (!radarActive) {
    tft.drawString(String(dist) + " m", centerX, 115, 4);  // Movido de 100 para 115 (mais baixo, distante da via)
  } else {
    tft.setTextFont(2);
    tft.drawString(String(dist) + " m", centerX, 65, 2);  // Movido de 60 para 65
  }
  
  // ========== ALERTA DE RADAR (PRIORIDADE VISUAL) ==========
  if (radarActive) {
    // Piscar a cada 500ms
    if (millis() - lastBlinkTime > 500) {
      radarBlinkState = !radarBlinkState;
      lastBlinkTime = millis();
    }
    
    if (radarBlinkState) {
      // Faixa vermelha piscante
      tft.fillRect(0, 80, 240, 50, TFT_RED);
      tft.setTextColor(TFT_WHITE, TFT_RED);
      tft.setTextFont(4);
      tft.drawString("RADAR", centerX, 95, 4);
      tft.setTextFont(2);
      tft.drawString(String(radarDist) + " m", centerX, 115, 2);
    }
  }
  
  // ========== VELOCIDADE (canto inferior esquerdo) ==========
  tft.setTextColor(TFT_GREEN);
  tft.setTextFont(4);
  tft.setTextDatum(BL_DATUM);
  tft.drawString(String(speed), 10, 130, 4);
  tft.setTextFont(2);
  tft.drawString("km/h", 10, 110, 2);
  
  // ========== HEADING / BÚSSOLA (canto inferior direito) ==========
  tft.setTextColor(TFT_CYAN);
  tft.setTextFont(3);  // Fonte ajustada: 2 → 3
  tft.setTextDatum(BR_DATUM);
  
  // Converter graus para ponto cardeal (N, NE, E, etc.)
  String cardinal = degreesToCardinal(heading);
  tft.drawString(cardinal, 230, 130, 3);  // Fonte 3
}

/* ================= BLE CALLBACK ================= */

class WriteCB : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    Serial.println("🔔 [DEBUG] onWrite() chamado!");
    
    std::string value = c->getValue();
    int len = value.length();
    
    Serial.printf("📊 [DEBUG] Tamanho: %d bytes\n", len);
    
    if (len == 0) {
      Serial.println("⚠️ [DEBUG] Valor vazio recebido!");
      return;
    }
    
    // Marcar que dados foram recebidos
    if (!dataReceived) {
      dataReceived = true;
      Serial.println("🎯 [BLE] Primeiros dados recebidos! Iniciando HUD...");
    }
    
    Serial.println("📥 [BLE] Recebido:");
    Serial.println(value.c_str());

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, value);

    if (err) {
      Serial.print("❌ [JSON] Erro: ");
      Serial.println(err.c_str());
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
      // Calcular diferença angular entre heading e radar bearing
      int angleDiff = abs(heading - radarBearing);
      
      // Normalizar para 0-180 (menor ângulo)
      if (angleDiff > 180) {
        angleDiff = 360 - angleDiff;
      }
      
      // Ativar alerta se radar estiver no mesmo sentido (± 45°)
      if (angleDiff <= 45) {
        radarActive = true;
        Serial.printf("🚨 [RADAR] ATIVO! Dist=%dm, Diff=%d°\n", radarDist, angleDiff);
      } else {
        Serial.printf("✅ [RADAR] Ignorado (sentido contrário). Diff=%d°\n", angleDiff);
      }
    }

    Serial.printf("✅ [JSON] dir=%c, dist=%d, speed=%d, heading=%d\n", 
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
  Serial.begin(115200);
  Serial.println("🚀 ESP32 HUD Iniciando...");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_CYAN);
  tft.setTextFont(4);
  tft.drawCentreString("ESP32 HUD", 120, 40, 4);

  tft.setTextFont(2);
  tft.drawCentreString("Aguardando BLE", 120, 90, 2);

  Serial.println("📶 Inicializando BLE...");
  BLEDevice::init("ESP32_HUD");
  BLEServer *server = BLEDevice::createServer();
  
  // Registrar callbacks de conexão/desconexão
  server->setCallbacks(new ServerCB());
  
  BLEService *service = server->createService(SERVICE_UUID);

  BLECharacteristic *writeChar = service->createCharacteristic(
    WRITE_UUID,
    BLECharacteristic::PROPERTY_WRITE  // Removido WRITE_NR para disparar callback
  );

  notifyChar = service->createCharacteristic(
    NOTIFY_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  writeChar->setCallbacks(new WriteCB());
  
  Serial.println("📋 [DEBUG] Característica Write criada:");
  Serial.print("  UUID: ");
  Serial.println(WRITE_UUID);
  Serial.println("  Propriedades: WRITE | WRITE_NR");
  Serial.println("  Callback registrado: WriteCB");

  service->start();
  server->getAdvertising()->start();
  
  Serial.println("✅ BLE Ativo! Nome: ESP32_HUD");
  Serial.println("📡 Aguardando conexão...");
}

void loop() {
  // Loop vazio - callbacks BLE são processados automaticamente
  delay(10);
}
