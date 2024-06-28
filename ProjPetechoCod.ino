/* 
  Faculdade SENAC PE
  Análise de Sistemas

  Projeto em Parceria com Lab de Inovação da ACCENTURE
  Autoria: alunos e professores voluntários da Faculdade SENAC
            Rafael (autor principal)
            Arnott Ramos Caiado (professor - orientador)
            Sérgio Cahu (orientador)

  Data: 27-06-2024
*/

//  0xFFE21D: // raio
//  0xFFA25D: // Power
//  0xFF02FD: // Cima
//  0xFF9867: // Baixo
//  FFA857: // centro
//  0xFF906F: // Direita
//  0xFFE01F: // Esquerda
//  0xFF30CF: // A
//  0xFF18E7: // B
//  0xFF7A85: // C
//  0xFF10EF: // D
//  0xFF38C7: // E
//  0xFF5AA5: // F
//  0xFF42BD: // G
//  0xFF4AB5: // H
//  0xFF52AD: // I

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <AccelStepper.h>
#include <NTC_Thermistor.h>

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <Adafruit_NeoPixel.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define DIR_PIN D7
#define STEP_PIN D8
#define MOSFET_GATE_PIN D0  // Pino do Arduino conectado ao MOSFET
#define THERMISTOR_PIN A0   // Pino analógico do Arduino conectado ao termistor
#define LED_PIN D2
const int pinoBotao = D3;
#define LED_COUNT 8
#define BRIGHTNESS 90

#define MOTOR_VELOCIDADEMAXIMA 200
#define MOTOR_VELOCIDADEMIM 0
#define MOTOR_ACELERACAO 100
#define MOTOR_INTERVALO 5


#define CMD_AUMENTATEMPERATURA 0xFF02FD
#define CMD_DIMINUETEMPERATURA 0xFF9867
#define CMD_AUMENTAVELOCIDADE 0xFF906F
#define CMD_DIMINUEVELOCIDADE 0xFFE01F

#define CMD_REPETICAO 0xFFFFFFFF

float TEMP_MINIMA = 0.0;
float TEMP_MAXIMA = 300.0;
float INTERVALO_TEMP = 5.0;

int IR_PIN = D1;  // Pino IN para leitura do controle remoto

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(IR_PIN);
decode_results results;

SSD1306Wire display(0x3c, D5, D6);

float temperature = 0.0;
int motorSpeed = 0;
int heaterTemp = 100;
bool power = false;

unsigned long agora = 0;
const unsigned long IntervaloDisplay = 1000;  // 3 segundos para atualização do display
unsigned long ultimoDisplayUpdate = 0;
// Variáveis para armazenar o estado anterior dos botões

unsigned long ultimoTempo = 0;
const unsigned long intervalo = 500;

bool botaoPressionadoAnterior = false;

int brightness = BRIGHTNESS;



AccelStepper myStepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

NTC_Thermistor* thermistor = new NTC_Thermistor(THERMISTOR_PIN, 10000, 100000, 25, 3950);



bool temperaturaLeitura();
bool remotoLeitura();
void displayMostraEstado();
void ledsAcendeEstado();
void mudarEstadoComando();
void atuadorTemperatura();
void atuadorMotor();

void setup() {
  // Inicializar variáveis
  Serial.begin(115200);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);

  strip.begin();
  strip.setBrightness(brightness);
  strip.show();

  myStepper.setMaxSpeed(MOTOR_VELOCIDADEMAXIMA);  // Velocidade máxima do motor
  myStepper.setAcceleration(MOTOR_ACELERACAO);    // Aceleração do motor
  pinMode(MOSFET_GATE_PIN, OUTPUT);
  // pinMode(pinoBotao, INPUT);
  irrecv.enableIRIn();

  // Chamar função para ligar LEDs e mostrar estado atual
  ledsAcendeEstado();
  displayMostraEstado();
}

