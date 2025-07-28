#include <SPI.h>
#include <Ethernet2.h>

// Объявления функций
void sendJsonStatus(EthernetClient &client);
void sendJsonSensors(EthernetClient &client);
void handleControlCommand(EthernetClient &client, String request);
void sendWebInterface(EthernetClient &client);
void updateSensors();

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 140);
EthernetServer server(80);

// Состояние системы
int ledState = LOW;
bool systemOn = false;
int powerLevel = 0;  // 0-100%
bool manualMode = false;

// Виртуальные датчики
float temperature = 22.5;
float humidity = 45.0;
float pressure = 1013.25;

// Время для обновления датчиков
unsigned long lastSensorUpdate = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }
  Serial.println(F("Arduino Sensor & Control Server"));
  
  pinMode(13, OUTPUT);
  digitalWrite(13, ledState);
  
  Serial.println(F("Initializing Ethernet..."));
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("DHCP failed, using static IP"));
    Ethernet.begin(mac, ip);
  }
  delay(1000);
  server.begin();
  
  Serial.print(F("Server IP: "));
  Serial.println(Ethernet.localIP());
  Serial.println(F("API: /api/status, /api/sensors, /api/control"));
  
  // Индикация готовности
  for (int i = 0; i < 2; i++) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
  }
}

void updateSensors() {
  // Обновляем датчики каждые 2 секунды
  if (millis() - lastSensorUpdate > 2000) {
    // Симулируем реалистичные изменения
    temperature += random(-10, 11) / 10.0;  // ±1°C
    humidity += random(-5, 6) / 10.0;       // ±0.5%
    pressure += random(-2, 3) / 10.0;       // ±0.2 hPa
    
    // Ограничиваем значения
    temperature = constrain(temperature, 15.0, 35.0);
    humidity = constrain(humidity, 30.0, 80.0);
    pressure = constrain(pressure, 1000.0, 1030.0);
    
    lastSensorUpdate = millis();
  }
}

void loop() {
  updateSensors();
  
  EthernetClient client = server.available();
  if (client) {
    Serial.println(F("Client connected!"));
    
    String request = "";
    String firstLine = "";
    bool currentLineIsBlank = true;
    bool gotFirstLine = false;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) break;
        if (c == '\n') {
          currentLineIsBlank = true;
          if (!gotFirstLine) {
            firstLine = request;
            gotFirstLine = true;
          }
          request = "";
        } else if (c != '\r') {
          currentLineIsBlank = false;
          request += c;
        }
      }
    }
    
    Serial.print(F("First line: "));
    Serial.println(firstLine);
    
    // Игнорируем favicon.ico
    if (firstLine.indexOf(F("GET /favicon.ico")) != -1) {
      client.println(F("HTTP/1.1 404 Not Found"));
      client.println(F("Connection: close"));
      client.println();
      client.stop();
      Serial.println(F("Ignored favicon.ico"));
      return;
    }
    
    // API endpoints
    if (firstLine.indexOf(F("GET /api/status")) != -1) {
      sendJsonStatus(client);
    }
    else if (firstLine.indexOf(F("GET /api/sensors")) != -1) {
      sendJsonSensors(client);
    }
    else if (firstLine.indexOf(F("GET /api/control")) != -1) {
      handleControlCommand(client, firstLine);
    }
    // Веб-интерфейс
    else if (firstLine.indexOf(F("GET / ")) != -1 || firstLine.indexOf(F("GET / HTTP")) != -1) {
      sendWebInterface(client);
    }
    else if (firstLine.indexOf(F("GET /led/on")) != -1) {
      ledState = HIGH;
      digitalWrite(13, ledState);
      Serial.println(F("LED ON"));
      sendWebInterface(client);
    }
    else if (firstLine.indexOf(F("GET /led/off")) != -1) {
      ledState = LOW;
      digitalWrite(13, ledState);
      Serial.println(F("LED OFF"));
      sendWebInterface(client);
    }
    else if (firstLine.indexOf(F("GET /led/toggle")) != -1) {
      ledState = !ledState;
      digitalWrite(13, ledState);
      Serial.println(F("LED TOGGLE"));
      sendWebInterface(client);
    }
    else {
      sendWebInterface(client);
    }
    
    delay(1);
    client.stop();
    Serial.println(F("Client disconnected"));
  }
}

