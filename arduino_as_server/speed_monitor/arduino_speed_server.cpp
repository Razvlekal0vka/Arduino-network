#include <SPI.h>
#include <Ethernet2.h>

// MAC-адрес и IP
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 140);
EthernetServer server(80);

// Статистика производительности
unsigned long totalRequests = 0;
unsigned long totalBytesSent = 0;
unsigned long totalBytesReceived = 0;
unsigned long startTime = 0;
unsigned long lastStatsTime = 0;

// Счетчики для разных типов запросов
unsigned long statusRequests = 0;
unsigned long sensorRequests = 0;
unsigned long stressRequests = 0;
unsigned long controlRequests = 0;

// Буфер для больших данных
char largeDataBuffer[1024];

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }
  
  Serial.println(F("=== Arduino Speed Test Server ==="));
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  // Инициализация Ethernet
  Serial.println(F("Initializing Ethernet..."));
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("DHCP failed, using static IP"));
    Ethernet.begin(mac, ip);
  }
  delay(1000);
  server.begin();
  
  Serial.print(F("Server IP: "));
  Serial.println(Ethernet.localIP());
  Serial.println(F("Speed Test Endpoints:"));
  Serial.println(F("  /api/status - статус системы"));
  Serial.println(F("  /api/sensors - данные датчиков"));
  Serial.println(F("  /api/stress - стресс-тест"));
  Serial.println(F("  /api/stats - статистика"));
  Serial.println(F("  /api/control - управление"));
  
  startTime = millis();
  lastStatsTime = millis();
  
  // Индикация готовности
  for (int i = 0; i < 3; i++) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
  }
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    unsigned long requestStartTime = millis();
    int bytesReceived = 0;
    
    Serial.println(F("Client connected!"));
    digitalWrite(13, HIGH); // Индикация активности
    
    String request = "";
    String firstLine = "";
    bool currentLineIsBlank = true;
    bool gotFirstLine = false;
    
    // Читаем запрос
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        bytesReceived++;
        
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
    
    totalBytesReceived += bytesReceived;
    totalRequests++;
    
    Serial.print(F("Request: "));
    Serial.println(firstLine);
    
    // Обработка запросов
    if (firstLine.indexOf(F("GET /api/status")) != -1) {
      statusRequests++;
      sendJsonStatus(client);
    }
    else if (firstLine.indexOf(F("GET /api/sensors")) != -1) {
      sensorRequests++;
      sendJsonSensors(client);
    }
    else if (firstLine.indexOf(F("GET /api/stress")) != -1) {
      stressRequests++;
      sendStressTestData(client);
    }
    else if (firstLine.indexOf(F("GET /api/stats")) != -1) {
      sendPerformanceStats(client);
    }
    else if (firstLine.indexOf(F("GET /api/control")) != -1) {
      controlRequests++;
      handleControlCommand(client, firstLine);
    }
    else {
      sendWebInterface(client);
    }
    
    unsigned long requestDuration = millis() - requestStartTime;
    totalBytesSent += calculateResponseSize(firstLine);
    
    // Выводим статистику каждые 10 секунд
    if (millis() - lastStatsTime > 10000) {
      printPerformanceStats();
      lastStatsTime = millis();
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
  
  unsigned long uptime = millis() - startTime;
  client.print(F("{"));
  client.print(F("\"uptime\":")); client.print(uptime);
  client.print(F(",\"requests\":")); client.print(totalRequests);
  client.print(F(",\"bytes_sent\":")); client.print(totalBytesSent);
  client.print(F(",\"bytes_received\":")); client.print(totalBytesReceived);
  client.println(F("}"));
}

void sendJsonSensors(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  
  // Симулируем данные датчиков
  float temperature = 22.5 + random(-50, 51) / 10.0;
  float humidity = 45.0 + random(-20, 21) / 10.0;
  float pressure = 1013.25 + random(-10, 11) / 10.0;
  
  client.print(F("{"));
  client.print(F("\"t\":")); client.print(temperature, 1);
  client.print(F(",\"h\":")); client.print(humidity, 1);
  client.print(F(",\"p\":")); client.print(pressure, 2);
  client.print(F(",\"timestamp\":")); client.print(millis());
  client.println(F("}"));
}

void sendStressTestData(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  
  // Генерируем большой объем данных для стресс-теста
  client.print(F("{"));
  client.print(F("\"stress_test\":true,"));
  client.print(F("\"timestamp\":")); client.print(millis());
  client.print(F(",\"data\":["));
  
  // Генерируем массив из 100 элементов
  for (int i = 0; i < 100; i++) {
    if (i > 0) client.print(F(","));
    client.print(F("{"));
    client.print(F("\"id\":")); client.print(i);
    client.print(F(",\"value\":")); client.print(random(1000));
    client.print(F(",\"status\":\"")); 
    client.print(i % 2 == 0 ? F("active") : F("inactive"));
    client.print(F("\""));
    client.print(F("}"));
  }
  
  client.println(F("]}"));
}

