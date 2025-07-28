# Arduino как сервер

Arduino работает как веб-сервер и предоставляет API для управления и мониторинга.

## 📁 Файлы

- `main.cpp` - Код для Arduino (сервер)
- `main.py` - Python клиент для управления

## 🚀 Как это работает

### Схема работы
```
Python клиент → Ethernet → Arduino (сервер)
```

1. **Arduino** запускается как веб-сервер на порту 80
2. **Python клиент** подключается к Arduino
3. **Отправляет** HTTP запросы к API
4. **Arduino** обрабатывает запросы и отвечает

## 🔧 Настройка

### 1. Настройка IP адреса

**В Arduino (`main.cpp`):**
```cpp
IPAddress ip(192, 168, 1, 140);  // IP Arduino
```

**В Python (`main.py`):**
```python
client = ArduinoClient("192.168.1.140")  // IP Arduino
```

### 2. Загрузка кода на Arduino

```bash
cd Arduino_with_network
pio run --target upload
```

### 3. Запуск Python клиента

```bash
cd arduino_as_server
python3 main.py
```

## 📊 API Endpoints

### GET /api/status
**Назначение**: Получить статус системы
**Ответ**:
```json
{
  "uptime": 12345,
  "requests": 42,
  "bytes_sent": 1500,
  "bytes_received": 800
}
```

### GET /api/sensors
**Назначение**: Получить данные датчиков
**Ответ**:
```json
{
  "t": 22.5,
  "h": 45.0,
  "p": 1013.25,
  "timestamp": 12345
}
```

### GET /api/stress
**Назначение**: Стресс-тест с большими данными
**Ответ**: Большой JSON с массивом из 100 элементов

### GET /api/stats
**Назначение**: Статистика производительности
**Ответ**:
```json
{
  "uptime": 12345,
  "total_requests": 42,
  "requests_per_second": 1.2,
  "bytes_per_second": 500.5,
  "status_requests": 10,
  "sensor_requests": 15,
  "stress_requests": 5,
  "control_requests": 12
}
```

### GET /api/control
**Назначение**: Управление системой
**Параметры**:
- `cmd=on` - включить систему
- `cmd=off` - выключить систему
- `cmd=manual` - ручной режим
- `cmd=auto` - автоматический режим
- `power=N` - установить мощность (0-100)

**Ответ**:
```json
{
  "ok": true,
  "on": true,
  "manual": false,
  "power": 75
}
```

### POST /api/control
**Назначение**: Адаптивное тестирование
**Тело запроса**: JSON данные
**Ответ**:
```json
{
  "received": true,
  "data_size": 150,
  "timestamp": 12345,
  "message": "Data received successfully"
}
```

## 🔍 Мониторинг

### Serial Monitor Arduino
```
=== Arduino Sensor & Control Server ===
Initializing Ethernet...
Server IP: 192.168.1.140
API: /api/status, /api/sensors, /api/control
Client connected!
Request: GET /api/status HTTP/1.1
Client disconnected
```

### Python клиент
```
🌐 Arduino Sensor & Control Client
========================================
🔍 Проверка подключения к 192.168.1.140...
✅ Подключение успешно!

📊 СТАТУС СИСТЕМЫ
==================================================
🔄 Система: 🟢 ВКЛЮЧЕНА
⚙️  Режим: 🤖 АВТОМАТИЧЕСКИЙ
💪 Мощность: 75%
==================================================
```

## 🎮 Интерактивный режим

### Доступные команды
- `status` - показать статус системы
- `sensors` - показать данные датчиков
- `on` - включить систему
- `off` - выключить систему
- `manual` - ручной режим
- `auto` - автоматический режим
- `power N` - установить мощность N% (0-100)
- `monitor` - мониторинг датчиков
- `help` - показать справку
- `quit` - выйти

### Пример использования
```
🔧 Введите команду: sensors

📡 ДАННЫЕ ДАТЧИКОВ
==================================================
🌡️  Температура: 22.5°C
💧 Влажность: 45.0%
🌪️  Давление: 1013.25 hPa
==================================================

🔧 Введите команду: power 50
✅ Мощность установлена на 50%
```

## ⚙️ Настройка параметров

### Изменение порта
В `main.cpp`:
```cpp
EthernetServer server(80);  // Измените на нужный порт
```

### Изменение MAC адреса
```cpp
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
```

### Добавление новых endpoints
```cpp
else if (firstLine.indexOf(F("GET /api/new")) != -1) {
  sendNewData(client);
}
```

## 📈 Расширение функциональности

### Добавление реальных датчиков
```cpp
// В main.cpp
#include <DHT.h>

DHT dht(2, DHT11);  // Пин 2, DHT11

void setup() {
  dht.begin();
  // ... остальной код
}

void sendJsonSensors(EthernetClient &client) {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  client.print(F("{"));
  client.print(F("\"t\":")); client.print(temperature, 1);
  client.print(F(",\"h\":")); client.print(humidity, 1);
  client.println(F("}"));
}
```

### Добавление управления реле
```cpp
// В main.cpp
void handleControlCommand(EthernetClient &client, String request) {
  if (request.indexOf(F("cmd=relay1_on")) != -1) {
    digitalWrite(8, HIGH);
    Serial.println(F("Relay 1 ON"));
  }
  else if (request.indexOf(F("cmd=relay1_off")) != -1) {
    digitalWrite(8, LOW);
    Serial.println(F("Relay 1 OFF"));
  }
  // ... остальной код
}
```

### Добавление логирования
```cpp
// В main.cpp
void logEvent(String event) {
  Serial.print(F("["));
  Serial.print(millis());
  Serial.print(F("] "));
  Serial.println(event);
}
```

## 🐛 Устранение неполадок

### Arduino не отвечает
1. Проверьте IP адрес в Serial Monitor
2. Убедитесь, что Ethernet подключен
3. Проверьте, что сервер запущен
4. Перезагрузите Arduino

### Ошибки Python клиента
1. Установите зависимости: `pip install requests`
2. Проверьте IP адрес Arduino
3. Убедитесь, что сервер работает
4. Проверьте сетевое подключение

### Медленная работа
1. Уменьшите частоту запросов
2. Проверьте качество Ethernet соединения
3. Закройте другие программы
4. Перезагрузите роутер

## 🎯 Применение

### Домашняя автоматизация
- Управление освещением
- Мониторинг температуры
- Контроль влажности

### Робототехника
- Управление движением
- Мониторинг состояния
- Передача команд

### IoT проекты
- Удаленное управление
- Сбор данных
- Мониторинг систем

## 📚 Дополнительные ресурсы

- [Arduino Ethernet документация](https://www.arduino.cc/en/Reference/Ethernet)
- [HTTP протокол](https://developer.mozilla.org/en-US/docs/Web/HTTP)
- [JSON формат](https://www.json.org/)

---

**Автор**: Arduino Network Project  
**Версия**: 1.0  
**Дата**: 2024 