void loop() {
  temperaturaLeitura();
  if (irrecv.decode(&results)) {
    Serial.print("Received IR code: ");
    Serial.println(results.value, HEX);
    mudarEstadoComando();
    irrecv.resume();
  }


  unsigned long agora = millis();

  // Atualizações do display a cada 3 segundos
  if (agora - ultimoDisplayUpdate >= IntervaloDisplay) {
    displayMostraEstado();
    ultimoDisplayUpdate = agora;
  }

  // paradaAltomatica();

  ledsAcendeEstado();

  atuadorTemperatura();

  atuadorMotor();
}


void ledsAcendeEstado() {

  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(161, 0, 255));  // Vermelho
  }
  strip.show();
}

void displayMostraEstado() {


  display.clear();
  display.drawString(0, 0, "Petecho");
  display.drawString(90, 0, (power ? "on" : " off"));
  display.drawString(0, 20, "Temp: ");
  display.drawString(45, 20, String(temperature, 0));
  display.drawString(75, 20, String(heaterTemp));

  display.drawString(0, 40, "Vel: ");
  display.drawString(30, 40, String(motorSpeed));
  display.display();
}

// void paradaAltomatica() {
//   // Verifica se o intervalo de tempo definido passou
//   if (millis() - ultimoTempo >= intervalo) {
//     ultimoTempo = millis();  // Atualiza o último tempo

//     // Lê o estado atual do botão
//     bool botaoPressionado = digitalRead(pinoBotao);

//     // Verifica se o botão foi pressionado e está agora sendo liberado
//     if (!botaoPressionado && botaoPressionadoAnterior) {
//       // Se a variável power está true, então muda para false
//       if (power) {
//         power = false;  // Inverte o estado da variável
//         Serial.print("Botão liberado! Variável power = ");
//         Serial.println(power);
//       }
//     }

//     // Atualiza o estado anterior do botão para o próximo ciclo
//     botaoPressionadoAnterior = botaoPressionado;
//   }
// }



// =========================================================================================
void mudarEstadoComando() {


  switch (results.value) {

    case CMD_REPETICAO:
      break;

    case CMD_AUMENTATEMPERATURA:

      heaterTemp += INTERVALO_TEMP;
      if (heaterTemp >= TEMP_MAXIMA)
        heaterTemp = TEMP_MAXIMA;
      break;
    case CMD_DIMINUETEMPERATURA:

      heaterTemp -= INTERVALO_TEMP;
      if (heaterTemp <= TEMP_MINIMA)
        heaterTemp = TEMP_MINIMA;
      break;
    case CMD_AUMENTAVELOCIDADE:
      motorSpeed += MOTOR_INTERVALO;
      if (motorSpeed >= MOTOR_VELOCIDADEMAXIMA)
        motorSpeed = MOTOR_VELOCIDADEMAXIMA;
      break;
    case CMD_DIMINUEVELOCIDADE:

      motorSpeed -= MOTOR_INTERVALO;
      if (motorSpeed <= MOTOR_VELOCIDADEMIM)
        motorSpeed = MOTOR_VELOCIDADEMIM;
      break;

    case 0xFFA25D:
      if (power == false) {
        power = true;
      } else {
        power = false;
      }
    default:
      break;
  }
}

// =============================================================================================
void atuadorTemperatura() {
  // Implementação necessária
  if (power == true) {
    temperature = thermistor->readCelsius();
    if (temperature < heaterTemp) {
      digitalWrite(MOSFET_GATE_PIN, HIGH);

    } else {
      digitalWrite(MOSFET_GATE_PIN, LOW);
    }
  }
}

// =============================================================================================
void atuadorMotor() {
  // Implementação necessária

  if (power == true) {
    myStepper.setSpeed(motorSpeed);

    myStepper.runSpeed();  // Controla a velocidade do motor continuamente
  }
}

// =============================================================================================
bool temperaturaLeitura() {
  // Aqui você deve colocar o código para ler a temperatura do termistor.
  // Este é um exemplo básico, você pode ajustá-lo conforme a necessidade do seu projeto.

  temperature = thermistor->readCelsius();


  // Exemplo de lógica simples para determinar se a leitura foi bem-sucedida
  if (temperature > -40 && temperature < 125) {  // Faixa típica de um termistor NTC
    return true;
  } else {
    return false;
  }
}