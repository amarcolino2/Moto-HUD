import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_tts/flutter_tts.dart';
import 'package:geolocator/geolocator.dart';
import 'package:http/http.dart' as http;
import 'package:permission_handler/permission_handler.dart';

import 'photon_search.dart'; // 🌟 Buscador mais eficiente

const SERVICE_UUID = "0000feed-0000-1000-8000-00805f9b34fb";
const WRITE_UUID = "0000beef-0000-1000-8000-00805f9b34fb";

void main() {
  runApp(const MotoHUDApp());
}

class MotoHUDApp extends StatelessWidget {
  const MotoHUDApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      theme: ThemeData.dark(),
      home: const HomePage(),
    );
  }
}

class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  BluetoothDevice? esp32;
  BluetoothCharacteristic? writeChar;

  String bleStatus = "Desconectado";
  String gpsStatus = "Aguardando GPS..."; // 📍 Status do GPS

  final FlutterTts tts = FlutterTts();

  List<dynamic> steps = [];
  int stepIndex = 0;
  Position? lastPos;

  final TextEditingController destinationCtrl = TextEditingController();
  List<Map<String, String>> suggestions = [];
  bool isLoadingSuggestions = false;
  Timer? _debounceTimer;

  @override
  void initState() {
    super.initState();
    _initBLE();
    _initGPS();
  }

  @override
  void dispose() {
    _debounceTimer?.cancel();
    destinationCtrl.dispose();
    super.dispose();
  }

  // ================= BLE =================

  Future<void> _initBLE() async {
    setState(() => bleStatus = "Procurando ESP32...");

    await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.location,
    ].request();

    if (!await FlutterBluePlus.isSupported) {
      setState(() => bleStatus = "Bluetooth não suportado");
      return;
    }

    final state = await FlutterBluePlus.adapterState.first;
    if (state != BluetoothAdapterState.on) {
      setState(() => bleStatus = "Ative o Bluetooth");
      return;
    }

    await FlutterBluePlus.startScan(timeout: const Duration(seconds: 6));
    await Future.delayed(const Duration(seconds: 3));

    for (var r in FlutterBluePlus.lastScanResults) {
      if (r.device.platformName.contains("ESP32")) {
        await FlutterBluePlus.stopScan();

        esp32 = r.device;
        setState(() => bleStatus = "Conectando...");

        await esp32!.connect(timeout: const Duration(seconds: 10));
        setState(() => bleStatus = "Conectado");

        final services = await esp32!.discoverServices();
        for (var s in services) {
          if (s.uuid.toString().toLowerCase().contains("feed")) {
            for (var c in s.characteristics) {
              if (c.uuid.toString().toLowerCase().contains("beef")) {
                writeChar = c;
                return;
              }
            }
          }
        }
      }
    }

    setState(() => bleStatus = "ESP32 não encontrado");
  }

  Future<void> _disconnectBLE() async {
    await esp32?.disconnect();
    setState(() {
      bleStatus = "Desconectado";
      esp32 = null;
      writeChar = null;
    });
  }

  // ================= GPS =================

  Future<void> _initGPS() async {
    setState(() => gpsStatus = "Aguardando GPS...");

    await Geolocator.requestPermission();

    // 🎯 Tentar obter posição inicial imediatamente
    try {
      lastPos = await Geolocator.getCurrentPosition(
        desiredAccuracy: LocationAccuracy.high,
      ).timeout(const Duration(seconds: 10));

      setState(() => gpsStatus = "GPS Ativo ✓");
      print(
          "📍 [GPS] Posição inicial: ${lastPos!.latitude}, ${lastPos!.longitude}");
    } catch (e) {
      print("⚠️ [GPS] Erro ao obter posição inicial: $e");
      setState(() => gpsStatus = "GPS Buscando...");
    }

    // Continuar ouvindo atualizações
    Geolocator.getPositionStream(
      locationSettings: const LocationSettings(
        accuracy: LocationAccuracy.best,
        distanceFilter: 5,
      ),
    ).listen((pos) {
      lastPos = pos;
      if (gpsStatus != "GPS Ativo ✓") {
        setState(() => gpsStatus = "GPS Ativo ✓");
        print("📍 [GPS] Sinal capturado!");
      }
      _updateHUD();
    });
  }

  // ================= AUTOCOMPLETE =================

  void _onSearchChanged(String query) {
    _debounceTimer?.cancel();
    _debounceTimer = Timer(const Duration(milliseconds: 500), () {
      _searchSuggestions(query);
    });
  }

  Future<void> _searchSuggestions(String query) async {
    if (query.length < 3) {
      setState(() {
        suggestions = [];
        isLoadingSuggestions = false;
      });
      return;
    }

    setState(() => isLoadingSuggestions = true);

    try {
      // 🌟 USAR PHOTON - Mais eficiente que Nominatim
      final results = await PhotonSearch.search(
        query,
        lat: lastPos?.latitude,
        lon: lastPos?.longitude,
      );

      setState(() {
        suggestions = results;
        isLoadingSuggestions = false;
      });
    } catch (e) {
      print("❌ [SEARCH] Erro: $e");
      setState(() {
        suggestions = [];
        isLoadingSuggestions = false;
      });
    }
  }

  // 📝 Construir endereço inteligente com número
  void _selectSuggestion(Map<String, String> s) {
    setState(() {
      destinationCtrl.text = s["name"]!;
      suggestions = [];
      isLoadingSuggestions = false;
    });

    // Calcular rota e enviar primeiro passo ao ESP32
    _calculateRouteFromCoords(
      double.parse(s["lat"]!),
      double.parse(s["lon"]!),
    );
  }

  // ================= ROTA =================

  Future<void> _calculateRouteManual(String destination) async {
    if (lastPos == null) {
      await tts.speak("Aguardando sinal de GPS.");
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text(
              "⚠️ Aguardando GPS... Verifique se a localização está ativada"),
          duration: Duration(seconds: 3),
        ),
      );
      return;
    }

    if (destination.isEmpty) {
      await tts.speak("Digite um destino.");
      return;
    }

    try {
      // Buscar coordenadas do destino
      String searchQuery = destination;
      if (!destination.toLowerCase().contains('rio de janeiro') &&
          !destination.toLowerCase().contains('brasil')) {
        searchQuery = '$destination, Rio de Janeiro, Brasil';
      }

      final geoUrl = Uri.parse(
        'https://nominatim.openstreetmap.org/search?q=${Uri.encodeComponent(searchQuery)}&format=json&limit=1',
      );

      final geoRes = await http.get(
        geoUrl,
        headers: {'User-Agent': 'MotoHUD/1.0'},
      );

      if (geoRes.statusCode == 200) {
        final List geoData = jsonDecode(geoRes.body);

        if (geoData.isEmpty) {
          await tts.speak("Destino não encontrado.");
          return;
        }

        final lat = double.parse(geoData[0]['lat']);
        final lon = double.parse(geoData[0]['lon']);

        await _calculateRouteFromCoords(lat, lon);
      }
    } catch (e) {
      print("❌ [ROTA] Erro: $e");
      await tts.speak("Erro ao calcular rota.");
    }
  }

  Future<void> _calculateRouteFromCoords(double lat, double lon) async {
    if (lastPos == null) return;

    try {
      final url = "https://router.project-osrm.org/route/v1/driving/"
          "${lastPos!.longitude},${lastPos!.latitude};$lon,$lat"
          "?steps=true&overview=false";

      final res = await http.get(Uri.parse(url));

      if (res.statusCode != 200) {
        await tts.speak("Erro ao calcular rota.");
        return;
      }

      final data = jsonDecode(res.body);

      if (data["routes"] == null || data["routes"].isEmpty) {
        await tts.speak("Nenhuma rota encontrada.");
        return;
      }

      setState(() {
        steps = data["routes"][0]["legs"][0]["steps"];
        stepIndex = 0;
      });

      final distance =
          (data["routes"][0]["distance"] / 1000).toStringAsFixed(1);
      final duration = (data["routes"][0]["duration"] / 60).round();

      print("✅ [ROTA] Rota calculada: $distance km, $duration min");
      await tts.speak(
          "Rota calculada. $distance quilômetros. Tempo estimado: $duration minutos. Boa viagem.");

      // Enviar primeiro passo imediatamente
      _updateHUD();
    } catch (e) {
      print("❌ [ROTA] Erro: $e");
      await tts.speak("Falha no roteamento.");
    }
  }

  // ================= HUD =================

  Future<void> _updateHUD() async {
    if (steps.isEmpty || writeChar == null || lastPos == null) return;

    final step = steps[stepIndex];
    final dist = step["distance"].round();

    if (dist < 15 && stepIndex < steps.length - 1) {
      stepIndex++;
    }

    // Extrair direção da manobra
    final maneuver = step["maneuver"]["modifier"] ?? "straight";
    final dir = maneuver.contains("left")
        ? "L"
        : maneuver.contains("right")
            ? "R"
            : "S";

    // 🚨 RADAR: Calcular distância e bearing do radar mais próximo
    // TODO: Integrar com API de radares (Waze, Google Maps, etc)
    // Por enquanto, valores simulados para demonstração
    int radarDistance = 9999; // metros (9999 = sem radar próximo)
    int radarBearing = 0; // graus (0-360)

    // Exemplo: Simular radar a 250m no bearing 90° (leste)
    // radarDistance = 250;
    // radarBearing = 90;

    final payload = jsonEncode({
      "i": dir, // Direção: L, R, S
      "d": dist, // Distância até manobra
      "s": (lastPos!.speed * 3.6).round(), // Velocidade km/h
      "h": step["maneuver"]["bearing_after"] ?? 0, // Heading atual
      "rd": radarDistance, // Distância até radar
      "rb": radarBearing, // Bearing do radar
    });

    print("📤 [HUD] Enviando: $payload");

    try {
      await writeChar!.write(
        Uint8List.fromList(utf8.encode(payload)),
        withoutResponse: false, // COM resposta para disparar callback no ESP32
      );
      print("✅ [HUD] Dados enviados!");
    } catch (e) {
      print("❌ [HUD] Erro ao enviar: $e");
    }
  }

  // ================= UI =================

  @override
  Widget build(BuildContext context) {
    final isConnected = bleStatus == "Conectado";
    final hasGPS = lastPos != null;

    return Scaffold(
      appBar: AppBar(title: const Text("Moto HUD Navigation")),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          children: [
            // 📶 Status BLE
            Text(
              "BLE: $bleStatus",
              style: TextStyle(
                color: isConnected ? Colors.green : Colors.red,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 8),
            // 📍 Status GPS
            Text(
              "GPS: $gpsStatus",
              style: TextStyle(
                color: hasGPS ? Colors.green : Colors.orange,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 12),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                ElevatedButton(
                  onPressed: isConnected ? null : _initBLE,
                  child: const Text("Conectar"),
                ),
                const SizedBox(width: 10),
                ElevatedButton(
                  onPressed: isConnected ? _disconnectBLE : null,
                  child: const Text("Desconectar"),
                ),
              ],
            ),
            const SizedBox(height: 16),
            TextField(
              controller: destinationCtrl,
              decoration: InputDecoration(
                labelText: "Destino",
                border: const OutlineInputBorder(),
                prefixIcon: const Icon(Icons.location_on),
                hintText: "Ex: Av Atlantica 1000, ou só Copacabana",
                helperText: "💡 Aceita: rua + número, bairro ou local",
                helperStyle: const TextStyle(fontSize: 11, color: Colors.grey),
                suffixIcon: isLoadingSuggestions
                    ? const Padding(
                        padding: EdgeInsets.all(12.0),
                        child: SizedBox(
                          width: 20,
                          height: 20,
                          child: CircularProgressIndicator(strokeWidth: 2),
                        ),
                      )
                    : null,
              ),
              onChanged: _onSearchChanged,
            ),

            // Botão Iniciar Navegação
            const SizedBox(height: 12),
            SizedBox(
              width: double.infinity,
              child: ElevatedButton.icon(
                onPressed:
                    destinationCtrl.text.isNotEmpty && suggestions.isEmpty
                        ? () => _calculateRouteManual(destinationCtrl.text)
                        : null,
                icon: const Icon(Icons.navigation),
                label: const Text("Iniciar Navegação"),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.green,
                  padding: const EdgeInsets.symmetric(vertical: 16),
                ),
              ),
            ),

            // Info de rota ativa
            if (steps.isNotEmpty) ...[
              const SizedBox(height: 16),
              Container(
                padding: const EdgeInsets.all(12),
                decoration: BoxDecoration(
                  color: Colors.blue.withOpacity(0.2),
                  borderRadius: BorderRadius.circular(8),
                  border: Border.all(color: Colors.blue),
                ),
                child: Row(
                  children: [
                    const Icon(Icons.route, color: Colors.blue, size: 24),
                    const SizedBox(width: 12),
                    Expanded(
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          const Text(
                            "Navegação ativa",
                            style: TextStyle(
                              fontSize: 16,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          const SizedBox(height: 4),
                          Text(
                            "Passo ${stepIndex + 1} de ${steps.length}",
                            style: const TextStyle(fontSize: 13),
                          ),
                        ],
                      ),
                    ),
                    IconButton(
                      icon: const Icon(Icons.close, color: Colors.red),
                      onPressed: () => setState(() {
                        steps = [];
                        stepIndex = 0;
                      }),
                      tooltip: "Cancelar navegação",
                    ),
                  ],
                ),
              ),
            ],

            // Sugestões
            if (suggestions.isNotEmpty)
              Expanded(
                child: Container(
                  margin: const EdgeInsets.only(top: 8),
                  decoration: BoxDecoration(
                    color: Colors.grey[850],
                    borderRadius: BorderRadius.circular(8),
                    border: Border.all(color: Colors.blue),
                  ),
                  child: ListView(
                    children: suggestions
                        .map((s) => ListTile(
                              dense: true,
                              leading: const Icon(Icons.place,
                                  color: Colors.blue, size: 20),
                              title: Text(
                                s["name"]!,
                                style: const TextStyle(fontSize: 14),
                                maxLines: 2,
                                overflow: TextOverflow.ellipsis,
                              ),
                              onTap: () => _selectSuggestion(s),
                            ))
                        .toList(),
                  ),
                ),
              ),
          ],
        ),
      ),
    );
  }
}
