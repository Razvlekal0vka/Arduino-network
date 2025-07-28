# Arduino Network Project

Проект для создания сетевой инфраструктуры на базе Arduino с Ethernet Shield W5500. Включает в себя серверы, клиенты и системы мониторинга скорости передачи данных.

## 📁 Структура проекта

```
Arduino-network/
├── arduino_as_client/           # Arduino как клиент + Python сервер
├── arduino_as_server/           # Arduino как сервер + Python клиент
│   └── speed_monitor/          # Система мониторинга скорости
├── Arduino_with_network/        # PlatformIO проект
├── Project_in_KiCad/           # Схемы в KiCad
├── w5500_datasheet.pdf         # Документация W5500
└── arduino-ethernet-shield-v2_schematic.png
```

## 🚀 Быстрый старт

### 1. Подключение оборудования
- Arduino Uno + Ethernet Shield W5500
- Ethernet кабель
- USB кабель для программирования

### 2. Настройка сети
- Подключите Ethernet кабель к роутеру
- Убедитесь, что Arduino получает IP адрес
- Запишите IP адрес Arduino (обычно 192.168.1.140)

### 3. Загрузка кода
```bash
cd Arduino_with_network
pio run --target upload
```

### 4. Тестирование
```bash
cd arduino_as_server/speed_monitor
python3 simple_speed_test.py
```

## 📊 Компоненты системы

### 🔧 Arduino как сервер (`arduino_as_server/`)
- **Назначение**: Веб-сервер на Arduino для предоставления API
- **Функции**: 
  - Статус системы
  - Данные датчиков
  - Управление устройствами
  - Стресс-тестирование
- **API endpoints**:
  - `GET /api/status` - статус системы
  - `GET /api/sensors` - данные датчиков
  - `GET /api/stress` - стресс-тест
  - `GET /api/stats` - статистика
  - `POST /api/control` - адаптивное тестирование

### 📡 Arduino как клиент (`arduino_as_client/`)
- **Назначение**: Arduino подключается к Python серверу
- **Функции**: Отправка данных на сервер
- **Использование**: Для простых сценариев передачи данных

### 📈 Система мониторинга скорости (`arduino_as_server/speed_monitor/`)
- **Назначение**: Тестирование и анализ производительности сети
- **Компоненты**:
  - `arduino_speed_server.cpp` - Arduino сервер для тестирования
  - `python_speed_client.py` - Python клиент с графиками
  - `simple_speed_test.py` - Упрощенный тестер
  - `adaptive_speed_test.py` - Адаптивный тестер
  - `quick_adaptive_test.py` - Быстрый тест

### 🛠️ PlatformIO проект (`Arduino_with_network/`)
- **Назначение**: Основной проект для разработки
- **Функции**: Компиляция и загрузка кода на Arduino
- **Конфигурация**: `platformio.ini` с настройками для W5500

## 🎯 Применение в робототехнике

### 📊 Передача данных робота (50 байт)
```json
{
  "id": "robot1",
  "temp": 23.5,
  "batt": 87,
  "x": 125.3,
  "y": 67.8,
  "angle": 45.2,
  "mode": "patrol",
  "status": "moving",
  "time": 143025
}
```

### ⚡ Оптимальные параметры для робота
- **Размер данных**: 50-200 байт
- **Частота**: 0.5-2.0 запросов в секунду
- **Время ответа**: 50-200 мс
- **Надежность**: 95-99%

## 🔧 Настройка и конфигурация

### Изменение IP адреса
В файле `Arduino_with_network/src/main.cpp`:
```cpp
IPAddress ip(192, 168, 1, 140);  // Измените на ваш IP
```

### Настройка MAC адреса
```cpp
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
```

### Настройка порта
```cpp
EthernetServer server(80);  // HTTP порт
```

## 📈 Мониторинг и тестирование

### Простой тест скорости
```bash
cd arduino_as_server/speed_monitor
python3 simple_speed_test.py
```

### Мониторинг с графиками
```bash
python3 python_speed_client.py
```

### Адаптивное тестирование
```bash
python3 adaptive_speed_test.py
```

### Быстрый тест
```bash
python3 quick_adaptive_test.py
```

## 📊 Метрики производительности

### Время ответа
- **Отлично**: < 100 мс
- **Хорошо**: 100-300 мс
- **Приемлемо**: 300-500 мс
- **Плохо**: > 500 мс

### Скорость передачи
- **Отлично**: > 1000 байт/сек
- **Хорошо**: 500-1000 байт/сек
- **Приемлемо**: 100-500 байт/сек
- **Плохо**: < 100 байт/сек

### Надежность
- **Отлично**: > 99%
- **Хорошо**: 95-99%
- **Приемлемо**: 90-95%
- **Плохо**: < 90%

## 🐛 Устранение неполадок

### Arduino не подключается к сети
1. Проверьте Ethernet кабель
2. Убедитесь, что DHCP работает
3. Проверьте IP адрес в Serial Monitor
4. Перезагрузите Arduino

### Ошибки Python клиента
1. Установите зависимости: `pip install requests matplotlib numpy`
2. Проверьте IP адрес Arduino
3. Убедитесь, что сервер работает
4. Проверьте сетевое подключение

### Медленная работа
1. Уменьшите частоту запросов
2. Проверьте качество Ethernet соединения
3. Закройте другие программы
4. Перезагрузите роутер

## 📚 Документация

### Основные файлы
- `arduino_as_server/main.cpp` - Основной сервер Arduino
- `arduino_as_server/main.py` - Python клиент
- `Arduino_with_network/platformio.ini` - Конфигурация PlatformIO

### Тестирование
- `arduino_as_server/speed_monitor/README.md` - Документация системы мониторинга
- `Arduino_with_network/PLATFORMIO_COMMANDS.md` - Команды PlatformIO

## 🤝 Вклад в проект

1. Форкните репозиторий
2. Создайте ветку для новой функции
3. Внесите изменения
4. Создайте Pull Request

## 📄 Лицензия

Этот проект распространяется под лицензией MIT.