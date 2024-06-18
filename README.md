# Projeto de Controle de Motor e Aquecedor com ESP8266

Este projeto utiliza um ESP8266 para controlar um motor de passo e um aquecedor, além de exibir informações em um display OLED e fornecer uma interface web para monitoramento e controle.

## Descrição

Este projeto consiste em um sistema de controle que utiliza um ESP8266 para gerenciar um motor de passo e um aquecedor. O sistema é capaz de:
- Controlar a velocidade do motor de passo.
- Monitorar e controlar a temperatura do aquecedor.
- Exibir informações no display OLED.
- Fornecer uma interface web para monitoramento e controle remoto.

## Componentes Utilizados

- ESP8266
- Motor de passo
- Driver de motor de passo
- MOSFET
- Termistor NTC
- Display OLED (SSD1306)
- Botões de controle

## Dependências

As seguintes bibliotecas são utilizadas neste projeto:

- [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)
- [ESP8266WebServer](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer)
- [Wire](https://www.arduino.cc/en/Reference/Wire)
- [SSD1306Wire](https://github.com/ThingPulse/esp8266-oled-ssd1306)
- [AccelStepper](https://www.airspayce.com/mikem/arduino/AccelStepper/)
- [NTC_Thermistor](https://github.com/RobTillaart/Arduino/tree/master/libraries/NTC_Thermistor)
- [ESP8266HTTPClient](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

## Instalação

1. Clone este repositório:
   ```bash
   git clone https://github.com/rafaelfb344355/PETECO.git
* Abra o projeto no Arduino IDE.
* Instale as bibliotecas necessárias via Library Manager ou usando o gerenciador de bibliotecas do Arduino IDE.
2. Configure seu SSID e senha de Wi-Fi no código:

* const char* ssid = "Nome_Wifi";
* const char* password = "Senha";
* Compile e envie o código para o seu ESP8266.

## Uso
1. Controle via Botões
- Botão Up (D1): Navega para cima no menu ou aumenta o valor selecionado.
- Botão Down (D2): Navega para baixo no menu ou diminui o valor selecionado.
- Botão Left (D3): Seleciona a opção ou alterna o estado do motor/aquecedor.
- Botão Right (D4): Exibe o endereço IP ou retorna ao menu principal.
2. Interface Web
- Conecte-se à rede Wi-Fi configurada.
- Acesse a interface web através do endereço IP do ESP8266.
- Utilize os controles na interface para monitorar e controlar o motor e o aquecedor.

### Explicação

1. **Descrição**: Uma visão geral do projeto.
2. **Componentes Utilizados**: Lista dos componentes de hardware usados no projeto.
3. **Dependências**: Bibliotecas necessárias para o projeto, com links para suas páginas.
4. **Instalação**: Instruções detalhadas para clonar o repositório, abrir o projeto, instalar dependências e configurar a rede Wi-Fi.
5. **Uso**: Explicação sobre como usar os botões e a interface web para controlar o sistema.


