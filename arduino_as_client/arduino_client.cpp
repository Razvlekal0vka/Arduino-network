#include <SPI.h>
#include <Ethernet2.h>

// MAC-адрес
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// IP адрес Arduino
IPAddress ip(192, 168, 1, 140);
// IP адрес ноутбука (ваш IP)
IPAddress serverIP(192, 168, 1, 103);

// Клиент
EthernetClient client;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // ждем подключения Serial порта
  }
  
  Serial.println("=== Arduino Client Test ===");
  
  // Инициализация Ethernet
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Serial.println("Trying with static IP...");
    Ethernet.begin(mac, ip);
  }
  
  // Выводим IP адрес
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  
  Serial.println("=============================");
}

void loop() {
  Serial.println("Trying to connect to server...");
  
  // Пытаемся подключиться к серверу
  if (client.connect(serverIP, 8080)) {
    Serial.println("Connected to server!");
    
    // Отправляем сообщение
    client.println("Hello from Arduino!");
    
    // Ждем ответ (до 2 секунд)
    unsigned long start = millis();
    while (millis() - start < 2000) {
      while (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }
    
    // Закрываем соединение
    client.stop();
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Connection failed!");
  }
  
  delay(5000); // Ждем 5 секунд перед следующей попыткой
} 