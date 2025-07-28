# PlatformIO Команды

## Активация PlatformIO

Перед использованием команд PlatformIO необходимо активировать виртуальное окружение:

```bash
source ~/.platformio/penv/bin/activate
```

## Основные команды

### Сборка проекта
```bash
source ~/.platformio/penv/bin/activate
pio run
```
Компилирует проект без загрузки на Arduino.

### Загрузка на Arduino
```bash
source ~/.platformio/penv/bin/activate
pio run --target upload
```
Компилирует и загружает код на Arduino.

### Очистка проекта
```bash
pio run --target clean
```
Удаляет все скомпилированные файлы.

### Мониторинг Serial порта
```bash
pio device monitor
```
Открывает монитор последовательного порта для просмотра вывода Arduino.

### Список устройств
```bash
pio device list
```
Показывает все доступные устройства для загрузки.

## Тестирование

### Запуск тестов
```bash
pio test
```
Запускает все тесты в проекте.

### Запуск тестов с подробным выводом
```bash
pio test -vvv
```
Запускает тесты с максимальной детализацией.

### Запуск конкретного теста
```bash
pio test -f test_main
```
Запускает только тест `test_main`.

## Полезные команды

### Обновление PlatformIO
```bash
pio update
```
Обновляет PlatformIO и все инструменты.

### Установка библиотеки
```bash
pio lib install "Ethernet2"
```
Устанавливает библиотеку Ethernet2.

### Поиск библиотек
```bash
pio lib search "ethernet"
```
Ищет библиотеки по ключевому слову.

### Информация о проекте
```bash
pio project info
```
Показывает информацию о текущем проекте.

## Примеры использования

### Полный цикл разработки
```bash
# 1. Сборка проекта
pio run

# 2. Загрузка на Arduino
pio run --target upload

# 3. Мониторинг вывода
pio device monitor
```

### Отладка проблем
```bash
# 1. Очистка проекта
pio run --target clean

# 2. Сборка с подробным выводом
pio run -vvv

# 3. Загрузка с подробным выводом
pio run --target upload -vvv
```

## Настройки в platformio.ini

```ini
[env:uno]
platform = atmelavr
board = uno
framework = arduino

; Библиотеки
lib_deps = 
    Ethernet2

; Скорости
upload_speed = 115200
monitor_speed = 9600

; Флаги компиляции
build_flags = 
    -D ARDUINO_AVR_UNO

; Настройки монитора
monitor_filters = 
    esp32_exception_decoder
    time
    colorize
```

## Решение проблем

### Ошибка "Nothing to build"
- Убедитесь, что код находится в папке `src/`
- Проверьте, что файл `main.cpp` существует

### Ошибка загрузки
- Проверьте подключение Arduino
- Убедитесь, что выбрана правильная плата
- Попробуйте другую скорость загрузки

### Ошибка библиотек
- Установите недостающие библиотеки: `pio lib install "имя_библиотеки"`
- Проверьте совместимость версий 