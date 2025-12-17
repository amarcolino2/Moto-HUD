import 'dart:convert';

import 'package:http/http.dart' as http;

/// 🌟 PHOTON GEOCODER - Mais eficiente que Nominatim
/// Aceita números de endereço e funciona melhor com endereços brasileiros
class PhotonSearch {
  static const String _baseUrl = 'https://photon.komoot.io/api/';

  /// Busca endereços usando Photon (baseado em OpenStreetMap)
  static Future<List<Map<String, String>>> search(
    String query, {
    double? lat,
    double? lon,
    int limit = 5,
  }) async {
    try {
      // Usar coordenadas do centro do Brasil como padrão (mais neutro)
      lat ??= -15.7939;
      lon ??= -47.8828;

      final url = Uri.parse(
        '$_baseUrl?'
        'q=${Uri.encodeComponent(query.trim())}&'
        'limit=$limit&'
        'lat=$lat&lon=$lon',
      );

      print("🔍 [PHOTON] Buscando: $query");

      final res = await http.get(url, headers: {'User-Agent': 'MotoHUD/1.0'});

      if (res.statusCode == 200) {
        final data = jsonDecode(res.body);
        final List features = data['features'] ?? [];

        print("📦 [PHOTON] Encontrados: ${features.length} resultados");

        List<Map<String, String>> results =
            features.map<Map<String, String>>((feature) {
          final props = feature['properties'] ?? {};
          final coords = feature['geometry']['coordinates'];

          // Construir endereço formatado
          List<String> parts = [];

          // 🏠 Nome da rua + número (SE EXISTIR)
          String streetPart = '';
          if (props['housenumber'] != null &&
              props['housenumber'].toString().isNotEmpty) {
            streetPart =
                '${props['name'] ?? props['street'] ?? ''} ${props['housenumber']}'
                    .trim();
          } else {
            streetPart =
                (props['name'] ?? props['street'] ?? '').toString().trim();
          }
          if (streetPart.isNotEmpty) parts.add(streetPart);

          // 🏘️ Bairro
          if (props['district'] != null &&
              props['district'].toString().isNotEmpty) {
            parts.add(props['district']);
          }

          // 🏙️ Cidade
          if (props['city'] != null && props['city'].toString().isNotEmpty) {
            parts.add(props['city']);
          } else if (props['county'] != null &&
              props['county'].toString().isNotEmpty) {
            parts.add(props['county']);
          }

          String displayName = parts.where((p) => p.isNotEmpty).join(', ');

          return {
            "name": displayName,
            "fullName": displayName,
            "lat": coords[1].toString(),
            "lon": coords[0].toString(),
          };
        }).toList();

        // 🔍 Se não encontrou nada e tem número, tenta sem o número
        if (results.isEmpty && RegExp(r'\d').hasMatch(query)) {
          print("🔄 [PHOTON] Tentando sem número...");
          String queryWithoutNumber = query
              .replaceAll(RegExp(r'\d+'), '')
              .replaceAll(RegExp(r',\s*,'), ',')
              .trim();
          return search(queryWithoutNumber, lat: lat, lon: lon, limit: limit);
        }

        return results;
      } else {
        print("❌ [PHOTON] Erro HTTP: ${res.statusCode}");
        return [];
      }
    } catch (e) {
      print("❌ [PHOTON] Erro: $e");
      return [];
    }
  }
}
