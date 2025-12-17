import 'dart:math' as math;

import 'package:flutter/material.dart';

/// Painter para HUD estilo automotivo com via curva e veículo triangular
class HUDPainter extends CustomPainter {
  final String direction; // 'L', 'R', 'S'
  final bool radarAlert;

  HUDPainter({
    required this.direction,
    this.radarAlert = false,
  });

  @override
  void paint(Canvas canvas, Size size) {
    final centerX = size.width / 2;
    final centerY = size.height / 2;

    // Desenhar via baseado na direção
    if (direction == 'L') {
      _drawLeftCurve(canvas, centerX, centerY);
    } else if (direction == 'R') {
      _drawRightCurve(canvas, centerX, centerY);
    } else {
      _drawStraightRoad(canvas, centerX, centerY);
    }

    // Desenhar veículo (triângulo azul fixo)
    _drawVehicle(canvas, centerX, centerY + 60);
  }

  /// Desenha via RETA
  void _drawStraightRoad(Canvas canvas, double centerX, double centerY) {
    const roadWidth = 100.0;
    const roadHeight = 200.0;

    // Fundo da via (cinza escuro)
    final roadPaint = Paint()
      ..color = Colors.grey[800]!
      ..style = PaintingStyle.fill;

    canvas.drawRect(
      Rect.fromCenter(
        center: Offset(centerX, centerY),
        width: roadWidth,
        height: roadHeight,
      ),
      roadPaint,
    );

    // Bordas brancas
    final linePaint = Paint()
      ..color = Colors.white
      ..strokeWidth = 3.0
      ..style = PaintingStyle.stroke;

    // Borda esquerda
    canvas.drawLine(
      Offset(centerX - roadWidth / 2, centerY - roadHeight / 2),
      Offset(centerX - roadWidth / 2, centerY + roadHeight / 2),
      linePaint,
    );

    // Borda direita
    canvas.drawLine(
      Offset(centerX + roadWidth / 2, centerY - roadHeight / 2),
      Offset(centerX + roadWidth / 2, centerY + roadHeight / 2),
      linePaint,
    );

    // Linha central tracejada
    final dashedPaint = Paint()
      ..color = Colors.yellow
      ..strokeWidth = 2.0;

    for (double y = centerY - roadHeight / 2;
        y < centerY + roadHeight / 2;
        y += 20) {
      canvas.drawLine(
        Offset(centerX, y),
        Offset(centerX, y + 10),
        dashedPaint,
      );
    }
  }

  /// Desenha via com CURVA À ESQUERDA (suave e fluida)
  void _drawLeftCurve(Canvas canvas, double centerX, double centerY) {
    const roadWidth = 100.0;
    const segments = 50;

    final roadPaint = Paint()
      ..color = Colors.grey[800]!
      ..style = PaintingStyle.fill;

    final linePaint = Paint()
      ..color = Colors.white
      ..strokeWidth = 3.0
      ..style = PaintingStyle.stroke;

    final dashedPaint = Paint()
      ..color = Colors.yellow
      ..strokeWidth = 2.0;

    // Desenhar via em segmentos curvos
    for (int i = 0; i < segments; i++) {
      final t = i / segments;
      final y = (t - 0.5) * 200 + centerY;

      // Curva suave à esquerda (parábola)
      final xOffset = -math.pow(t - 0.5, 2) * 150;

      final leftEdge = centerX - roadWidth / 2 + xOffset;
      final rightEdge = centerX + roadWidth / 2 + xOffset;

      // Desenhar segmento da via
      canvas.drawLine(
        Offset(leftEdge, y),
        Offset(rightEdge, y),
        roadPaint..strokeWidth = 4.0,
      );

      // Bordas brancas (espaçadas)
      if (i % 2 == 0) {
        canvas.drawCircle(
            Offset(leftEdge, y), 2.0, linePaint..style = PaintingStyle.fill);
        canvas.drawCircle(
            Offset(rightEdge, y), 2.0, linePaint..style = PaintingStyle.fill);
      }

      // Linha central tracejada
      if (i % 6 < 3 && i % 2 == 0) {
        canvas.drawCircle(Offset(centerX + xOffset, y), 1.5,
            dashedPaint..style = PaintingStyle.fill);
      }
    }
  }

  /// Desenha via com CURVA À DIREITA
  void _drawRightCurve(Canvas canvas, double centerX, double centerY) {
    const roadWidth = 100.0;
    const segments = 50;

    final roadPaint = Paint()
      ..color = Colors.grey[800]!
      ..style = PaintingStyle.fill;

    final linePaint = Paint()
      ..color = Colors.white
      ..strokeWidth = 3.0
      ..style = PaintingStyle.stroke;

    final dashedPaint = Paint()
      ..color = Colors.yellow
      ..strokeWidth = 2.0;

    for (int i = 0; i < segments; i++) {
      final t = i / segments;
      final y = (t - 0.5) * 200 + centerY;

      // Curva suave à direita
      final xOffset = math.pow(t - 0.5, 2) * 150;

      final leftEdge = centerX - roadWidth / 2 + xOffset;
      final rightEdge = centerX + roadWidth / 2 + xOffset;

      canvas.drawLine(
        Offset(leftEdge, y),
        Offset(rightEdge, y),
        roadPaint..strokeWidth = 4.0,
      );

      if (i % 2 == 0) {
        canvas.drawCircle(
            Offset(leftEdge, y), 2.0, linePaint..style = PaintingStyle.fill);
        canvas.drawCircle(
            Offset(rightEdge, y), 2.0, linePaint..style = PaintingStyle.fill);
      }

      if (i % 6 < 3 && i % 2 == 0) {
        canvas.drawCircle(Offset(centerX + xOffset, y), 1.5,
            dashedPaint..style = PaintingStyle.fill);
      }
    }
  }

  /// Desenha o veículo (triângulo azul sólido)
  void _drawVehicle(Canvas canvas, double centerX, double centerY) {
    final vehiclePaint = Paint()
      ..color = Colors.blue[700]!
      ..style = PaintingStyle.fill;

    const size = 20.0;

    // Triângulo apontando para cima
    final path = Path()
      ..moveTo(centerX, centerY - size) // Topo
      ..lineTo(centerX - size / 2, centerY + size / 2) // Base esquerda
      ..lineTo(centerX + size / 2, centerY + size / 2) // Base direita
      ..close();

    canvas.drawPath(path, vehiclePaint);

    // Contorno branco para destacar
    final outlinePaint = Paint()
      ..color = Colors.white
      ..strokeWidth = 2.0
      ..style = PaintingStyle.stroke;

    canvas.drawPath(path, outlinePaint);
  }

  @override
  bool shouldRepaint(covariant HUDPainter oldDelegate) {
    return oldDelegate.direction != direction ||
        oldDelegate.radarAlert != radarAlert;
  }
}
