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
#define LED_PIN D2          // Pino OUT para barra de LEDs RGB
#define LED_COUNT 8
#define BRIGHTNESS 50

#define MOTOR_VELOCIDADEMAXIMA 200
#define MOTOR_VELOCIDADEMIM 0
#define MOTOR_ACELERACAO 100
#define MOTOR_INTERVALO 5


#define CMD_AUMENTATEMPERATURA  0xFF02FD
#define CMD_DIMINUETEMPERATURA  0xFF9867
#define CMD_AUMENTAVELOCIDADE   0xFF906F
#define CMD_DIMINUEVELOCIDADE   0xFFE01F

#define CMD_REPETICAO 0xFFFFFFFF

float TEMP_MINIMA = 0.0;
float TEMP_MAXIMA = 250.0;
float INTERVALO_TEMP = 5.0;

int IR_PIN = D1;  // Pino IN para leitura do controle remoto

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(IR_PIN);
decode_results results;

SSD1306Wire display(0x3c, D5, D6);

int menuLevel = 0;  // Nível do menu
int opcaoLevel = 0;

bool motorOn = false;
bool heaterOn = false;
float temperature = 0.0;
int motorSpeed = 0;
int heaterTemp = 0;

unsigned long lastDataSentTime = 0;
const unsigned long dataSendInterval = 10000;
// Variáveis para armazenar o estado anterior dos botões

uint32_t colors[] = { 
  strip.Color(255, 0, 0), 
  strip.Color(0, 255, 0), 
  strip.Color(0, 0, 255), 
  strip.Color(255, 255, 0), 
  strip.Color(0, 255, 255), 
  strip.Color(255, 0, 255), 
  strip.Color(255, 255, 255), 
  strip.Color(127, 127, 127), 
  strip.Color(0, 0, 0) 
};
int numLeds = 1;
int brightness = BRIGHTNESS;
bool power = false;
bool controller = false;

const char* ssid = "Rafael";
const char* password = "rafaelfb344355";

AccelStepper myStepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

NTC_Thermistor* thermistor = new NTC_Thermistor(THERMISTOR_PIN, 10000, 100000, 25, 3950);

int rightButtonPressCount = 0;

bool temperaturaLeitura();
bool remotoLeitura();
void apiLeitura();
void servidorLocalLeitura();
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
  myStepper.setAcceleration(MOTOR_ACELERACAO);  // Aceleração do motor

  irrecv.enableIRIn();

  // Chamar função para ligar LEDs e mostrar estado atual
  ledsAcendeEstado();
  displayMostraEstado();
}

void loop() {
  //temperaturaLeitura();
 if (irrecv.decode(&results)) {
    Serial.print("Received IR code: ");
    Serial.println(results.value, HEX);
    mudarEstadoComando();
    irrecv.resume();
 }
  apiLeitura();

  servidorLocalLeitura();

  displayMostraEstado();
   
  ledsAcendeEstado();


  atuadorTemperatura();

  atuadorMotor();

  delay(500);
}


void ledsAcendeEstado() {
  // Implementação necessária
}

void displayMostraEstado() {
  

  display.clear(); 
  display.drawString(0, 0, "PETECHO"); 
  display.drawString(90, 0, (heaterOn ? "on" : " off"));
  display.drawString(0, 20,String(temperature) ); 
  display.drawString(0, 40, String(heaterTemp)); 
  display.display();
}  

void apiLeitura() {
  // Implementação necessária
}

void servidorLocalLeitura() {
  // Implementação necessária
}

// =========================================================================================
void mudarEstadoComando() {
  // Implementação necessária
  
  switch ( results.value ){

    case CMD_REPETICAO :
      break;

      case CMD_AUMENTATEMPERATURA :
      Serial.println("temp+");
        heaterTemp += INTERVALO_TEMP;
        if ( heaterTemp >= TEMP_MAXIMA)
          heaterTemp = TEMP_MAXIMA;
        break;
      case  CMD_DIMINUETEMPERATURA :
      Serial.println("temp-");
        heaterTemp -= INTERVALO_TEMP;
        if ( heaterTemp <= TEMP_MINIMA )
          heaterTemp = TEMP_MINIMA;
        break;
      case CMD_AUMENTAVELOCIDADE :
              motorSpeed += MOTOR_INTERVALO;
        if ( motorSpeed >= MOTOR_VELOCIDADEMAXIMA)
          motorSpeed= MOTOR_VELOCIDADEMAXIMA;
        break;
      case CMD_DIMINUEVELOCIDADE :

       motorSpeed -= MOTOR_INTERVALO;
        if ( motorSpeed <= MOTOR_VELOCIDADEMIM)
          motorSpeed = MOTOR_VELOCIDADEMIM;
        break;

      case 0xFFA25D :
      if(heaterOn == false){
        heaterOn= true;
      }else{
        heaterOn=false;
      }
      default :
        break;

  }
 
}

// =============================================================================================
void atuadorTemperatura() {
  // Implementação necessária
 
  temperature = thermistor->readCelsius();
  if (temperature < heaterTemp) {
    digitalWrite(MOSFET_GATE_PIN, HIGH); // Ligar o aquecedor
  } else {
    digitalWrite(MOSFET_GATE_PIN, LOW); // Desligar o aquecedor
  }

}

// =============================================================================================
void atuadorMotor() {
  // Implementação necessária
 
    Serial.println(motorSpeed);
    myStepper.setSpeed(motorSpeed); 

    myStepper.runSpeed(); // Controla a velocidade do motor continuamente 

  

}

// =============================================================================================
bool temperaturaLeitura() {
  // Aqui você deve colocar o código para ler a temperatura do termistor.
  // Este é um exemplo básico, você pode ajustá-lo conforme a necessidade do seu projeto.

  temperature = thermistor->readCelsius();
  Serial.print("Temperatura: ");
  Serial.println(temperature);

  // Exemplo de lógica simples para determinar se a leitura foi bem-sucedida
  if (temperature > -40 && temperature < 125) { // Faixa típica de um termistor NTC
    return true;
  } else {
    return false;
  }
}