void sendPerformanceStats(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  
  unsigned long uptime = millis() - startTime;
  float requestsPerSecond = (float)totalRequests / (uptime / 1000.0);
  float bytesPerSecond = (float)totalBytesSent / (uptime / 1000.0);
  
  client.print(F("{"));
  client.print(F("\"uptime\":")); client.print(uptime);
  client.print(F(",\"total_requests\":")); client.print(totalRequests);
  client.print(F(",\"requests_per_second\":")); client.print(requestsPerSecond, 2);
  client.print(F(",\"total_bytes_sent\":")); client.print(totalBytesSent);
  client.print(F(",\"total_bytes_received\":")); client.print(totalBytesReceived);
  client.print(F(",\"bytes_per_second\":")); client.print(bytesPerSecond, 2);
  client.print(F(",\"status_requests\":")); client.print(statusRequests);
  client.print(F(",\"sensor_requests\":")); client.print(sensorRequests);
  client.print(F(",\"stress_requests\":")); client.print(stressRequests);
  client.print(F(",\"control_requests\":")); client.print(controlRequests);
  client.println(F("}"));
}

void handleControlCommand(EthernetClient &client, String request) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  
  client.print(F("{"));
  client.print(F("\"command_received\":true,"));
  client.print(F("\"timestamp\":")); client.print(millis());
  client.print(F(",\"request\":\"")); client.print(request);
  client.println(F("\"}"));
}

void sendWebInterface(EthernetClient &client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html; charset=utf-8"));
  client.println(F("Connection: close"));
  client.println();
  
  client.println(F("<html><head><title>Arduino Speed Test Server</title><meta charset='utf-8'></head><body>"));
  client.println(F("<h2>Arduino Speed Test Server</h2>"));
  client.println(F("<p><b>API Endpoints:</b></p>"));
  client.println(F("<ul>"));
  client.println(F("<li><a href='/api/status'>/api/status</a> - статус системы</li>"));
  client.println(F("<li><a href='/api/sensors'>/api/sensors</a> - данные датчиков</li>"));
  client.println(F("<li><a href='/api/stress'>/api/stress</a> - стресс-тест</li>"));
  client.println(F("<li><a href='/api/stats'>/api/stats</a> - статистика производительности</li>"));
  client.println(F("<li><a href='/api/control'>/api/control</a> - управление</li>"));
  client.println(F("</ul>"));
  client.println(F("<p><b>Total Requests:</b> ")); client.print(totalRequests); client.println(F("</p>"));
  client.println(F("<p><b>Uptime:</b> ")); client.print((millis() - startTime) / 1000); client.println(F(" сек</p>"));
  client.println(F("</body></html>"));
}

int calculateResponseSize(String request) {
  // Примерный размер ответа в байтах
  if (request.indexOf(F("GET /api/status")) != -1) return 150;
  if (request.indexOf(F("GET /api/sensors")) != -1) return 120;
  if (request.indexOf(F("GET /api/stress")) != -1) return 2500;
  if (request.indexOf(F("GET /api/stats")) != -1) return 300;
  if (request.indexOf(F("GET /api/control")) != -1) return 200;
  return 500; // веб-интерфейс
}

void printPerformanceStats() {
  unsigned long uptime = millis() - startTime;
  float requestsPerSecond = (float)totalRequests / (uptime / 1000.0);
  float bytesPerSecond = (float)totalBytesSent / (uptime / 1000.0);
  
  Serial.println(F("\n=== СТАТИСТИКА ПРОИЗВОДИТЕЛЬНОСТИ ==="));
  Serial.print(F("Время работы: ")); Serial.print(uptime / 1000); Serial.println(F(" сек"));
  Serial.print(F("Всего запросов: ")); Serial.println(totalRequests);
  Serial.print(F("Запросов/сек: ")); Serial.println(requestsPerSecond, 2);
  Serial.print(F("Байт отправлено: ")); Serial.println(totalBytesSent);
  Serial.print(F("Байт/сек: ")); Serial.println(bytesPerSecond, 2);
  Serial.print(F("Статус запросов: ")); Serial.println(statusRequests);
  Serial.print(F("Датчик запросов: ")); Serial.println(sensorRequests);
  Serial.print(F("Стресс запросов: ")); Serial.println(stressRequests);
  Serial.print(F("Управление запросов: ")); Serial.println(controlRequests);
  Serial.println(F("=" * 40));
} 