# ESP32 HUD - PlatformIO Version

Projeto de HUD (Head-Up Display) para ESP32 com T-Display e comunicação BLE.

## Hardware

- ESP32 com T-Display (ST7789)
- Display: 135x240 pixels

## Bibliotecas

- TFT_eSPI: Para controle do display
- ArduinoJson: Para parsing de dados JSON
- BLE (ESP32): Para comunicação Bluetooth Low Energy

## Configuração

O arquivo `platformio.ini` já está configurado com os pinos corretos para o T-Display:
- TFT_MOSI: GPIO 19
- TFT_SCLK: GPIO 18
- TFT_CS: GPIO 5
- TFT_DC: GPIO 16
- TFT_RST: GPIO 23
- TFT_BL: GPIO 4

## Como usar

1. Abra o projeto no VS Code com PlatformIO instalado
2. Conecte seu ESP32
3. Clique em "Build" (ícone de check) para compilar
4. Clique em "Upload" (ícone de seta) para fazer upload

## Protocolo BLE

**Service UUID**: `0000feed-0000-1000-8000-00805f9b34fb`

**Write Characteristic**: `0000beef-0000-1000-8000-00805f9b34fb`
- Formato JSON:
```json
{
  "i": "L",      // Direção: 'L' (esquerda), 'R' (direita), '^' (reto)
  "d": 100,      // Distância em metros
  "s": 50,       // Velocidade
  "r": "Rua...", // Nome da rua
  "h": 270       // Heading (direção em graus)
}
```

**Notify Characteristic**: `0000ack0-0000-1000-8000-00805f9b34fb`
- Retorna: `{"ok":1}` após processar comando

## Estrutura do Projeto

```
esp32_hud_platformio/
├── platformio.ini      # Configurações do PlatformIO
├── src/
│   └── main.cpp        # Código principal
├── include/            # Headers personalizados (se necessário)
└── README.md          # Este arquivo
```

## Troubleshooting

Se o display não funcionar corretamente, verifique os pinos no arquivo `platformio.ini` na seção `build_flags` e ajuste conforme seu hardware específico.
