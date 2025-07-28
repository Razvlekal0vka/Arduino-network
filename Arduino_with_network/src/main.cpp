#include <SPI.h>
#include <Ethernet2.h>
#include "NonBlockingTimer.h"

// Прототипы функций
void sendJsonStatus(EthernetClient &client);
void sendJsonSensors(EthernetClient &client);
void sendStressTestData(EthernetClient &client);
void sendPerformanceStats(EthernetClient &client);
void handleControlCommand(EthernetClient &client, String request);
void sendWebInterface(EthernetClient &client);
int calculateResponseSize(String request);
void printPerformanceStats();
void handlePostRequest(EthernetClient &client, String postData);
void handleRobotData(EthernetClient &client, String postData);
String parseJsonValue(String json, String key);
void processRobotCommand(String command, String data);

// MAC-адрес и IP
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 140);
EthernetServer server(80);

// Статистика производительности
unsigned long totalRequests = 0;
unsigned long totalBytesSent = 0;
unsigned long totalBytesReceived = 0;
unsigned long startTime = 0;

// Счетчики для разных типов запросов
unsigned long statusRequests = 0;
unsigned long sensorRequests = 0;
unsigned long stressRequests = 0;
unsigned long controlRequests = 0;
unsigned long postRequests = 0;
unsigned long robotRequests = 0;

// Буфер для больших данных
char largeDataBuffer[1024];

// Данные робота
struct RobotData {
  String id;
  float temperature;
  int battery;
  float x, y, angle;
  String mode;
  String status;
  unsigned long timestamp;
} robotData;

// Состояние системы
bool systemOn = true;
bool manualMode = false;
int powerLevel = 75;

// Неблокирующие таймеры
NonBlockingTimer statsTimer(10000);      // Статистика каждые 10 сек
NonBlockingTimer robotUpdateTimer(5000); // Обновление робота каждые 5 сек
NonBlockingTimer ethernetCheckTimer(30000); // Проверка сети каждые 30 сек
NonBlockingDelay ledBlinkDelay;         // Задержка для мигания LED

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
  
  // Неблокирующая задержка для стабилизации Ethernet
  ledBlinkDelay.start(1000);
  while (!ledBlinkDelay.isFinished()) {
    // Ждем 1 секунду неблокирующе
  }
  
  server.begin();
  
  Serial.print(F("Server IP: "));
  Serial.println(Ethernet.localIP());
  Serial.println(F("Speed Test Endpoints:"));
  Serial.println(F("  /api/status - статус системы"));
  Serial.println(F("  /api/sensors - данные датчиков"));
  Serial.println(F("  /api/stress - стресс-тест"));
  Serial.println(F("  /api/stats - статистика"));
  Serial.println(F("  /api/control - управление"));
  Serial.println(F("  POST /api/control - адаптивное тестирование"));
  Serial.println(F("  POST /api/robot - данные робота"));
  
  startTime = millis();
  
  // Инициализация данных робота
  robotData.id = "robot1";
  robotData.temperature = 22.5;
  robotData.battery = 87;
  robotData.x = 0.0;
  robotData.y = 0.0;
  robotData.angle = 0.0;
  robotData.mode = "idle";
  robotData.status = "ready";
  robotData.timestamp = millis();
  
  // Индикация готовности (неблокирующая)
  Serial.println(F("Initializing..."));
  for (int i = 0; i < 3; i++) {
    digitalWrite(13, HIGH);
    ledBlinkDelay.start(100);
    while (!ledBlinkDelay.isFinished()) {
      // Ждем 100мс неблокирующе
    }
    digitalWrite(13, LOW);
    ledBlinkDelay.start(100);
    while (!ledBlinkDelay.isFinished()) {
      // Ждем 100мс неблокирующе
    }
  }
  
  // Запускаем таймеры
  statsTimer.start();
  robotUpdateTimer.start();
  ethernetCheckTimer.start();
  
  Serial.println(F("Server ready!"));
}

