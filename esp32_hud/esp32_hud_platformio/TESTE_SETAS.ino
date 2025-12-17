// 🎨 TESTE VISUAL DAS SETAS - Cole no Arduino IDE para testar

#include <TFT_eSPI.h>

TFT_eSPI tft;

void drawLeftArrow(int centerX, int centerY) {
  int arrowWidth = 80;
  int arrowHeight = 60;
  int thickness = 15;
  
  int tipX = centerX - arrowWidth / 2;
  int tipY = centerY;
  int hasteStartX = tipX + 30;
  int hasteEndX = centerX + arrowWidth / 2;
  
  tft.fillRect(hasteStartX, centerY - thickness/2, 
               hasteEndX - hasteStartX, thickness, TFT_WHITE);
  
  tft.fillTriangle(
    tipX, tipY,
    hasteStartX, tipY - arrowHeight/2,
    hasteStartX, tipY + arrowHeight/2,
    TFT_WHITE
  );
}

void drawRightArrow(int centerX, int centerY) {
  int arrowWidth = 80;
  int arrowHeight = 60;
  int thickness = 15;
  
  int tipX = centerX + arrowWidth / 2;
  int tipY = centerY;
  int hasteStartX = centerX - arrowWidth / 2;
  int hasteEndX = tipX - 30;
  
  tft.fillRect(hasteStartX, centerY - thickness/2, 
               hasteEndX - hasteStartX, thickness, TFT_WHITE);
  
  tft.fillTriangle(
    tipX, tipY,
    hasteEndX, tipY - arrowHeight/2,
    hasteEndX, tipY + arrowHeight/2,
    TFT_WHITE
  );
}

void drawStraightArrow(int centerX, int centerY) {
  int arrowWidth = 60;
  int arrowHeight = 80;
  int thickness = 15;
  
  int tipX = centerX;
  int tipY = centerY - arrowHeight / 2;
  int hasteStartY = tipY + 30;
  int hasteEndY = centerY + arrowHeight / 2;
  
  tft.fillRect(centerX - thickness/2, hasteStartY, 
               thickness, hasteEndY - hasteStartY, TFT_WHITE);
  
  tft.fillTriangle(
    tipX, tipY,
    tipX - arrowWidth/2, hasteStartY,
    tipX + arrowWidth/2, hasteStartY,
    TFT_WHITE
  );
}

void setup() {
  tft.init();
  tft.setRotation(1); // Landscape
  tft.fillScreen(TFT_BLACK);
  
  // Teste 1: Seta ESQUERDA
  delay(2000);
  tft.fillScreen(TFT_BLACK);
  drawLeftArrow(120, 67); // Centro do display 240x135
  delay(2000);
  
  // Teste 2: Seta DIREITA
  tft.fillScreen(TFT_BLACK);
  drawRightArrow(120, 67);
  delay(2000);
  
  // Teste 3: Seta FRENTE
  tft.fillScreen(TFT_BLACK);
  drawStraightArrow(120, 67);
}

void loop() {
  // Loop infinito mostrando as 3 setas
  tft.fillScreen(TFT_BLACK);
  drawLeftArrow(120, 67);
  delay(2000);
  
  tft.fillScreen(TFT_BLACK);
  drawStraightArrow(120, 67);
  delay(2000);
  
  tft.fillScreen(TFT_BLACK);
  drawRightArrow(120, 67);
  delay(2000);
}
