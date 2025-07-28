# Arduino как клиент

Простая система, где Arduino работает как клиент и подключается к Python серверу.

## 📁 Файлы

- `arduino_client.cpp` - Код для Arduino (клиент)
- `python_server.py` - Python сервер для приема данных

## 🚀 Как это работает

### Схема работы
```
Arduino (клиент) → Ethernet → Python сервер
```

1. **Arduino** подключается к Python серверу
2. **Отправляет** данные каждые 5 секунд
3. **Python сервер** принимает и обрабатывает данные
4. **Отправляет** ответ обратно на Arduino

## 🔧 Настройка

### 1. Настройка IP адресов

**В Arduino (`arduino_client.cpp`):**
```cpp
IPAddress serverIP(192, 168, 1, 103);  // IP вашего компьютера
IPAddress ip(192, 168, 1, 140);        // IP Arduino
```

**В Python (`python_server.py`):**
```python
HOST = '192.168.1.103'  # IP вашего компьютера
PORT = 8080
```

### 2. Загрузка кода на Arduino

1. Откройте `arduino_client.cpp` в Arduino IDE
2. Подключите Arduino к компьютеру
3. Выберите правильную плату и порт
4. Загрузите код

### 3. Запуск Python сервера

```bash
cd arduino_as_client
python3 python_server.py
```

## 📊 Что передается

### Данные от Arduino
```
"Hello from Arduino!"
```

### Ответ от сервера
```
"Hello from Python server!"
```

## 🔍 Мониторинг

### Serial Monitor Arduino
```
=== Arduino Client Test ===
My IP address: 192.168.1.140
=============================
Trying to connect to server...
Connected to server!
Hello from Python server!
Disconnected from server
```

### Python сервер
```
Сервер запущен на 192.168.1.103:8080
Ожидание подключений...
Подключение от ('192.168.1.140', 12345)
Получено: Hello from Arduino!
Отправлено: Hello from Python server!
Клиент отключен
```

## ⚙️ Настройка параметров

### Изменение интервала отправки
В `arduino_client.cpp`:
```cpp
delay(5000);  // Измените на нужный интервал (в миллисекундах)
```

### Изменение порта
В `python_server.py`:
```python
PORT = 8080  // Измените на нужный порт
```

### Изменение сообщения
В `arduino_client.cpp`:
```cpp
client.println("Hello from Arduino!");  // Измените сообщение
```

## 🐛 Устранение неполадок

### Arduino не подключается
1. Проверьте IP адрес сервера
2. Убедитесь, что Python сервер запущен
3. Проверьте сетевое подключение
4. Проверьте порт (8080)

### Ошибки Python сервера
1. Убедитесь, что порт 8080 свободен
2. Проверьте права доступа
3. Остановите другие серверы на этом порту

### Медленная работа
1. Увеличьте интервал между запросами
2. Проверьте качество Ethernet соединения
3. Закройте другие программы

## 📈 Расширение функциональности

### Добавление датчиков
```cpp
// В arduino_client.cpp
float temperature = analogRead(A0) * 0.488;  // LM35
int humidity = analogRead(A1) / 10;          // DHT11

String message = "Temp:" + String(temperature) + 
                ",Hum:" + String(humidity);
client.println(message);
```

### Обработка команд от сервера
```cpp
// В arduino_client.cpp
while (client.available()) {
    char c = client.read();
    if (c == 'L') {
        digitalWrite(13, HIGH);  // Включить LED
    } else if (c == 'l') {
        digitalWrite(13, LOW);   // Выключить LED
    }
}
```

### Сохранение данных в файл
```python
# В python_server.py
import json
from datetime import datetime

data = {
    'timestamp': datetime.now().isoformat(),
    'message': decoded_data.strip(),
    'client_ip': address[0]
}

with open('arduino_data.json', 'a') as f:
    json.dump(data, f)
    f.write('\n')
```

## 🎯 Применение

### Простые сценарии
- Отправка данных с датчиков
- Удаленное управление устройствами
- Мониторинг состояния системы

### Робототехника
- Передача позиции робота
- Отправка данных с камеры
- Управление движением

### IoT проекты
- Мониторинг температуры/влажности
- Управление освещением
- Сбор данных с сенсоров

## 📚 Дополнительные ресурсы

- [Ethernet2 библиотека](https://github.com/adafruit/Ethernet2)
- [Arduino Ethernet документация](https://www.arduino.cc/en/Reference/Ethernet)
- [Python socket документация](https://docs.python.org/3/library/socket.html)

---

**Автор**: Arduino Network Project  
**Версия**: 1.0  
**Дата**: 2024 