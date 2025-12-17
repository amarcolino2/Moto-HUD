# 🔐 Correção de Permissões - HUD BLE

## ✅ Mudanças Implementadas

### 1. **AndroidManifest.xml - Permissões Adicionadas**

Foram adicionadas TODAS as permissões necessárias:

**Para Android 12+ (API 31+):**
- ✅ `BLUETOOTH_SCAN` - para escanear dispositivos Bluetooth
- ✅ `BLUETOOTH_CONNECT` - para conectar aos dispositivos
- ✅ `BLUETOOTH_ADVERTISE` - para advertising

**Para Android 11 e anteriores:**
- ✅ `BLUETOOTH` - permissão básica de Bluetooth
- ✅ `BLUETOOTH_ADMIN` - para gerenciar Bluetooth

**Para GPS:**
- ✅ `ACCESS_FINE_LOCATION` - localização precisa
- ✅ `ACCESS_COARSE_LOCATION` - localização aproximada

### 2. **Pacote permission_handler Instalado**

Adicionado `permission_handler: ^11.0.1` ao `pubspec.yaml` para solicitar permissões em tempo de execução.

### 3. **Código de Solicitação de Permissões Melhorado**

Agora o app:
- ✅ Solicita TODAS as permissões ao iniciar
- ✅ Mostra um diálogo se permissões forem negadas permanentemente
- ✅ Oferece botão para abrir as Configurações do app
- ✅ Mostra logs detalhados do status de cada permissão

---

## 📱 Como Testar

### 1. **Desinstale o app antigo do celular**
```bash
adb uninstall com.example.hud_ble
```

### 2. **Execute o app novamente**
```bash
flutter run
```

### 3. **Ao abrir o app:**
- Você verá diálogos solicitando permissões
- **IMPORTANTE:** Aceite TODAS as permissões!
- Se negar alguma, o app mostrará um diálogo para abrir as Configurações

---

## 🔍 Verificando Permissões (via ADB)

```bash
# Ver todas as permissões do app
adb shell dumpsys package com.example.hud_ble | findstr "permission"

# Conceder permissões manualmente (se necessário)
adb shell pm grant com.example.hud_ble android.permission.BLUETOOTH_SCAN
adb shell pm grant com.example.hud_ble android.permission.BLUETOOTH_CONNECT
adb shell pm grant com.example.hud_ble android.permission.ACCESS_FINE_LOCATION
```

---

## ⚠️ Importante no Android 12+

No Android 12 (API 31) e superior, você precisa aceitar:

1. **"Dispositivos Próximos"** - Para scan Bluetooth
2. **"Localização"** - Necessária para scan Bluetooth (exigência do Android)
3. **"Localização Precisa"** - Para GPS funcionar

**Nota:** Mesmo que você não use a localização para o Bluetooth, o Android OBRIGA a permissão de localização para escanear dispositivos Bluetooth.

---

## 🐛 Solução de Problemas

### ❌ "Permissão negada" ainda aparece

1. **Desinstale completamente o app:**
   ```bash
   adb uninstall com.example.hud_ble
   ```

2. **Limpe o cache do Flutter:**
   ```bash
   flutter clean
   ```

3. **Reinstale:**
   ```bash
   flutter run
   ```

### ❌ Diálogo de permissão não aparece

1. Vá em **Configurações > Apps > HUD BLE**
2. Toque em **Permissões**
3. Ative manualmente:
   - Dispositivos Próximos (ou Bluetooth)
   - Localização (Precisa)

### ❌ "Negado permanentemente"

Se você negou as permissões e marcou "Não perguntar novamente":

1. O app mostrará um diálogo
2. Clique em **"Abrir Configurações"**
3. Ative manualmente todas as permissões
4. Volte ao app e tente conectar novamente

---

## 📋 Checklist de Permissões

Antes de tentar conectar ao ESP32, certifique-se:

- [ ] Bluetooth do celular está ligado
- [ ] GPS do celular está ligado
- [ ] App tem permissão de "Dispositivos Próximos" ou "Bluetooth"
- [ ] App tem permissão de "Localização Precisa"
- [ ] App não está em modo de economia de bateria restritivo
- [ ] ESP32 está ligado e em modo advertising

---

## 🚀 Próximos Passos

Após executar o app:

1. Aceite todas as permissões
2. Verifique os logs no console
3. Clique em "Conectar ESP32"
4. Os logs mostrarão todos os dispositivos encontrados
5. Se o ESP32 aparecer, ele conectará automaticamente

Se precisar de ajuda, compartilhe os logs do console! 📊
