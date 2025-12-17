# Instruções de Debug - HUD BLE

## Melhorias Implementadas

### 1. **Logs Detalhados** 📊
Agora o app mostra no console:
- 🔍 Todos os dispositivos encontrados durante o scan
- 📡 Estado do Bluetooth
- ✅ Confirmação de conexão
- 🔍 Todos os serviços e características descobertos
- 📤 Dados sendo enviados ao ESP32
- ❌ Erros detalhados com stack trace

### 2. **Interface Melhorada** 🎨
- Status visual em tempo real
- Indicador de carregamento durante conexão
- Informações do dispositivo conectado
- Velocidade e direção em tempo real
- Botão de desconectar

### 3. **Tratamento de Erros** 🛡️
- Verifica se Bluetooth está ligado
- Timeout de conexão de 10 segundos
- Diálogos de erro informativos
- Previne múltiplas conexões simultâneas

### 4. **Busca Melhorada** 🔎
Agora aceita dispositivos com nome contendo:
- "ESP32"
- "ESP_"
- Maiúsculas ou minúsculas

## Como Usar

### 1. **Conectar o Celular**
```bash
# Verificar se o celular está conectado
adb devices

# Se não aparecer, reconecte o USB e ative "Depuração USB"
```

### 2. **Executar o App**
```bash
cd "d:\Estudos\Projetos\Oculos RA\HUD_ESP32_Flutter\flutter_app"
flutter run
```

### 3. **Ver os Logs**
Os logs aparecerão no terminal automaticamente. Procure por:
- 🔍 "Dispositivo encontrado" - mostra todos os dispositivos
- ✅ "ESP32 encontrado" - quando detecta o ESP32
- ❌ "ERRO" - se houver algum problema

## Problemas Comuns

### ❌ "ESP32 não encontrado"
**Causas possíveis:**
1. ESP32 não está ligado ou não está em modo de advertising
2. Nome do ESP32 não contém "ESP32" ou "ESP_"
3. ESP32 está fora do alcance

**Solução:**
- Verifique no log todos os dispositivos encontrados
- Se seu ESP32 aparecer com outro nome, modifique a linha 76 do código:
  ```dart
  if (deviceName.toUpperCase().contains("SEU_NOME_AQUI")) {
  ```

### ❌ "Serviço não encontrado"
**Causa:** Os UUIDs no app não correspondem aos UUIDs do ESP32

**Solução:**
- Verifique os logs: "Serviço: xxxx"
- Compare com os UUIDs do seu ESP32
- Atualize no código (linhas 32-34)

### ❌ "Por favor, ligue o Bluetooth"
**Solução:** Ative o Bluetooth nas configurações do Android

### ❌ Nenhum dispositivo aparece
**Solução:** 
1. Vá em Configurações > Apps > Seu App > Permissões
2. Ative todas as permissões (Localização, Bluetooth, Nearby Devices)
3. Reinicie o app

## Testando sem ESP32

Se quiser testar a busca mesmo sem ESP32:
1. Modifique a linha 76 para aceitar qualquer dispositivo:
   ```dart
   if (deviceName.isNotEmpty) {  // Conecta no primeiro dispositivo com nome
   ```
2. Salve e faça hot reload (pressione 'r' no terminal)

## Verificando Permissões

Execute no terminal:
```bash
adb shell dumpsys package com.example.hud_ble | find "permission"
```

Deve mostrar:
- android.permission.BLUETOOTH_SCAN: granted=true
- android.permission.BLUETOOTH_CONNECT: granted=true
- android.permission.ACCESS_FINE_LOCATION: granted=true

## UUIDs do Projeto

- **Service UUID:** 0000feed-0000-1000-8000-00805f9b34fb
- **Write UUID:** 0000beef-0000-1000-8000-00805f9b34fb
- **Notify UUID:** 0000ack0-0000-1000-8000-00805f9b34fb

⚠️ **Certifique-se que o ESP32 usa exatamente estes UUIDs!**
