# Arduino Network - PlatformIO проект

PlatformIO проект для разработки и загрузки кода на Arduino с Ethernet Shield W5500.

## 📁 Структура проекта

```
Arduino_with_network/
├── src/
│   └── main.cpp              # Основной код Arduino
├── include/                  # Заголовочные файлы
├── lib/                      # Библиотеки
├── test/                     # Тесты
├── platformio.ini           # Конфигурация PlatformIO
└── PLATFORMIO_COMMANDS.md   # Команды PlatformIO
```

## 🚀 Быстрый старт

### 1. Установка PlatformIO

```bash
# Через pip
pip install platformio

# Или через Homebrew (macOS)
brew install platformio
```

### 2. Клонирование проекта

```bash
git clone <repository-url>
cd Arduino_with_network
```

### 3. Компиляция и загрузка

```bash
# Компиляция
pio run

# Загрузка на Arduino
pio run --target upload

# Компиляция и загрузка одной командой
pio run --target upload
```

### 4. Мониторинг Serial

```bash
pio device monitor
```

## ⚙️ Конфигурация

### platformio.ini
```ini
[env:uno]
platform = atmelavr
board = uno
framework = arduino

; Настройки для Ethernet Shield W5500
lib_deps = 
    Ethernet2

; Настройки загрузки
upload_speed = 115200
monitor_speed = 9600

; Дополнительные флаги компиляции
build_flags = 
    -D ARDUINO_AVR_UNO
```

### Настройка IP адреса
В `src/main.cpp`:
```cpp
IPAddress ip(192, 168, 1, 140);  // Измените на ваш IP
```

### Настройка MAC адреса
```cpp
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
```

## 🔧 Команды PlatformIO

### Основные команды
```bash
# Компиляция
pio run

# Загрузка
pio run --target upload

# Очистка
pio run --target clean

# Мониторинг Serial
pio device monitor

# Список устройств
pio device list

# Информация о проекте
pio project info
```

### Отладка
```bash
# Компиляция с отладкой
pio run --target debug

# Запуск отладчика
pio debug
```

### Тестирование
```bash
# Запуск тестов
pio test

# Запуск конкретного теста
pio test --filter test_main
```

## 📊 Функциональность

### Веб-сервер на Arduino
- HTTP сервер на порту 80
- REST API endpoints
- JSON ответы
- Статистика производительности

### API Endpoints
- `GET /api/status` - статус системы
- `GET /api/sensors` - данные датчиков
- `GET /api/stress` - стресс-тест
- `GET /api/stats` - статистика
- `GET /api/control` - управление
- `POST /api/control` - адаптивное тестирование

### Мониторинг
- Время работы системы
- Количество запросов
- Объем переданных данных
- Статистика по типам запросов

## 🔍 Отладка

### Serial Monitor
```bash
pio device monitor
```

### Логи компиляции
```bash
pio run -v
```

### Проверка размера кода
```bash
pio run --target size
```

### Анализ зависимостей
```bash
pio run --target check
```

## 📈 Производительность

### Использование памяти
- **RAM**: ~22% (451 байт из 2048)
- **Flash**: ~74% (23850 байт из 32256)

### Оптимизация
- Использование `F()` для строк в Flash
- Минимизация использования RAM
- Оптимизация HTTP ответов

## 🐛 Устранение неполадок

### Ошибки компиляции
1. Проверьте версию PlatformIO
2. Убедитесь, что все библиотеки установлены
3. Проверьте синтаксис кода
4. Очистите проект: `pio run --target clean`

### Ошибки загрузки
1. Проверьте подключение Arduino
2. Убедитесь, что выбрана правильная плата
3. Проверьте драйверы USB
4. Попробуйте другой USB кабель

### Ошибки мониторинга
1. Проверьте скорость Serial (9600 baud)
2. Убедитесь, что порт не занят
3. Перезагрузите Arduino
4. Проверьте права доступа к порту

## 📚 Библиотеки

### Основные библиотеки
- **Ethernet2** - для работы с Ethernet Shield W5500
- **SPI** - для связи с W5500
- **Arduino** - основная библиотека

### Установка библиотек
```bash
# Установка библиотеки
pio lib install "Ethernet2"

# Установка конкретной версии
pio lib install "Ethernet2@1.0.4"

# Удаление библиотеки
pio lib uninstall "Ethernet2"
```

## 🎯 Применение

### Робототехника
- Удаленное управление роботом
- Передача данных с датчиков
- Мониторинг состояния

### IoT проекты
- Домашняя автоматизация
- Промышленный мониторинг
- Умные устройства

### Обучение
- Изучение сетевых протоколов
- Работа с HTTP API
- Программирование Arduino

## 📝 Разработка

### Структура кода
```cpp
// Включения
#include <SPI.h>
#include <Ethernet2.h>

// Константы
#define SERVER_PORT 80

// Глобальные переменные
EthernetServer server(SERVER_PORT);

// Функции
void setup() {
    // Инициализация
}

void loop() {
    // Основной цикл
}
```

### Добавление новых функций
1. Создайте функцию в `src/main.cpp`
2. Добавьте обработку в `loop()`
3. Протестируйте через Serial Monitor
4. Загрузите на Arduino

## 📚 Дополнительные ресурсы

- [PlatformIO документация](https://docs.platformio.org/)
- [Arduino документация](https://www.arduino.cc/reference/)
- [Ethernet2 библиотека](https://github.com/adafruit/Ethernet2)
- [HTTP протокол](https://developer.mozilla.org/en-US/docs/Web/HTTP)

## 🤝 Вклад в проект

1. Форкните репозиторий
2. Создайте ветку для новой функции
3. Внесите изменения
4. Протестируйте: `pio test`
5. Создайте Pull Request

---

**Автор**: Arduino Network Project  
**Версия**: 1.0  
**Дата**: 2024 