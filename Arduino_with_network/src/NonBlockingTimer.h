#ifndef NONBLOCKING_TIMER_H
#define NONBLOCKING_TIMER_H

#include <Arduino.h>

class NonBlockingTimer {
private:
  unsigned long previousMillis;
  unsigned long interval;
  bool running;
  
public:
  NonBlockingTimer(unsigned long interval_ms = 1000) {
    this->interval = interval_ms;
    this->previousMillis = 0;
    this->running = false;
  }
  
  void start() {
    running = true;
    previousMillis = millis();
  }
  
  void stop() {
    running = false;
  }
  
  void reset() {
    previousMillis = millis();
  }
  
  void setInterval(unsigned long interval_ms) {
    interval = interval_ms;
  }
  
  bool isReady() {
    if (!running) return false;
    
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      return true;
    }
    return false;
  }
  
  bool isRunning() {
    return running;
  }
  
  unsigned long getElapsed() {
    return millis() - previousMillis;
  }
  
  unsigned long getRemaining() {
    if (!running) return 0;
    unsigned long elapsed = millis() - previousMillis;
    return (elapsed >= interval) ? 0 : (interval - elapsed);
  }
};

class NonBlockingDelay {
private:
  unsigned long startTime;
  unsigned long delayTime;
  bool active;
  
public:
  NonBlockingDelay() {
    active = false;
  }
  
  void start(unsigned long delay_ms) {
    startTime = millis();
    delayTime = delay_ms;
    active = true;
  }
  
  bool isFinished() {
    if (!active) return true;
    
    if (millis() - startTime >= delayTime) {
      active = false;
      return true;
    }
    return false;
  }
  
  void cancel() {
    active = false;
  }
  
  bool isActive() {
    return active;
  }
  
  unsigned long getRemaining() {
    if (!active) return 0;
    unsigned long elapsed = millis() - startTime;
    return (elapsed >= delayTime) ? 0 : (delayTime - elapsed);
  }
};

#endif 