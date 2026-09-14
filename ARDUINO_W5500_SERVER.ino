/*
 * Arduino W5500 Ethernet Server
 * Простой веб-сервер для Arduino Uno с W5500
 * 
 * Подключение W5500:
 * VCC  -> 5V
 * GND  -> GND
 * MOSI -> Pin 11
 * MISO -> Pin 12
 * SCK  -> Pin 13
 * WCS  -> Pin 10
 * RESER-> Pin 9 (желательно)
 */

#include <SPI.h>
#include <Ethernet2.h>

// MAC-адрес (можно оставить как есть или изменить)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// IP адрес (ИЗМЕНИТЕ НА ВАШ IP!)
IPAddress ip(192, 168, 1, 140);

// Порт сервера
EthernetServer server(80);

// Состояние системы
bool systemOn = false;
int powerLevel = 0;
bool manualMode = false;

// Виртуальные датчики
float temperature = 22.5;
float humidity = 45.0;
float pressure = 1013.25;

void setup() {
  Serial.begin(9600);
  while (!Serial) { 
    ; // ждем подключения Serial порта
  }
  
  delay(2000); // Даем время для открытия Serial Monitor
  
  Serial.println(F("\n\n=== Arduino W5500 Server ==="));
  Serial.println(F("Версия с улучшенной диагностикой\n"));
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  // Инициализация пинов W5500
  Serial.println(F("1. Инициализация пинов..."));
  pinMode(10, OUTPUT);  // CS пин
  digitalWrite(10, HIGH);
  pinMode(9, OUTPUT);   // Reset пин (если подключен)
  digitalWrite(9, LOW);
  delay(100);
  digitalWrite(9, HIGH);
  delay(100);
  Serial.println(F("   ✓ Пины инициализированы"));
  
  // Инициализация SPI
  Serial.println(F("2. Инициализация SPI..."));
  SPI.begin();
  Serial.println(F("   ✓ SPI инициализирован"));
  
  // Инициализация Ethernet
  Serial.println(F("3. Инициализация Ethernet..."));
  Serial.print(F("   MAC: "));
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 0x10) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  Serial.println(F("   Пытаемся получить IP через DHCP..."));
  Serial.println(F("   (таймаут 15 секунд)"));
  
  unsigned long startTime = millis();
  byte result = Ethernet.begin(mac, 15000); // Таймаут 15 секунд
  unsigned long elapsed = millis() - startTime;
  
  if (result == 0) {
    Serial.print(F("   ⚠ DHCP не удался (время: "));
    Serial.print(elapsed);
    Serial.println(F(" мс)"));
    Serial.println(F("   Пробуем статический IP..."));
    
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    Ethernet.begin(mac, ip, gateway, subnet);
    
    Serial.print(F("   Статический IP: "));
    Serial.println(Ethernet.localIP());
  } else {
    Serial.print(F("   ✓ DHCP успешен! (время: "));
    Serial.print(elapsed);
    Serial.print(F(" мс)"));
    Serial.print(F(" IP: "));
    Serial.println(Ethernet.localIP());
  }
  
  // Проверка IP адреса
  IPAddress localIP = Ethernet.localIP();
  if (localIP[0] == 0 && localIP[1] == 0 && localIP[2] == 0 && localIP[3] == 0) {
    Serial.println(F("\n   ❌ ОШИБКА: IP адрес = 0.0.0.0"));
    Serial.println(F("   W5500 не отвечает!"));
    Serial.println(F("\n   Проверьте:"));
    Serial.println(F("   - Подключение пинов (VCC, GND, MOSI, MISO, SCK, CS, Reset)"));
    Serial.println(F("   - Питание (5V или 3.3V)"));
    Serial.println(F("   - Ethernet кабель"));
    Serial.println(F("   - Библиотеку Ethernet2 (не Ethernet!)"));
    Serial.println(F("\n   Загрузите W5500_DIAGNOSTIC.ino для детальной диагностики"));
    
    // Мигаем LED быстро (ошибка)
    while (true) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
  }
  
  delay(1000);
  
  Serial.println(F("\n4. Запуск сервера..."));
  server.begin();
  
  Serial.print(F("   ✓ Сервер запущен на IP: "));
  Serial.println(Ethernet.localIP());
  Serial.println(F("   ✓ Порт: 80"));
  
  Serial.println(F("\n5. API endpoints:"));
  Serial.println(F("   GET /api/status"));
  Serial.println(F("   GET /api/sensors"));
  Serial.println(F("   GET /api/control?cmd=on"));
  Serial.println(F("   GET /api/control?cmd=off"));
  Serial.println(F("   GET /api/control?power=50"));
  
  // Индикация готовности
  Serial.println(F("\n✅ Сервер готов!"));
  for (int i = 0; i < 3; i++) {
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);
  }
  
  Serial.println(F("\nОткройте в браузере: http://"));
  Serial.print(Ethernet.localIP());
  Serial.println(F("\n"));
}

