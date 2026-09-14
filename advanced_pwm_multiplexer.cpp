#include <Arduino.h>

// Конфигурация мультиплексора CD4051
const int MUX_A = 2;      // A0 - младший бит
const int MUX_B = 3;      // A1 - средний бит  
const int MUX_C = 4;      // A2 - старший бит
const int MUX_IN = 5;     // Вход мультиплексора (ШИМ сигнал)

// ШИМ пины Arduino (можно использовать несколько)
const int PWM_PIN_1 = 9;  // Основной ШИМ выход
const int PWM_PIN_2 = 10; // Дополнительный ШИМ выход

// Состояние каналов
int channelValues[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // 0-255 для каждого канала
bool channelActive[8] = {false, false, false, false, false, false, false, false};

// Таймер для мультиплексирования
unsigned long lastSwitchTime = 0;
const unsigned long SWITCH_INTERVAL = 1000; // 1 мс между переключениями

// Режимы работы
enum MultiplexerMode {
  SINGLE_CHANNEL,    // Один канал за раз
  MULTI_CHANNEL,     // Быстрое переключение между каналами
  SIMULTANEOUS       // Имитация одновременного управления
};

MultiplexerMode currentMode = MULTI_CHANNEL;

void setup() {
  Serial.begin(9600);
  Serial.println(F("=== Продвинутый ШИМ Мультиплексор ==="));
  
  // Настройка пинов мультиплексора
  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);
  pinMode(MUX_IN, OUTPUT);
  
  // Настройка ШИМ
  pinMode(PWM_PIN_1, OUTPUT);
  pinMode(PWM_PIN_2, OUTPUT);
  
  // Инициализация каналов
  initializeChannels();
  
  Serial.println(F("Мультиплексор инициализирован"));
  Serial.println(F("Команды:"));
  Serial.println(F("  c0-c7 - выбрать канал"));
  Serial.println(F("  v0-v255 - установить значение"));
  Serial.println(F("  a0-a7 - активировать канал"));
  Serial.println(F("  d0-d7 - деактивировать канал"));
  Serial.println(F("  m0-m2 - режим работы (0-одиночный, 1-мульти, 2-одновременный)"));
  Serial.println(F("  s - показать статус"));
  Serial.println(F("  t - тест всех каналов"));
}

void loop() {
  // Обработка команд через Serial
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    processCommand(command);
  }
  
  // Обновление ШИМ в зависимости от режима
  switch (currentMode) {
    case SINGLE_CHANNEL:
      updateSingleChannel();
      break;
    case MULTI_CHANNEL:
      updateMultiChannel();
      break;
    case SIMULTANEOUS:
      updateSimultaneous();
      break;
  }
  
  delay(1); // Минимальная задержка
}

void processCommand(String command) {
  if (command.startsWith("c")) {
    // Выбор канала
    int channel = command.substring(1).toInt();
    if (channel >= 0 && channel < 8) {
      setActiveChannel(channel);
      Serial.print(F("Канал выбран: "));
      Serial.println(channel);
    }
  }
  else if (command.startsWith("v")) {
    // Установка значения
    int value = command.substring(1).toInt();
    if (value >= 0 && value <= 255) {
      channelValues[getActiveChannel()] = value;
      Serial.print(F("Канал "));
      Serial.print(getActiveChannel());
      Serial.print(F(" установлен в: "));
      Serial.println(value);
    }
  }
  else if (command.startsWith("a")) {
    // Активация канала
    int channel = command.substring(1).toInt();
    if (channel >= 0 && channel < 8) {
      channelActive[channel] = true;
      Serial.print(F("Канал "));
      Serial.print(channel);
      Serial.println(F(" активирован"));
    }
  }
  else if (command.startsWith("d")) {
    // Деактивация канала
    int channel = command.substring(1).toInt();
    if (channel >= 0 && channel < 8) {
      channelActive[channel] = false;
      Serial.print(F("Канал "));
      Serial.print(channel);
      Serial.println(F(" деактивирован"));
    }
  }
  else if (command.startsWith("m")) {
    // Смена режима
    int mode = command.substring(1).toInt();
    if (mode >= 0 && mode <= 2) {
      currentMode = (MultiplexerMode)mode;
      Serial.print(F("Режим изменен на: "));
      Serial.println(mode);
    }
  }
  else if (command == "s") {
    showStatus();
  }
  else if (command == "t") {
    testAllChannels();
  }
}

void updateSingleChannel() {
  // Простое управление одним каналом
  int activeChannel = getActiveChannel();
  setMultiplexerChannel(activeChannel);
  analogWrite(PWM_PIN_1, channelValues[activeChannel]);
}