void sendJsonStatus(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  client.print(F("{"));
  client.print(F("\"on\":")); client.print(systemOn ? F("true") : F("false"));
  client.print(F(",\"manual\":")); client.print(manualMode ? F("true") : F("false"));
  client.print(F(",\"power\":")); client.print(powerLevel);
  client.println(F("}"));
}

void sendJsonSensors(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  client.print(F("{"));
  client.print(F("\"t\":")); client.print(temperature, 1);
  client.print(F(",\"h\":")); client.print(humidity, 1);
  client.print(F(",\"p\":")); client.print(pressure, 2);
  client.println(F("}"));
}

void handleControlCommand(EthernetClient &client, String request) {
  // Простая обработка команд из URL параметров
  if (request.indexOf(F("cmd=on")) != -1) {
    systemOn = true;
    Serial.println(F("System ON"));
  }
  else if (request.indexOf(F("cmd=off")) != -1) {
    systemOn = false;
    powerLevel = 0;
    Serial.println(F("System OFF"));
  }
  else if (request.indexOf(F("cmd=manual")) != -1) {
    manualMode = true;
    Serial.println(F("Manual mode ON"));
  }
  else if (request.indexOf(F("cmd=auto")) != -1) {
    manualMode = false;
    Serial.println(F("Auto mode ON"));
  }
  
  // Обработка мощности
  if (request.indexOf(F("power=")) != -1) {
    int start = request.indexOf(F("power=")) + 6;
    int end = request.indexOf('&', start);
    if (end == -1) end = request.length();
    String powerStr = request.substring(start, end);
    powerLevel = powerStr.toInt();
    powerLevel = constrain(powerLevel, 0, 100);
    Serial.print(F("Power set to: "));
    Serial.println(powerLevel);
  }
  
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  client.print(F("{"));
  client.print(F("\"ok\":true,"));
  client.print(F("\"on\":")); client.print(systemOn ? F("true") : F("false"));
  client.print(F(",\"manual\":")); client.print(manualMode ? F("true") : F("false"));
  client.print(F(",\"power\":")); client.print(powerLevel);
  client.println(F("}"));
}

void sendWebInterface(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html; charset=utf-8"));
  client.println(F("Connection: close"));
  client.println();
  
  client.println(F("<html><head><title>Arduino Control</title><meta charset='utf-8'></head><body>"));
  client.println(F("<h2>Arduino Control</h2>"));
  client.print(F("<b>System: </b>")); client.print(systemOn ? F("ON") : F("OFF"));
  client.print(F(" | <b>Mode: </b>")); client.print(manualMode ? F("MANUAL") : F("AUTO"));
  client.print(F(" | <b>Power: </b>")); client.print(powerLevel); client.println(F("%<br>"));
  client.print(F("<b>Temp: </b>")); client.print(temperature, 1);
  client.print(F("C | <b>Hum: </b>")); client.print(humidity, 1);
  client.print(F("% | <b>Press: </b>")); client.print(pressure, 2); client.println(F(" hPa<br><br>"));
  client.println(F("<a href='/api/control?cmd=on'>ON</a> | <a href='/api/control?cmd=off'>OFF</a> | <a href='/api/control?cmd=manual'>MANUAL</a> | <a href='/api/control?cmd=auto'>AUTO</a><br>"));
  client.println(F("<a href='/api/control?power=25'>25%</a> <a href='/api/control?power=50'>50%</a> <a href='/api/control?power=75'>75%</a> <a href='/api/control?power=100'>100%</a><br>"));
  client.println(F("<a href='/led/on'>LED ON</a> <a href='/led/off'>LED OFF</a> <a href='/'>REFRESH</a>"));
  client.println(F("</body></html>"));
} 