Moto HUD

Open Source Motorcycle Head-Up Display

Um sistema de navegação HUD (Head-Up Display) para motocicletas utilizando um smartphone Android e um ESP32.

O objetivo é fornecer informações essenciais de navegação diretamente no campo de visão do piloto, reduzindo o tempo em que ele precisa desviar os olhos para o painel ou celular.

Projeto experimental e colaborativo.

Visão Geral
O sistema é dividido em duas partes:
Aplicativo Android (Flutter)
Navegação baseada em GPS
Cálculo de rotas via OSRM
Comunicação Bluetooth Low Energy (BLE)
Interface otimizada para HUD
Envio apenas das informações necessárias ao capacete
Hardware
ESP32
Display TFT 135x240
Comunicação BLE
Baixo consumo
Atualização em tempo real
Fluxo do sistema
Plain text
GPS
   │
Flutter
   │
OSRM
   │
BLE
   │
ESP32
   │
Display TFT
   │
Conjunto óptico
   │
Olho do piloto
Tecnologias
Flutter
Dart
ESP32
Bluetooth Low Energy
OSRM
GPS Android
TFT 135x240
Objetivos
Navegação segura
Hardware de baixo custo
Código aberto
Fácil reprodução
Plataforma para pesquisas futuras
Próxima etapa
Atualmente o projeto está funcional na parte eletrônica e de software.

O próximo desenvolvimento será o módulo óptico, responsável por transformar a pequena tela em uma imagem virtual confortável para visualização.

Os estudos incluem:
lentes asféricas
espelho semirreflexivo (combiner)
prismas
ajuste focal
distância virtual entre 2 e 5 metros
Esta etapa permitirá uma experiência muito próxima de um HUD comercial.
Ideias futuras
Velocidade da motocicleta via OBD-II
Aviso de radares
Alertas de chuva
Chamadas e notificações inteligentes
Controle por voz
Integração com capacetes inteligentes
Reconhecimento de placas
IA para previsão de riscos
Atualizações OTA do ESP32

Colaboração

Este projeto foi publicado como Open Source para incentivar colaboração.
Toda melhoria é bem-vinda:
Pull Requests
Issues
Discussões
Ideias
Testes em campo
Aviso

Este projeto é experimental.
Não substitui os equipamentos de segurança nem deve ser utilizado de forma que comprometa a atenção do piloto.

Licença
MIT