void loop() {
  // Проверяем подключение клиента
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
    bool isPostRequest = false;
    int contentLength = 0;
    
    // Читаем заголовки запроса
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
            // Проверяем, это POST запрос?
            if (firstLine.indexOf(F("POST")) != -1) {
              isPostRequest = true;
            }
          } else {
            // Ищем Content-Length для POST запросов
            if (isPostRequest && request.indexOf(F("Content-Length:")) != -1) {
              int start = request.indexOf(F("Content-Length:")) + 15;
              String lengthStr = request.substring(start);
              lengthStr.trim();
              contentLength = lengthStr.toInt();
            }
          }
          request = "";
        } else if (c != '\r') {
          currentLineIsBlank = false;
          request += c;
        }
      }
    }
    
    // Читаем тело POST запроса
    String postData = "";
    if (isPostRequest && contentLength > 0) {
      Serial.print(F("Reading POST data, length: "));
      Serial.println(contentLength);
      
      unsigned long postStartTime = millis();
      while (client.available() && postData.length() < contentLength) {
        char c = client.read();
        postData += c;
        bytesReceived++;
        
        // Таймаут для чтения POST данных
        if (millis() - postStartTime > 5000) break;
      }
      
      Serial.print(F("Received POST data: "));
      Serial.print(postData.length());
      Serial.println(F(" bytes"));
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
    else if (firstLine.indexOf(F("POST /api/control")) != -1) {
      postRequests++;
      handlePostRequest(client, postData);
    }
    else if (firstLine.indexOf(F("POST /api/robot")) != -1) {
      handleRobotData(client, postData);
    }
    else {
      sendWebInterface(client);
    }
    
    totalBytesSent += calculateResponseSize(firstLine);
    
    client.stop();
    digitalWrite(13, LOW);
    Serial.println(F("Client disconnected"));
  }
  
  // Неблокирующие операции в основном цикле
  
  // Вывод статистики каждые 10 секунд
  if (statsTimer.isReady()) {
    printPerformanceStats();
  }
  
  // Обновление данных робота каждые 5 секунд
  if (robotUpdateTimer.isReady()) {
    // Автообновление данных робота (если нужно)
    // robotData.temperature += random(-10, 11) / 10.0;
  }
  
  // Проверка состояния Ethernet каждые 30 секунд
  if (ethernetCheckTimer.isReady()) {
    // Можно добавить проверку состояния сети
    // Serial.println(F("Network status check..."));
  }
  
  // Другие неблокирующие операции можно добавить здесь
  // Например, мигание LED для индикации активности
  static bool ledState = false;
  static unsigned long lastLedToggle = 0;
  
  if (millis() - lastLedToggle > 5000) { // Мигаем каждые 5 секунд
    ledState = !ledState;
    digitalWrite(13, ledState);
    lastLedToggle = millis();
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
  sensorRequests++;
  
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  
  // Используем данные робота для датчиков
  client.print(F("{"));
  client.print(F("\"t\":")); client.print(robotData.temperature, 1);
  client.print(F(",\"h\":")); client.print(45.0, 1); // Фиксированная влажность
  client.print(F(",\"p\":")); client.print(1013.25, 2); // Фиксированное давление
  client.print(F(",\"battery\":")); client.print(robotData.battery);
  client.print(F(",\"x\":")); client.print(robotData.x, 1);
  client.print(F(",\"y\":")); client.print(robotData.y, 1);
  client.print(F(",\"angle\":")); client.print(robotData.angle, 1);
  client.print(F(",\"mode\":\"")); client.print(robotData.mode);
  client.print(F("\",\"status\":\"")); client.print(robotData.status);
  client.print(F("\",\"timestamp\":")); client.print(millis());
  client.println(F("}"));
}

