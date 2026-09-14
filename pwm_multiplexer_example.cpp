#include <Arduino.h>

// Конфигурация мультиплексора CD4051
const int MUX_A = 2;      // A0 - младший бит
const int MUX_B = 3;      // A1 - средний бит  
const int MUX_C = 4;      // A2 - старший бит
const int MUX_IN = 5;     // Вход мультиплексора (ШИМ сигнал)
const int MUX_OUT = 6;    // Выход мультиплексора (к нагрузке)

// ШИМ пины Arduino
const int PWM_PIN = 9;    // ШИМ выход Arduino

// Состояние каналов
int channelValues[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // 0-255 для каждого канала
int currentChannel = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("=== ШИМ Мультиплексор ==="));
  
  // Настройка пинов мультиплексора
  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);
  pinMode(MUX_IN, OUTPUT);
  pinMode(MUX_OUT, INPUT);  // Для чтения состояния
  
  // Настройка ШИМ
  pinMode(PWM_PIN, OUTPUT);
  
  // Инициализация каналов
  for (int i = 0; i < 8; i++) {
    channelValues[i] = random(0, 256); // Случайные значения
  }
  
  Serial.println(F("Мультиплексор инициализирован"));
  Serial.println(F("Команды:"));
  Serial.println(F("  c0-c7 - выбрать канал"));
  Serial.println(F("  v0-v255 - установить значение"));
  Serial.println(F("  s - показать статус"));
}

void loop() {
  // Обработка команд через Serial
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    if (command.startsWith("c")) {
      // Выбор канала
      int channel = command.substring(1).toInt();
      if (channel >= 0 && channel < 8) {
        currentChannel = channel;
        Serial.print(F("Канал выбран: "));
        Serial.println(currentChannel);
      }
    }
    else if (command.startsWith("v")) {
      // Установка значения
      int value = command.substring(1).toInt();
      if (value >= 0 && value <= 255) {
        channelValues[currentChannel] = value;
        Serial.print(F("Канал "));
        Serial.print(currentChannel);
        Serial.print(F(" установлен в: "));
        Serial.println(value);
      }
    }
    else if (command == "s") {
      // Показать статус
      showStatus();
    }
  }
  
  // Обновление ШИМ для текущего канала
  updatePWM();
  
  delay(10); // Небольшая задержка
}

void updatePWM() {
  // Устанавливаем адрес мультиплексора
  digitalWrite(MUX_A, currentChannel & 0x01);
  digitalWrite(MUX_B, (currentChannel >> 1) & 0x01);
  digitalWrite(MUX_C, (currentChannel >> 2) & 0x01);
  
  // Устанавливаем ШИМ значение
  analogWrite(PWM_PIN, channelValues[currentChannel]);
  
  // Небольшая задержка для стабилизации
  delayMicroseconds(100);
}

void showStatus() {
  Serial.println(F("\n=== СТАТУС МУЛЬТИПЛЕКСОРА ==="));
  Serial.print(F("Текущий канал: "));
  Serial.println(currentChannel);
  Serial.println(F("Значения каналов:"));
  
  for (int i = 0; i < 8; i++) {
    Serial.print(F("  Канал "));
    Serial.print(i);
    Serial.print(F(": "));
    Serial.print(channelValues[i]);
    Serial.print(F(" ("));
    Serial.print(map(channelValues[i], 0, 255, 0, 100));
    Serial.println(F("%)"));
  }
  Serial.println(F("================================"));
}

// Дополнительные функции для расширенного управления

void setAllChannels(int value) {
  for (int i = 0; i < 8; i++) {
    channelValues[i] = constrain(value, 0, 255);
  }
  Serial.print(F("Все каналы установлены в: "));
  Serial.println(value);
}

void fadeChannel(int channel, int startValue, int endValue, int duration) {
  if (channel < 0 || channel >= 8) return;
  
  int steps = abs(endValue - startValue);
  int delayTime = duration / steps;
  
  for (int i = 0; i <= steps; i++) {
    int value = map(i, 0, steps, startValue, endValue);
    channelValues[channel] = value;
    
    // Временно переключаемся на этот канал
    int oldChannel = currentChannel;
    currentChannel = channel;
    updatePWM();
    currentChannel = oldChannel;
    
    delay(delayTime);
  }
}

void testAllChannels() {
  Serial.println(F("Тестирование всех каналов..."));
  
  for (int ch = 0; ch < 8; ch++) {
    Serial.print(F("Тест канала "));
    Serial.println(ch);
    
    // Плавное включение
    fadeChannel(ch, 0, 255, 1000);
    delay(500);
    
    // Плавное выключение
    fadeChannel(ch, 255, 0, 1000);
    delay(500);
  }
  
  Serial.println(F("Тестирование завершено"));
} 