void loop() {
  // Обновляем датчики (симуляция)
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    temperature += random(-10, 11) / 10.0;
    humidity += random(-5, 6) / 10.0;
    pressure += random(-2, 3) / 10.0;
    
    temperature = constrain(temperature, 15.0, 35.0);
    humidity = constrain(humidity, 30.0, 80.0);
    pressure = constrain(pressure, 1000.0, 1030.0);
    
    lastUpdate = millis();
  }
  
  // Проверяем подключение клиента
  EthernetClient client = server.available();
  if (client) {
    Serial.println(F("Client connected!"));
    digitalWrite(13, HIGH);
    
    String request = "";
    String firstLine = "";
    bool currentLineIsBlank = true;
    bool gotFirstLine = false;
    
    // Читаем HTTP запрос
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n' && currentLineIsBlank) {
          break; // Конец запроса
        }
        
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
    
    Serial.print(F("Request: "));
    Serial.println(firstLine);
    
    // Обработка запросов
    if (firstLine.indexOf(F("GET /api/status")) != -1) {
      sendJsonStatus(client);
    }
    else if (firstLine.indexOf(F("GET /api/sensors")) != -1) {
      sendJsonSensors(client);
    }
    else if (firstLine.indexOf(F("GET /api/control")) != -1) {
      handleControlCommand(client, firstLine);
    }
    else {
      sendWebInterface(client);
    }
    
    delay(1);
    client.stop();
    digitalWrite(13, LOW);
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
  client.print(F("\"on\":")); 
  client.print(systemOn ? F("true") : F("false"));
  client.print(F(",\"manual\":")); 
  client.print(manualMode ? F("true") : F("false"));
  client.print(F(",\"power\":")); 
  client.print(powerLevel);
  client.println(F("}"));
}

void sendJsonSensors(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  
  client.print(F("{"));
  client.print(F("\"t\":")); 
  client.print(temperature, 1);
  client.print(F(",\"h\":")); 
  client.print(humidity, 1);
  client.print(F(",\"p\":")); 
  client.print(pressure, 2);
  client.println(F("}"));
}

void handleControlCommand(EthernetClient &client, String request) {
  // Обработка команд
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
    Serial.println(F("Manual mode"));
  }
  else if (request.indexOf(F("cmd=auto")) != -1) {
    manualMode = false;
    Serial.println(F("Auto mode"));
  }
  
  // Обработка мощности
  if (request.indexOf(F("power=")) != -1) {
    int start = request.indexOf(F("power=")) + 6;
    int end = request.indexOf('&', start);
    if (end == -1) end = request.length();
    String powerStr = request.substring(start, end);
    powerLevel = powerStr.toInt();
    powerLevel = constrain(powerLevel, 0, 100);
    Serial.print(F("Power: "));
    Serial.println(powerLevel);
  }
  
  // Отправляем ответ
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  
  client.print(F("{"));
  client.print(F("\"ok\":true,"));
  client.print(F("\"on\":")); 
  client.print(systemOn ? F("true") : F("false"));
  client.print(F(",\"manual\":")); 
  client.print(manualMode ? F("true") : F("false"));
  client.print(F(",\"power\":")); 
  client.print(powerLevel);
  client.println(F("}"));
}

void sendWebInterface(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html; charset=utf-8"));
  client.println(F("Connection: close"));
  client.println();
  
  client.println(F("<!DOCTYPE html>"));
  client.println(F("<html><head><title>Arduino W5500 Server</title>"));
  client.println(F("<meta charset='utf-8'></head><body>"));
  client.println(F("<h2>Arduino W5500 Server</h2>"));
  
  client.print(F("<p><b>System:</b> ")); 
  client.print(systemOn ? F("ON") : F("OFF"));
  client.print(F(" | <b>Mode:</b> ")); 
  client.print(manualMode ? F("MANUAL") : F("AUTO"));
  client.print(F(" | <b>Power:</b> ")); 
  client.print(powerLevel); 
  client.println(F("%</p>"));
  
  client.print(F("<p><b>Temperature:</b> ")); 
  client.print(temperature, 1);
  client.print(F("°C | <b>Humidity:</b> ")); 
  client.print(humidity, 1);
  client.print(F("% | <b>Pressure:</b> ")); 
  client.print(pressure, 2); 
  client.println(F(" hPa</p>"));
  
  client.println(F("<hr>"));
  client.println(F("<h3>API Endpoints:</h3>"));
  client.println(F("<ul>"));
  client.println(F("<li><a href='/api/status'>/api/status</a> - статус системы</li>"));
  client.println(F("<li><a href='/api/sensors'>/api/sensors</a> - данные датчиков</li>"));
  client.println(F("<li><a href='/api/control?cmd=on'>/api/control?cmd=on</a> - включить</li>"));
  client.println(F("<li><a href='/api/control?cmd=off'>/api/control?cmd=off</a> - выключить</li>"));
  client.println(F("<li><a href='/api/control?power=50'>/api/control?power=50</a> - мощность 50%</li>"));
  client.println(F("</ul>"));
  
  client.println(F("<hr>"));
  client.println(F("<p><a href='/'>Обновить</a></p>"));
  client.println(F("</body></html>"));
}

