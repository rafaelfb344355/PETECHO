
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

#define DIR_PIN  D7 

#define STEP_PIN D8 

#define MOSFET_GATE_PIN D0 // Pino do Arduino conectado ao MOSFET

#define THERMISTOR_PIN A0 // Pino analógico do Arduino conectado ao termistor

#define LED_PIN     D2

#define LED_COUNT   8

#define BRIGHTNESS  50

int IR_PIN  =  D1;


Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(IR_PIN);
decode_results results;

 

SSD1306Wire display(0x3c, D5, D6);

 

int menuLevel = 0; // Nível do menu 

int opcaoLevel = 0; 



bool motorOn = false; 

bool heaterOn = false; 

float temperature = 0.0;

int motorSpeed = 0; 

int heaterTemp = 0; 

 
unsigned long lastDataSentTime = 0;
const unsigned long dataSendInterval = 10000;
// Variáveis para armazenar o estado anterior dos botões 

uint32_t colors[] = {strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255), strip.Color(255, 255, 0), strip.Color(0, 255, 255), strip.Color(255, 0, 255), strip.Color(255, 255, 255), strip.Color(127, 127, 127), strip.Color(0, 0, 0)};
int numLeds = 1;
int brightness = BRIGHTNESS;
bool power = false;
bool contoler = false;
const char* ssid = "Rafael";
const char* password = "rafaelfb344355";

ESP8266WebServer server(80);

AccelStepper myStepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN); 



NTC_Thermistor* thermistor = new NTC_Thermistor(THERMISTOR_PIN, 10000, 100000, 25, 3950);


int rightButtonPressCount = 0;

