/*
 * W5500 Diagnostic Tool
 * Диагностический код для проверки подключения W5500
 * 
 * Этот код поможет выяснить, почему Ethernet не инициализируется
 */

#include <SPI.h>
#include <Ethernet2.h>

// MAC-адрес
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// IP адрес (попробуем DHCP сначала)
IPAddress ip(192, 168, 1, 140);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Пины для W5500 (стандартные)
const int W5500_CS = 10;   // Chip Select
const int W5500_RST = 9;   // Reset (если подключен)

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // ждем подключения Serial порта
  }
  
  delay(2000); // Даем время для открытия Serial Monitor
  
  Serial.println(F("\n\n========================================"));
  Serial.println(F("=== W5500 Diagnostic Tool ==="));
  Serial.println(F("========================================\n"));
  
  // Проверка пинов
  Serial.println(F("1. Проверка пинов..."));
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  // Проверка Reset пина (если подключен)
  Serial.print(F("   Reset пин (9): "));
  pinMode(W5500_RST, OUTPUT);
  digitalWrite(W5500_RST, LOW);
  delay(100);
  digitalWrite(W5500_RST, HIGH);
  delay(100);
  Serial.println(F("OK"));
  
  // Проверка CS пина
  Serial.print(F("   CS пин (10): "));
  pinMode(W5500_CS, OUTPUT);
  digitalWrite(W5500_CS, HIGH);
  Serial.println(F("OK"));
  
  Serial.println();
  
  // Инициализация SPI
  Serial.println(F("2. Инициализация SPI..."));
  SPI.begin();
  Serial.println(F("   SPI.begin() выполнен"));
  Serial.println(F("   MOSI: Pin 11"));
  Serial.println(F("   MISO: Pin 12"));
  Serial.println(F("   SCK:  Pin 13"));
  Serial.println();
  
  // Инициализация Ethernet с таймаутом
  Serial.println(F("3. Инициализация Ethernet..."));
  Serial.print(F("   MAC адрес: "));
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 0x10) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  Serial.println(F("   Пытаемся получить IP через DHCP..."));
  Serial.println(F("   (это может занять до 30 секунд)"));
  
  unsigned long startTime = millis();
  byte result = Ethernet.begin(mac, 10000); // Таймаут 10 секунд
  
  unsigned long elapsed = millis() - startTime;
  
  if (result == 0) {
    Serial.println(F("   ❌ DHCP не удался"));
    Serial.println(F("   Пробуем статический IP..."));
    
    Ethernet.begin(mac, ip, gateway, subnet);
    
    Serial.print(F("   Статический IP установлен: "));
    Serial.println(Ethernet.localIP());
    
    // Проверяем, действительно ли получили IP
    IPAddress localIP = Ethernet.localIP();
    if (localIP[0] == 0 && localIP[1] == 0 && localIP[2] == 0 && localIP[3] == 0) {
      Serial.println(F("   ❌ ОШИБКА: IP адрес = 0.0.0.0"));
      Serial.println(F("   Это означает, что W5500 не отвечает!"));
      Serial.println();
      printDiagnostics();
      return;
    }
  } else {
    Serial.print(F("   ✅ DHCP успешен! Время: "));
    Serial.print(elapsed);
    Serial.println(F(" мс"));
    Serial.print(F("   IP адрес: "));
    Serial.println(Ethernet.localIP());
  }
  
  Serial.println();
  
  // Проверка связи
  Serial.println(F("4. Проверка связи с W5500..."));
  IPAddress localIP = Ethernet.localIP();
  
  if (localIP[0] != 0 || localIP[1] != 0 || localIP[2] != 0 || localIP[3] != 0) {
    Serial.println(F("   ✅ W5500 отвечает!"));
    Serial.print(F("   IP адрес: "));
    Serial.println(localIP);
    Serial.print(F("   Subnet mask: "));
    Serial.println(Ethernet.subnetMask());
    Serial.print(F("   Gateway: "));
    Serial.println(Ethernet.gatewayIP());
    Serial.print(F("   DNS: "));
    Serial.println(Ethernet.dnsServerIP());
  } else {
    Serial.println(F("   ❌ W5500 не отвечает!"));
    printDiagnostics();
    return;
  }
  
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("✅ Диагностика завершена успешно!"));
  Serial.println(F("========================================\n"));
  
  // Мигаем LED 3 раза
  for (int i = 0; i < 3; i++) {
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);
  }
  
  Serial.println(F("Теперь можно загрузить основной код сервера."));
}

void loop() {
  // Периодически проверяем состояние
  static unsigned long lastCheck = 0;
  
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    
    // Проверяем, не потеряли ли мы IP
    IPAddress currentIP = Ethernet.maintain();
    
    if (currentIP[0] == 0 && currentIP[1] == 0 && 
        currentIP[2] == 0 && currentIP[3] == 0) {
      Serial.println(F("⚠️  ВНИМАНИЕ: IP адрес потерян!"));
      printDiagnostics();
    } else {
      Serial.print(F("✓ IP активен: "));
      Serial.println(Ethernet.localIP());
    }
  }
  
  // Мигаем LED каждые 2 секунды
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  if (millis() - lastBlink > 2000) {
    lastBlink = millis();
    ledState = !ledState;
    digitalWrite(13, ledState);
  }
}

void printDiagnostics() {
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("🔍 ДИАГНОСТИКА ПРОБЛЕМЫ"));
  Serial.println(F("========================================"));
  Serial.println();
  
  Serial.println(F("Проверьте следующее:"));
  Serial.println();
  
  Serial.println(F("1. ПОДКЛЮЧЕНИЕ ПИТАНИЯ:"));
  Serial.println(F("   ✓ VCC подключен к 5V (или 3.3V)"));
  Serial.println(F("   ✓ GND подключен к GND"));
  Serial.println();
  
  Serial.println(F("2. ПОДКЛЮЧЕНИЕ SPI:"));
  Serial.println(F("   ✓ MOSI → Pin 11"));
  Serial.println(F("   ✓ MISO → Pin 12"));
  Serial.println(F("   ✓ SCK  → Pin 13"));
  Serial.println();
  
  Serial.println(F("3. ПОДКЛЮЧЕНИЕ УПРАВЛЯЮЩИХ СИГНАЛОВ:"));
  Serial.println(F("   ✓ WCS (CS) → Pin 10"));
  Serial.println(F("   ✓ RESER (Reset) → Pin 9 (желательно)"));
  Serial.println();
  
  Serial.println(F("4. ПРОВЕРКА БИБЛИОТЕКИ:"));
  Serial.println(F("   ✓ Установлена библиотека Ethernet2 (не Ethernet!)"));
  Serial.println();
  
  Serial.println(F("5. ПРОВЕРКА СЕТИ:"));
  Serial.println(F("   ✓ Ethernet кабель подключен"));
  Serial.println(F("   ✓ Роутер/свитч включен"));
  Serial.println(F("   ✓ Светодиоды на разъеме горят"));
  Serial.println();
  
  Serial.println(F("6. ВОЗМОЖНЫЕ ПРОБЛЕМЫ:"));
  Serial.println(F("   - Неправильное напряжение питания"));
  Serial.println(F("   - Неправильное подключение CS или Reset"));
  Serial.println(F("   - Проблемы с SPI (проверьте другие устройства на SPI)"));
  Serial.println(F("   - Неисправный модуль W5500"));
  Serial.println(F("   - Проблемы с библиотекой Ethernet2"));
  Serial.println();
  
  Serial.println(F("========================================"));
}


