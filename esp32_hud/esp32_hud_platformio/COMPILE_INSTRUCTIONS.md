# 🔧 Como Compilar e Fazer Upload no ESP32

## ✅ O que foi corrigido no código ESP32:

### 1️⃣ **Adicionado suporte aos campos do JSON**
- ✅ `"i"` - Direção (L/R/S) 
- ✅ `"d"` - Distância em metros
- ✅ `"s"` - Velocidade em km/h
- ✅ `"r"` - **NOVO!** Nome da rua
- ✅ `"h"` - Heading (recebido mas não usado ainda)

### 2️⃣ **Adicionado logs de debug via Serial**
```
📥 [BLE] Recebido: {"i":"L","d":123,"r":"Av Atlantica","s":45,"h":90}
✅ [JSON] dir=L, dist=123, speed=45, road=Av Atlantica
```

### 3️⃣ **Melhorado o display HUD**
- 🧭 Seta de direção **maior** (fonte 7)
- 📏 Distância em **metros** destacada (fonte 6)
- 🛣️ **Nome da rua** no centro (fonte 2, azul ciano)
- ⚡ Velocidade no canto (fonte 4, verde)

---

## 📋 Como Compilar:

### **Opção 1: VS Code + PlatformIO Extension**
1. Abra o VS Code
2. Abra a pasta: `D:\Estudos\Projetos\Oculos RA\HUD_ESP32_Flutter\esp32_hud\esp32_hud_platformio`
3. Conecte o ESP32 via USB
4. Pressione: **Ctrl + Alt + U** (Upload)
5. Aguarde compilação e upload

### **Opção 2: Arduino IDE**
1. Abra o Arduino IDE
2. Instale bibliotecas:
   - `TFT_eSPI`
   - `ArduinoJson` (versão 7+)
   - `ESP32 BLE Arduino`
3. Abra: `src/main.cpp`
4. Selecione placa: **ESP32 Dev Module**
5. Clique em **Upload** (➡️)

### **Opção 3: Terminal (se PlatformIO instalado)**
```cmd
cd "D:\Estudos\Projetos\Oculos RA\HUD_ESP32_Flutter\esp32_hud\esp32_hud_platformio"
platformio run -t upload
```

---

## 🔍 Como Monitorar os Logs (Serial Monitor):

### **VS Code + PlatformIO:**
- Pressione: **Ctrl + Alt + S**
- Baud rate: **115200**

### **Arduino IDE:**
- Tools → Serial Monitor
- Baud rate: **115200**

### **Terminal:**
```cmd
platformio device monitor -b 115200
```

---

## 📊 O que você verá nos logs:

```
🚀 ESP32 HUD Iniciando...
📶 Inicializando BLE...
✅ BLE Ativo! Nome: ESP32_HUD
📡 Aguardando conexão...

[Quando app Flutter conectar e enviar dados:]

📥 [BLE] Recebido:
{"i":"L","d":250,"r":"Rua Eutíquio Soledade","s":35,"h":180}
✅ [JSON] dir=L, dist=250, speed=35, road=Rua Eutíquio Soledade
```

---

## 🚨 **Se der erro de compilação:**

### Erro: `'min' was not declared`
Adicione no topo do arquivo:
```cpp
#include <algorithm>
using std::min;
```

### Erro: Biblioteca não encontrada
Instale via PlatformIO Library Manager:
```cmd
platformio lib install "TFT_eSPI"
platformio lib install "ArduinoJson@^7.0.0"
```

---

## 🧪 Como Testar se Está Funcionando:

1. **Compile e faça upload** no ESP32
2. **Abra Serial Monitor** (115200 baud)
3. **Execute o app Flutter** no celular
4. **Conecte ao ESP32_HUD**
5. **Digite destino e inicie navegação**
6. **Veja logs no Serial Monitor:**
   - Deve aparecer: `📥 [BLE] Recebido: {...}`
   - Deve aparecer: `✅ [JSON] dir=..., dist=..., speed=..., road=...`
7. **Display ESP32 deve mostrar:**
   - Seta grande no topo (< > ^)
   - Distância em metros (ex: "250m")
   - Nome da rua (ex: "Rua Eutíquio Soledade")
   - Velocidade no canto (ex: "35 km/h")

---

## 🎯 **Próximos Passos:**

Após compilar e testar:
1. Verifique se aparecem logs no Serial Monitor
2. Me avise se o display está mostrando os dados
3. Se não funcionar, copie os logs e me envie

**Compile agora e me diga o que acontece!** 🚀
