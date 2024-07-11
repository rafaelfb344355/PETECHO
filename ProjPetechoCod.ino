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
int IR_PIN = D1;
#define LED_COUNT 8
#define BRIGHTNESS 90

#define MOTOR_VELOCIDADEMAXIMA 200
#define MOTOR_VELOCIDADEMIM 0
#define MOTOR_ACELERACAO 100
#define MOTOR_INTERVALO 5

#define CMD_POWER 0xFFA25D
#define CMD_AUMENTATEMPERATURA 0xFF02FD
#define CMD_DIMINUETEMPERATURA 0xFF9867
#define CMD_AUMENTAVELOCIDADE 0xFF906F
#define CMD_DIMINUEVELOCIDADE 0xFFE01F

#define CMD_REPETICAO 0xFFFFFFFFFFFFFFFF

float TEMP_MINIMA = 0.0;
float TEMP_MAXIMA = 300.0;
float INTERVALO_TEMP = 5.0;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(IR_PIN);
decode_results results;

SSD1306Wire display(0x3c, D5, D6);

float temperature = 0.0;
int motorSpeed = 0;
int heaterTemp = 200;
bool power = false;

unsigned long agora = 0;
const unsigned long IntervaloDisplay = 1000;  // 1 segundo para atualização do display
unsigned long ultimoDisplayUpdate = 0;
unsigned long ultimoTempoLeitura = 0;
const unsigned long intervaloLeituraTemp = 2000;  // 2 segundos para atualização da temperatura

AccelStepper myStepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
NTC_Thermistor* thermistor = new NTC_Thermistor(THERMISTOR_PIN, 10000, 100000, 25, 3950);

void setup() {
  Serial.begin(115200);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();

  myStepper.setMaxSpeed(MOTOR_VELOCIDADEMAXIMA);
  myStepper.setAcceleration(MOTOR_ACELERACAO);

  pinMode(MOSFET_GATE_PIN, OUTPUT);
  irrecv.enableIRIn();

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

  agora = millis();

  if (agora - ultimoDisplayUpdate >= IntervaloDisplay) {
    displayMostraEstado();
    ultimoDisplayUpdate = agora;
  }

  if (agora - ultimoTempoLeitura >= intervaloLeituraTemp) {
    atuadorTemperatura();
    ultimoTempoLeitura = agora;
  }

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
  display.drawString(90, 0, (power ? "on" : "off"));
  display.drawString(0, 20, "Temp: ");
  display.drawString(45, 20, String(temperature, 0));
  display.drawString(75, 20, String(heaterTemp));
  display.drawString(0, 40, "Vel: ");
  display.drawString(30, 40, String(motorSpeed));
  display.display();
}

void mudarEstadoComando() {
  switch (results.value) {
    case CMD_REPETICAO:
      break;
    case CMD_AUMENTATEMPERATURA:
      heaterTemp += INTERVALO_TEMP;
      if (heaterTemp >= TEMP_MAXIMA) heaterTemp = TEMP_MAXIMA;
      break;
    case CMD_DIMINUETEMPERATURA:
      heaterTemp -= INTERVALO_TEMP;
      if (heaterTemp <= TEMP_MINIMA) heaterTemp = TEMP_MINIMA;
      break;
    case CMD_AUMENTAVELOCIDADE:
      motorSpeed += MOTOR_INTERVALO;
      if (motorSpeed >= MOTOR_VELOCIDADEMAXIMA) motorSpeed = MOTOR_VELOCIDADEMAXIMA;
      break;
    case CMD_DIMINUEVELOCIDADE:
      motorSpeed -= MOTOR_INTERVALO;
      if (motorSpeed <= MOTOR_VELOCIDADEMIM) motorSpeed = MOTOR_VELOCIDADEMIM;
      break;
    case CMD_POWER:
      power = !power;
      break;
  }
}

void atuadorTemperatura() {
  if (power) {
    if (temperature < heaterTemp) {
      digitalWrite(MOSFET_GATE_PIN, HIGH);
    } else {
      digitalWrite(MOSFET_GATE_PIN, LOW);
    }
  } else {
    digitalWrite(MOSFET_GATE_PIN, LOW);
  }
}

void atuadorMotor() {
 if (power) {
  myStepper.setSpeed(motorSpeed);
  myStepper.runSpeed();  
 }
}

bool temperaturaLeitura() {
  temperature = thermistor->readCelsius();
  if (temperature > -40 && temperature < 125) {
    return true;
  } else {
    return false;
  }
}
