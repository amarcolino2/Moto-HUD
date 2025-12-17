# 🧪 GUIA DE TESTE - HUD Profissional

## ✅ COMPORTAMENTO NORMAL (NÃO É ERRO!)

### **Tela 1: Aguardando BLE**
```
┌─────────────────────┐
│    ESP32 HUD        │  (Azul ciano)
│                     │
│  Aguardando BLE     │
└─────────────────────┘
```
**Quando aparece:** ESP32 ligado, aguardando conexão do celular

---

### **Tela 2: CONECTADO - Aguardando dados** ⬅️ **VOCÊ ESTÁ AQUI**
```
┌─────────────────────┐
│    CONECTADO        │  (Verde)
│                     │
│ Aguardando dados... │  (Amarelo)
│                     │
│ 1. Obtenha GPS      │  (Ciano)
│ 2. Digite destino   │
│ 3. Inicie navegacao │
└─────────────────────┘
```
**Quando aparece:** BLE conectado, mas ainda não iniciou navegação

**Isso é NORMAL!** O ESP32 está aguardando você:
1. ✅ Ter sinal GPS (ícone verde no app)
2. ✅ Digitar um destino
3. ✅ Clicar em "Iniciar Navegação"

---

### **Tela 3: HUD de Navegação Ativo** ⬅️ **OBJETIVO**
```
┌─────────────────────┐
│         ↑           │  (Seta grande)
│       120 m         │
│                     │
│  65 km/h     270°   │
└─────────────────────┘
```
**Quando aparece:** Após iniciar navegação com sucesso

---

## 📋 PASSO A PASSO PARA TESTAR

### **1️⃣ Verificar GPS no App Flutter**

Abra o app e veja o status:
```
BLE: Conectado       ✅ (Verde)
GPS: GPS Ativo ✓     ✅ (Verde) ← PRECISA ESTAR VERDE!
```

Se o GPS estiver:
- 🟠 **"Aguardando GPS..."** → Aguarde capturar sinal (pode levar 10-30s)
- 🟠 **"GPS Buscando..."** → Vai ao ar livre ou perto da janela
- 🔴 **"Erro"** → Verifique permissões de localização

---

### **2️⃣ Buscar Destino**

Digite no campo de busca:
```
Rua Eutíquio Soledade, Tauá
```

Aguarde sugestões aparecerem (autocomplete via Photon API)

---

### **3️⃣ Selecionar Destino**

Clique em uma das sugestões que apareceram

---

### **4️⃣ Iniciar Navegação**

Clique no botão: **"Iniciar Navegação"**

**Se tudo OK:**
- 🔊 Ouvirá voz: "Rota calculada..."
- 📱 App mostra passo a passo
- 📺 **ESP32 muda para tela de navegação com seta**

**Se aparecer erro "Aguardando GPS":**
- ⚠️ GPS ainda não capturou posição
- Aguarde mais alguns segundos
- Verifique se está ao ar livre

---

## 🐛 TROUBLESHOOTING

### **Problema: GPS não fica verde**

**Causas:**
1. Permissão de localização não concedida
2. GPS do celular desligado
3. Está dentro de casa (sinal fraco)

**Soluções:**
1. Configurações → Apps → MotoHUD → Permissões → Localização → **"Sempre permitir"**
2. Configurações → Localização → **Ativado**
3. Vá para perto da janela ou ao ar livre

---

### **Problema: Display fica em "Aguardando dados"**

**Isso é NORMAL se você NÃO iniciou navegação ainda!**

O display SÓ muda para HUD de navegação quando você:
1. ✅ Tem GPS ativo
2. ✅ Selecionou destino
3. ✅ Clicou "Iniciar Navegação"

**Para testar AGORA sem sair de casa:**

Você pode **SIMULAR** dados de navegação editando o código Flutter:

No arquivo `lib/main.dart`, método `_updateHUD()`, adicione no início:
```dart
void _updateHUD() {
  // 🧪 TESTE: Enviar dados simulados
  if (writeChar != null) {
    final testPayload = jsonEncode({
      "i": "L",      // Esquerda
      "d": 250,      // 250 metros
      "s": 45,       // 45 km/h
      "h": 90,       // 90 graus
      "rd": 9999,    // Sem radar
      "rb": 0,
    });
    
    writeChar!.write(
      Uint8List.fromList(utf8.encode(testPayload)),
      withoutResponse: true,
    );
    print("🧪 [TESTE] Dados simulados enviados!");
  }
  return; // REMOVER depois do teste
  
  // ... resto do código original
}
```

**Resultado esperado:**
- Display mostrará seta para ESQUERDA
- Distância: 250m
- Velocidade: 45 km/h
- Heading: 90°

---

## 📊 LOGS ESPERADOS (Serial Monitor ESP32)

### **Após conectar BLE:**
```
✅ [BLE] Cliente conectado!
```

### **Após iniciar navegação:**
```
🎯 [BLE] Primeiros dados recebidos! Iniciando HUD...
📥 [BLE] Recebido:
{"i":"L","d":250,"s":45,"h":90,"rd":9999,"rb":0}
✅ [JSON] dir=L, dist=250, speed=45, heading=90
```

### **Se tiver radar próximo:**
```
🚨 [RADAR] ATIVO! Dist=180m, Diff=5°
```

---

## ✅ CHECKLIST

- [ ] App Flutter instalado e aberto
- [ ] BLE status: **CONECTADO (verde)**
- [ ] GPS status: **GPS Ativo ✓ (verde)**
- [ ] Destino digitado e selecionado
- [ ] Botão "Iniciar Navegação" clicado
- [ ] Display ESP32 mostrando seta de navegação

---

## 🎯 CONCLUSÃO

**Se o display está mostrando "CONECTADO - Aguardando dados":**

✅ **Está funcionando perfeitamente!**

Ele está aguardando você:
1. Obter GPS
2. Selecionar destino
3. Iniciar navegação

**Quando fizer isso, o HUD de navegação aparecerá automaticamente!** 🏍️💨

---

## 📞 AINDA COM DÚVIDA?

Me diga:
1. Status do GPS no app (verde/laranja/vermelho)
2. O que aparece quando clica "Iniciar Navegação"
3. Logs do Serial Monitor do ESP32