void setup() { 

  Serial.begin(9600); 
 
  display.init(); 

  display.flipScreenVertically(); 

  display.setFont(ArialMT_Plain_16); 

 
  strip.begin();
  strip.setBrightness(brightness);
  strip.show();
  irrecv.enableIRIn();

  myStepper.setMaxSpeed(1000); // Velocidade máxima do motor 

  myStepper.setAcceleration(100); // Aceleração do motor 

   WiFi.begin(ssid, password); // Conecta ao Wi-Fi

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  // Definir rotas do servidor web
  server.on("/", HTTP_GET, handleRoot);
  server.on("/state", HTTP_OPTIONS, [](){
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200, "text/plain", "");
  });
  server.on("/data", HTTP_GET, [](){
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["speed"] = motorSpeed;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  server.on("/state", HTTP_POST, handleState);
  server.on("/pagina2", HTTP_GET, handlePagina2);
  // Iniciar servidor web
  server.begin();
}
  


void loop() { 

  server.handleClient(); 
  
 unsigned long currentMillis = millis();
  if (currentMillis - lastDataSentTime >= dataSendInterval) {
    // Chama a função que envia os dados atuais (temperatura e velocidade)
    sendTemperatureAndSpeed();
    lastDataSentTime = currentMillis; // Reinicia o temporizador
  }

if (irrecv.decode(&results)) {
    Serial.print("Received IR code: ");
    Serial.println(results.value, HEX);
    switch (results.value) {
      case 0xFFE21D:
      if (contoler==false){
        contoler = true;
      }else 
      contoler = false;
      Serial.println(contoler);
      break;
      case 0xFFA25D: // Power
     
        power = !power;
        Serial.println(power ? "Power ON" : "Power OFF");
        break;
      case 0xFF02FD: // Cima
      
      if(contoler==true){
        //menu
         handleButtonUp(); 
         Serial.print("cima");
      }else{
        numLeds = min(numLeds + 1, LED_COUNT);
        Serial.print("Number of LEDs: ");
        Serial.println(numLeds);
      }
        break;
      case 0xFF9867: // Baixo
        
if(contoler==true){
        handleButtonDown(); 
         Serial.print("baixo ");
}else{
         
        numLeds = max(numLeds - 1, 1);
        Serial.print("Number of LEDs: ");
        Serial.println(numLeds);
}
        break;
      case 0xFF906F: // Direita
        
if(contoler==true){
        if (menuLevel == 0) {
      rightButtonPressCount++;
      if (rightButtonPressCount == 2) {
        // Mostra o endereço IP no LCD por 30 segundos
        display.clear();
        display.drawString(0, 0, "IP Address:");
        display.drawString(0, 20, WiFi.localIP().toString());
        display.display();
        delay(30000); // Aguarda 30 segundos

        // Limpa o display e volta para o menuLevel 0
        display.clear();
        display.display();
        rightButtonPressCount = 0;
      }
    } else {
      handleButtonRight();
    }
      }else{

brightness = min(brightness + 10, 255);
        strip.setBrightness(brightness);
        Serial.print("Brightness: ");
        Serial.println(brightness);
      }
        break;
      case 0xFFE01F: // Esquerda
        
if(contoler==true){
     handleButtonLeft(); 
     Serial.print("esquerda ");
}else{
brightness = max(brightness - 10, 0);
        strip.setBrightness(brightness);
        Serial.print("Brightness: ");
        Serial.println(brightness);
}
        break;
      case 0xFF30CF: // A
      case 0xFF18E7: // B
      case 0xFF7A85: // C
      case 0xFF10EF: // D
      case 0xFF38C7: // E
      case 0xFF5AA5: // F
      case 0xFF42BD: // G
      case 0xFF4AB5: // H
      case 0xFF52AD: // I
        for (int i = 0; i < numLeds; i++) {
          strip.setPixelColor(i, colors[(results.value & 0xFF) % 9]);
        }
        Serial.print("Set color: ");
        Serial.println(colors[(results.value & 0xFF) % 9], HEX);
        break;
    }
    if (power) {
      strip.show();
    } else {
      strip.clear();
      strip.show();
    }
    irrecv.resume();
  }

  // Controle do motor de passo 

  if (motorOn) { 

    myStepper.setSpeed(motorSpeed); 

    myStepper.runSpeed(); // Controla a velocidade do motor continuamente 

  } 

if(heaterOn){
  temperature = thermistor->readCelsius();
  Serial.println(temperature);
  if (temperature < heaterTemp) {
    digitalWrite(MOSFET_GATE_PIN, HIGH); // Ligar o aquecedor
  } else {
    digitalWrite(MOSFET_GATE_PIN, LOW); // Desligar o aquecedor
  }
}
 

  // Desenhar o menu 

  display.clear(); 

  switch (menuLevel) { 

    case 0: // Menu principal 

      display.drawString(0, 0, "Menu:"); 

      display.drawString(0, 20, (opcaoLevel == 0 ? "> Motor" : "Motor")); 

      display.drawString(0, 40, (opcaoLevel == 1 ? "> Aquecedor" : "Aquecedor")); 

      break; 

    case 1: // Submenu Motor 

      display.drawString(0, 0, "Motor:"); 

      display.drawString(0, 20, (motorOn ? "> Desligar" : "> Ligar")); 

      display.drawString(0, 40, (opcaoLevel == 1 ? "> Velocidade" : "Velocidade")); 

      break; 

    case 2: // Submenu Velocidade 

      display.drawString(0, 0, "Velocidade:"); 

      display.drawString(0, 20, String(motorSpeed)); 

      break; 

    case 3: // Submenu Aquecedor 

      display.drawString(0, 0, "Aquecedor:"); 

      display.drawString(0, 20, (heaterOn ? "> Desligar" : "> Ligar")); 

      display.drawString(0, 40, (opcaoLevel == 1 ? "> Temperatura" : "Temperatura")); 

      break; 

    case 4: // Submenu Temperatura 

      display.drawString(0, 0, "Temperatura:"); 

      display.drawString(0, 20, String(heaterTemp)); 

      break; 

  } 

  display.display(); 

} 

void handleRoot() {
  // Página principal servida pelo ESP8266
  String html = R"(
    <!DOCTYPE html>
    <html lang="pt-br">
    <head>
      <meta name="theme-color" content="#317EFB">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body {
    font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif;
    background-color: #00A1E0;
    color: #ffffff;
    padding: 20px !important;
  }
  
  h1 {
    color: #A100FF;
    text-align: center;
  }
  .contaner {
margin-top: 20px;
  }
  form {
    display: flex;
    flex-direction: column;
    max-width: 600px;
    margin: 0 auto;
    background-color: #ffffff;
    color: #000000;
    padding: 20px;
    border-radius: 10px;
    box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1);
  }
  
  label {
    margin-top: 20px;
    font-weight: bold;
  }
  
  input[type="number"],
  select {
    height: 30px !important;
    margin-top: 5px;
    border-radius: 5px;
    border: 1px solid #A100FF;
    padding: 5px;
  }
  
  input[type="submit"] {
    margin-top: 20px !important;
    background-color: #A100FF;
    color: #ffffff;
    border: none;
    padding: 10px 20px !important;
    cursor: pointer;
    border-radius: 5px;
    transition: background-color 0.3s ease;
  }
  
  input[type="submit"]:hover {
    background-color: #7A00CC;
  }
  
  #response {
    margin-top: 40px;
    white-space: pre-wrap;
    font-size: 16px;
    background-color: #ffffff;
    color: #000000;
    padding: 20px;
    border-radius: 10px;
    box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1);
  }

  button{
    margin-top: 20px !important;
    background-color: #A100FF;
    color: #ffffff;
    border: none;
    padding: 10px 20px !important;
    cursor: pointer;
    border-radius: 5px;
    transition: background-color 0.3s ease;
  }
   
      </style>
      <title>Teste da API</title>
    </head>
    <body>
      <div class="container">
        <h1>Teste da API</h1>
        <form id="stateForm">
          <label for="motorOn"><i class="fas fa-gear"></i> Motor :</label>
          <input type="checkbox" id="motorOn" name="motorOn"><br>
          <label for="motorSpeed"><i class="fas fa-tachometer-alt-fast"></i> Velocidade:</label><br>
          <input type="number" id="motorSpeed" name="motorSpeed"><br>
          <label for="heaterOn"><i class="fas fa-heaterTemp-high"></i> Aquecedor:</label>
          <input type="checkbox" id="heaterOn" name="heaterOn"><br>
          <label for="heaterTemp"><i class="fas fa-thermometer-half"></i> Temperatura:</label><br>
          <input type="number" id="heaterTemp" name="heaterTemp"><br>
          <input type="submit" value="Enviar">
        </form>
        <div id="response"></div>
        <div>
          <a href="/pagina2">Monitorar</a>
        </div>
      </div>
      <script>
        document.getElementById('stateForm').addEventListener('submit', function(event) {
          event.preventDefault();
         
          var state = {
            motorOn: document.getElementById('motorOn').checked,
            motorSpeed: document.getElementById('motorSpeed').value,
            heaterOn: document.getElementById('heaterOn').checked,
            heaterTemp: document.getElementById('heaterTemp').value
          };
          
          var ip = location.hostname; // Obtém o IP do ESP8266
          fetch('http://' + ip + '/state', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json',
            },
            body: JSON.stringify(state),
          })
          .then(response => response.text())
          .then(data => {
            document.getElementById('response').innerText = data;
          })
          .catch((error) => {
            console.error('Erro:', error);
          });
        });
      </script>
    </body>
    </html>
  )";
  server.send(200, "text/html", html);
}