void updateMultiChannel() {
  // Быстрое переключение между активными каналами
  static int currentChannelIndex = 0;
  
  if (millis() - lastSwitchTime >= SWITCH_INTERVAL) {
    // Ищем следующий активный канал
    int nextChannel = findNextActiveChannel(currentChannelIndex);
    if (nextChannel != -1) {
      setMultiplexerChannel(nextChannel);
      analogWrite(PWM_PIN_1, channelValues[nextChannel]);
      currentChannelIndex = nextChannel;
    }
    lastSwitchTime = millis();
  }
}

void updateSimultaneous() {
  // Имитация одновременного управления через быстрое переключение
  static int channelIndex = 0;
  
  if (millis() - lastSwitchTime >= 100) { // Более быстрое переключение
    if (channelActive[channelIndex]) {
      setMultiplexerChannel(channelIndex);
      analogWrite(PWM_PIN_1, channelValues[channelIndex]);
    }
    
    channelIndex = (channelIndex + 1) % 8;
    lastSwitchTime = millis();
  }
}

void setMultiplexerChannel(int channel) {
  // Устанавливаем адрес мультиплексора
  digitalWrite(MUX_A, channel & 0x01);
  digitalWrite(MUX_B, (channel >> 1) & 0x01);
  digitalWrite(MUX_C, (channel >> 2) & 0x01);
  
  // Небольшая задержка для стабилизации
  delayMicroseconds(50);
}

int getActiveChannel() {
  // Возвращает первый активный канал
  for (int i = 0; i < 8; i++) {
    if (channelActive[i]) return i;
  }
  return 0; // По умолчанию канал 0
}

int findNextActiveChannel(int startChannel) {
  // Ищет следующий активный канал после startChannel
  for (int i = 1; i <= 8; i++) {
    int channel = (startChannel + i) % 8;
    if (channelActive[channel]) return channel;
  }
  return -1; // Нет активных каналов
}

void initializeChannels() {
  // Инициализация каналов случайными значениями
  for (int i = 0; i < 8; i++) {
    channelValues[i] = random(0, 256);
    channelActive[i] = true; // Все каналы активны по умолчанию
  }
}

void showStatus() {
  Serial.println(F("\n=== СТАТУС МУЛЬТИПЛЕКСОРА ==="));
  Serial.print(F("Режим работы: "));
  switch (currentMode) {
    case SINGLE_CHANNEL: Serial.println(F("Одиночный канал")); break;
    case MULTI_CHANNEL: Serial.println(F("Мультиплексирование")); break;
    case SIMULTANEOUS: Serial.println(F("Одновременный")); break;
  }
  
  Serial.println(F("Каналы:"));
  for (int i = 0; i < 8; i++) {
    Serial.print(F("  Канал "));
    Serial.print(i);
    Serial.print(F(": "));
    Serial.print(channelValues[i]);
    Serial.print(F(" ("));
    Serial.print(map(channelValues[i], 0, 255, 0, 100));
    Serial.print(F("%) "));
    Serial.println(channelActive[i] ? F("[АКТИВЕН]") : F("[НЕАКТИВЕН]"));
  }
  Serial.println(F("================================"));
}

void testAllChannels() {
  Serial.println(F("Тестирование всех каналов..."));
  
  // Активируем все каналы
  for (int i = 0; i < 8; i++) {
    channelActive[i] = true;
  }
  
  // Тест с разными режимами
  currentMode = MULTI_CHANNEL;
  delay(2000);
  
  currentMode = SIMULTANEOUS;
  delay(2000);
  
  currentMode = SINGLE_CHANNEL;
  Serial.println(F("Тестирование завершено"));
}

// Дополнительные функции для расширенного управления

void fadeAllChannels(int startValue, int endValue, int duration) {
  int steps = abs(endValue - startValue);
  int delayTime = duration / steps;
  
  for (int i = 0; i <= steps; i++) {
    int value = map(i, 0, steps, startValue, endValue);
    
    for (int ch = 0; ch < 8; ch++) {
      if (channelActive[ch]) {
        channelValues[ch] = value;
      }
    }
    
    delay(delayTime);
  }
}

void setChannelPattern(int pattern[8]) {
  for (int i = 0; i < 8; i++) {
    channelValues[i] = constrain(pattern[i], 0, 255);
  }
}

void createWavePattern() {
  // Создание волнового паттерна
  for (int i = 0; i < 8; i++) {
    int value = map(sin(i * PI / 4) * 255, -255, 255, 0, 255);
    channelValues[i] = value;
  }
} 