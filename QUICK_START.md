# 🚀 Быстрый старт - Arduino Network

## 📋 Что нужно

- Arduino Uno
- Ethernet Shield W5500
- Ethernet кабель
- USB кабель

## ⚡ Быстрая настройка (5 минут)

### 1. Подключение
```
Arduino Uno ←→ Ethernet Shield W5500 ←→ Ethernet кабель ←→ Роутер
```

### 2. Загрузка кода
```bash
cd Arduino_with_network
pio run --target upload
```

### 3. Проверка работы
```bash
pio device monitor
```
Должно появиться:
```
=== Arduino Speed Test Server ===
Server IP: 192.168.1.140
```

### 4. Тест скорости
```bash
cd arduino_as_server/speed_monitor
python3 quick_adaptive_test.py
```

## 🎯 Что получится

### Веб-сервер на Arduino
- IP: `http://192.168.1.140`
- API: `/api/status`, `/api/sensors`, `/api/stress`
- Статистика производительности

### Python клиент
- Интерактивное управление
- Мониторинг датчиков
- Стресс-тестирование

### Система мониторинга
- Графики в реальном времени
- Анализ производительности
- Рекомендации для робота

## 📊 Пример для робота

### Передача данных (50 байт)
```json
{
  "id": "robot1",
  "temp": 23.5,
  "batt": 87,
  "x": 125.3,
  "y": 67.8,
  "angle": 45.2,
  "mode": "patrol",
  "status": "moving"
}
```

### Оптимальные параметры
- **Размер**: 50-200 байт
- **Частота**: 0.5-2.0 запр/сек
- **Время ответа**: 50-200 мс
- **Надежность**: 95-99%

## 🔧 Настройка IP

В `Arduino_with_network/src/main.cpp`:
```cpp
IPAddress ip(192, 168, 1, 140);  // Измените на ваш IP
```

## 🐛 Если не работает

1. **Arduino не подключается**
   - Проверьте Ethernet кабель
   - Перезагрузите Arduino
   - Проверьте IP в Serial Monitor

2. **Python ошибки**
   - `pip install requests matplotlib numpy`
   - Проверьте IP адрес
   - Убедитесь, что сервер работает

3. **Медленная работа**
   - Уменьшите частоту запросов
   - Проверьте качество Ethernet
   - Закройте другие программы

## 📚 Документация

- `README.md` - Основная документация
- `arduino_as_server/README.md` - Arduino сервер
- `arduino_as_client/README.md` - Arduino клиент
- `Arduino_with_network/README.md` - PlatformIO проект
- `arduino_as_server/speed_monitor/README.md` - Мониторинг

## 🎮 Команды

### Основные
```bash
# Загрузка кода
pio run --target upload

# Мониторинг
pio device monitor

# Тест скорости
python3 simple_speed_test.py

# Графики
python3 python_speed_client.py

# Адаптивный тест
python3 adaptive_speed_test.py
```

### Python клиент
```bash
cd arduino_as_server
python3 main.py
```
Команды: `status`, `sensors`, `on`, `off`, `power 50`, `monitor`

---

**Время настройки**: 5 минут  
**Сложность**: Начинающий  
**Применение**: Робототехника, IoT, обучение 