void sendStressTestData(EthernetClient &client) {
  stressRequests++;
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
  controlRequests++;
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
  controlRequests++;
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

void handlePostRequest(EthernetClient &client, String postData) {
  postRequests++;
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  
  // Анализируем полученные данные
  int dataSize = postData.length();
  unsigned long timestamp = millis();
  
  client.print(F("{"));
  client.print(F("\"received\":true,"));
  client.print(F("\"data_size\":")); client.print(dataSize);
  client.print(F(",\"timestamp\":")); client.print(timestamp);
  client.print(F(",\"message\":\"Data received successfully\""));
  client.println(F("}"));
  
  Serial.print(F("POST request processed, data size: "));
  Serial.println(dataSize);
}

void handleRobotData(EthernetClient &client, String postData) {
  robotRequests++;
  
  // Парсим JSON данные робота
  robotData.id = parseJsonValue(postData, "id");
  robotData.temperature = parseJsonValue(postData, "temp").toFloat();
  robotData.battery = parseJsonValue(postData, "batt").toInt();
  robotData.x = parseJsonValue(postData, "x").toFloat();
  robotData.y = parseJsonValue(postData, "y").toFloat();
  robotData.angle = parseJsonValue(postData, "angle").toFloat();
  robotData.mode = parseJsonValue(postData, "mode");
  robotData.status = parseJsonValue(postData, "status");
  robotData.timestamp = millis();
  
  // Обрабатываем команды робота
  String command = parseJsonValue(postData, "command");
  if (command.length() > 0) {
    processRobotCommand(command, postData);
  }
  
  // Отправляем ответ
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Connection: close"));
  client.println();
  
  client.print(F("{"));
  client.print(F("\"robot_data_received\":true,"));
  client.print(F("\"timestamp\":")); client.print(robotData.timestamp);
  client.print(F(",\"system_status\":{"));
  client.print(F("\"on\":")); client.print(systemOn ? "true" : "false");
  client.print(F(",\"manual\":")); client.print(manualMode ? "true" : "false");
  client.print(F(",\"power\":")); client.print(powerLevel);
  client.print(F(",\"led\":")); client.print(digitalRead(13) ? "true" : "false");
  client.print(F("},"));
  client.print(F("\"response\":\"Robot data processed successfully\""));
  client.println(F("}"));
  
  Serial.print(F("Robot data received: "));
  Serial.print(robotData.id);
  Serial.print(F(" | Temp: ")); Serial.print(robotData.temperature);
  Serial.print(F(" | Batt: ")); Serial.print(robotData.battery);
  Serial.print(F("% | Pos: (")); Serial.print(robotData.x);
  Serial.print(F(",")); Serial.print(robotData.y);
  Serial.print(F(") | Angle: ")); Serial.print(robotData.angle);
  Serial.print(F(" | Mode: ")); Serial.print(robotData.mode);
  Serial.print(F(" | Status: ")); Serial.println(robotData.status);
}

String parseJsonValue(String json, String key) {
  String searchKey = "\"" + key + "\":";
  int startPos = json.indexOf(searchKey);
  if (startPos == -1) return "";
  
  startPos += searchKey.length();
  
  // Пропускаем пробелы
  while (startPos < json.length() && json.charAt(startPos) == ' ') {
    startPos++;
  }
  
  int endPos = startPos;
  
  // Если значение в кавычках
  if (startPos < json.length() && json.charAt(startPos) == '"') {
    startPos++;
    endPos = json.indexOf('"', startPos);
  } else {
    // Если числовое значение
    while (endPos < json.length() && 
           (isDigit(json.charAt(endPos)) || json.charAt(endPos) == '.' || json.charAt(endPos) == '-')) {
      endPos++;
    }
  }
  
  if (endPos > startPos) {
    return json.substring(startPos, endPos);
  }
  
  return "";
}

void processRobotCommand(String command, String data) {
  Serial.print(F("Processing robot command: "));
  Serial.println(command);
  
  if (command == "led_on") {
    digitalWrite(13, HIGH);
    Serial.println(F("LED turned ON"));
  }
  else if (command == "led_off") {
    digitalWrite(13, LOW);
    Serial.println(F("LED turned OFF"));
  }
  else if (command == "system_on") {
    systemOn = true;
    Serial.println(F("System turned ON"));
  }
  else if (command == "system_off") {
    systemOn = false;
    Serial.println(F("System turned OFF"));
  }
  else if (command == "manual_mode") {
    manualMode = true;
    Serial.println(F("Manual mode enabled"));
  }
  else if (command == "auto_mode") {
    manualMode = false;
    Serial.println(F("Auto mode enabled"));
  }
  else if (command == "set_power") {
    String powerStr = parseJsonValue(data, "power");
    if (powerStr.length() > 0) {
      powerLevel = powerStr.toInt();
      Serial.print(F("Power level set to: "));
      Serial.println(powerLevel);
    }
  }
  else if (command == "get_status") {
    Serial.println(F("Status request processed"));
  }
  else {
    Serial.print(F("Unknown command: "));
    Serial.println(command);
  }
}

int calculateResponseSize(String request) {
  // Примерный размер ответа в байтах
  if (request.indexOf(F("GET /api/status")) != -1) return 150;
  if (request.indexOf(F("GET /api/sensors")) != -1) return 200; // Увеличил для данных робота
  if (request.indexOf(F("GET /api/stress")) != -1) return 2500;
  if (request.indexOf(F("GET /api/stats")) != -1) return 300;
  if (request.indexOf(F("GET /api/control")) != -1) return 200;
  if (request.indexOf(F("POST /api/control")) != -1) return 100;
  if (request.indexOf(F("POST /api/robot")) != -1) return 150; // Новый endpoint
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
  Serial.print(F("POST запросов: ")); Serial.println(postRequests);
  Serial.print(F("Робот запросов: ")); Serial.println(robotRequests);
  Serial.println(F("========================================"));
} 