void handlePagina2() {
  String html = R"(
   <!DOCTYPE html>
<html lang="pt-br">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Monitoramento de Temperatura e Velocidade</title>

  <style>
    body {
      font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif;
    background-color: #00A1E0;
    color: #ffffff;
    padding: 20px !important;
    }
    h1 {
      color: #3D3D3D;
      text-align: center;
    }
    .container {
      max-width: 800px;
      margin: 0 auto;
      padding: 20px;
    }
    .card {
      margin-bottom: 20px;
    }
    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      background-color: #A460D0;
      padding: 10px;
      border-radius: 5px 5px 0 0; /* Arredondando os cantos superiores */
    }
    .card-header i {
      font-size: 1.5rem;
      margin-right: 10px;
    }
    .card-title {
      font-size: 1.5rem;
      margin-bottom: 0;
      color: #fff; /* Cor do texto do título */
    }
    .card-body {
      background-color: #A100FF;
      padding: 20px;
    }
    .data-value {
      font-size: 3rem;
      font-weight: bold;
    }
    .data-icon {
      font-size: 2rem;
      color: #fff;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Monitoramento de Temperatura e Velocidade</h1>
    <div class="row">
      <div class="col-md-6">
        <div class="card text-white bg-primary">
          <div class="card-header">
            <i class="fas fa-thermometer-half data-icon"></i>
            <span class="card-title">Temperatura</span>
          </div>
          <div class="card-body">
            <p class="data-value" id="temperature">-- °C</p>
          </div>
        </div>
      </div>
      <div class="col-md-6">
        <div class="card text-white bg-success">
          <div class="card-header">
            <i class="fas fa-tachometer-alt data-icon"></i>
            <span class="card-title">Velocidade</span>
          </div>
          <div class="card-body">
            <p class="data-value" id="speed">--</p>
          </div>
        </div>
      </div>
    </div>
  </div>

  <script>
    document.addEventListener('DOMContentLoaded', function() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          document.getElementById('temperature').innerText = data.temperature.toFixed(2) + ' °C';
          document.getElementById('speed').innerText = data.speed;
        })
        .catch(error => {
          console.error('Erro ao obter dados:', error);
        });
    });
  </script>
</body>
</html>
  )";
  server.send(200, "text/html", html);
}



void handleState() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (server.method() == HTTP_POST) {
    // Lê o corpo da requisição POST
    String jsonStr = server.arg("plain");

    // Alocar um buffer para o documento JSON
    DynamicJsonDocument doc(1024); // Tamanho do buffer ajustável conforme necessário

    // Deserializar o JSON
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error) {
      Serial.print("deserializeJson() falhou: ");
      Serial.println(error.f_str());
    } else {
      // Ler os dados do JSON e converter conforme necessário
      motorOn  = doc["motorOn"];
      
      heaterOn = doc["heaterOn"];
    
      motorSpeed  = doc["motorSpeed"];

      heaterTemp = doc["heaterTemp"];
      
    }
  }

  // Montagem da mensagem de resposta
  String message = "motor: " + String(motorOn) + "\n"
                 + "speed: " + String(motorSpeed) + "\n"
                 + "heater: " + String(heaterOn) + "\n"
                 + "temperature: " + String(heaterTemp);
  server.send(200, "text/plain", message);
}

void sendTemperatureAndSpeed() {
  // Cria um objeto JSON com os dados atuais de temperatura e velocidade
  DynamicJsonDocument doc(256);
  doc["temperature"] = thermistor->readCelsius(); // Leitura da temperatura atual
  doc["speed"] = motorSpeed; // Velocidade atual do motor

  // Serializa o JSON para uma string
  String output;
  serializeJson(doc, output);

  // Envia a resposta JSON para o cliente
  server.send(200, "application/json", output);
}
void handleButtonUp() { 
  if (menuLevel == 0) { 

    opcaoLevel = (opcaoLevel + 1) % 2; // Alternar entre as opções no Menu Principal 

  } else if (menuLevel == 1) { 

    opcaoLevel = (opcaoLevel + 1) % 2; // Alternar entre as opções no Submenu Motor 

  } else if (menuLevel == 2) { 

    motorSpeed += 25; // Aumentar a velocidade do motor 

    if (motorSpeed > 1000) motorSpeed = 1000; // Limitar a velocidade máxima 

  } else if (menuLevel == 3) { 

    opcaoLevel = (opcaoLevel + 1) % 2; // Alternar entre as opções no Submenu Aquecedor 

  } else if (menuLevel == 4) { 

    heaterTemp += 10; // Aumentar a temperatura do aquecedor 

    if (heaterTemp > 300) heaterTemp = 300; // Limitar a temperatura máxima 

  } 

} 

 

void handleButtonDown() { 


  if (menuLevel == 0) { 

    opcaoLevel = (opcaoLevel - 1 + 2) % 2; // Alternar entre as opções no Menu Principal 

  } else if (menuLevel == 1) { 

    opcaoLevel = (opcaoLevel - 1 + 2) % 2; // Alternar entre as opções no Submenu Motor 

  } else if (menuLevel == 2) { 

    motorSpeed -= 5; // Diminuir a velocidade do motor 

    if (motorSpeed < 5) motorSpeed = 5; // Limitar a velocidade mínima 

  } else if (menuLevel == 3) { 

    opcaoLevel = (opcaoLevel - 1 + 2) % 2; // Alternar entre as opções no Submenu Aquecedor 

  } else if (menuLevel == 4) { 

    heaterTemp -= 10; // Diminuir a temperatura do aquecedor 

    if (heaterTemp < 200) heaterTemp = 200; // Limitar a temperatura mínima 

  } 

} 

 

void handleButtonLeft() { 

  if (menuLevel == 0 && opcaoLevel == 0) { 

    menuLevel = 1; // Ir para o submenu Motor 

    opcaoLevel = 0; // Reiniciar as opções 

  } else if (menuLevel == 1 && opcaoLevel == 0) { 

    motorOn = !motorOn; // Alternar estado do motor 

    if (motorOn) { 

      myStepper.setSpeed(motorSpeed); 

    } else { 

      myStepper.stop(); 

    } 

  } else if (menuLevel == 1 && opcaoLevel == 1) { 

    menuLevel = 2; // Ir para o submenu Velocidade 

  } else if (menuLevel == 0 && opcaoLevel == 1) { 

    menuLevel = 3; // Ir para o submenu Aquecedor 

    opcaoLevel = 0; // Reiniciar as opções 

  } else if (menuLevel == 3 && opcaoLevel == 0) { 

    heaterOn = !heaterOn; // Alternar estado do aquecedor 

  } else if (menuLevel == 3 && opcaoLevel == 1) { 

    menuLevel = 4; // Ir para o submenu Temperatura 

  } else if (menuLevel == 2 || menuLevel == 4) { 

    menuLevel = 0; // Voltar ao menu principal 

  } 

} 

 

void handleButtonRight() { 

  if (menuLevel == 2 || menuLevel == 4) { 

    menuLevel = 0; // Voltar ao menu principal 
  }
  
  } 