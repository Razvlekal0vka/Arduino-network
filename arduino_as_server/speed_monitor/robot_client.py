#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Python клиент для двусторонней передачи данных с роботом
Отправляет данные робота на Arduino и получает ответы
"""

import requests
import json
import time
import threading
from datetime import datetime
import random
import signal
import sys

class RobotClient:
    def __init__(self, ip="192.168.1.140", port=80):
        self.base_url = f"http://{ip}:{port}"
        self.session = requests.Session()
        self.timeout = 3
        
        # Настройки сессии
        self.session.headers.update({
            'User-Agent': 'RobotClient/1.0',
            'Content-Type': 'application/json',
            'Accept': 'application/json'
        })
        
        # Данные робота
        self.robot_data = {
            "id": "robot1",
            "temp": 22.5,
            "batt": 87,
            "x": 0.0,
            "y": 0.0,
            "angle": 0.0,
            "mode": "idle",
            "status": "ready",
            "command": ""
        }
        
        # Статистика
        self.total_requests = 0
        self.successful_requests = 0
        self.failed_requests = 0
        self.start_time = time.time()
        
        # Флаги управления
        self.running = False
        
        # Обработка сигналов
        signal.signal(signal.SIGINT, self.signal_handler)
        
    def signal_handler(self, signum, frame):
        """Обработчик сигналов для graceful shutdown"""
        print("\n🛑 Получен сигнал прерывания. Завершение работы...")
        self.running = False
        
    def update_robot_data(self):
        """Обновляет данные робота"""
        # Симулируем движение робота
        self.robot_data["x"] += random.uniform(-0.5, 0.5)
        self.robot_data["y"] += random.uniform(-0.5, 0.5)
        self.robot_data["angle"] += random.uniform(-5, 5)
        if self.robot_data["angle"] > 360:
            self.robot_data["angle"] -= 360
        elif self.robot_data["angle"] < 0:
            self.robot_data["angle"] += 360
            
        # Симулируем изменение температуры
        self.robot_data["temp"] += random.uniform(-0.2, 0.2)
        self.robot_data["temp"] = max(18.0, min(28.0, self.robot_data["temp"]))
        
        # Симулируем изменение батареи
        self.robot_data["batt"] += random.uniform(-0.1, 0.1)
        self.robot_data["batt"] = max(10, min(100, self.robot_data["batt"]))
        
        # Симулируем изменение режима
        modes = ["idle", "patrol", "charging", "maintenance"]
        if random.random() < 0.01:  # 1% шанс смены режима
            self.robot_data["mode"] = random.choice(modes)
            
        # Симулируем изменение статуса
        statuses = ["ready", "moving", "stopped", "error"]
        if random.random() < 0.02:  # 2% шанс смены статуса
            self.robot_data["status"] = random.choice(statuses)
    
    def send_robot_data(self, command=""):
        """Отправляет данные робота на Arduino"""
        try:
            self.update_robot_data()
            
            # Добавляем команду если указана
            if command:
                self.robot_data["command"] = command
            else:
                self.robot_data["command"] = ""
            
            start_time = time.time()
            response = self.session.post(
                f"{self.base_url}/api/robot",
                json=self.robot_data,
                timeout=self.timeout
            )
            end_time = time.time()
            
            response_time = (end_time - start_time) * 1000
            
            if response.status_code == 200:
                self.successful_requests += 1
                response_data = response.json()
                
                print(f"🤖 Робот {self.robot_data['id']}: "
                      f"Позиция({self.robot_data['x']:.1f}, {self.robot_data['y']:.1f}) "
                      f"Угол: {self.robot_data['angle']:.1f}° "
                      f"Темп: {self.robot_data['temp']:.1f}°C "
                      f"Батарея: {self.robot_data['batt']:.1f}% "
                      f"Режим: {self.robot_data['mode']} "
                      f"Статус: {self.robot_data['status']}")
                
                if response_data.get("system_status"):
                    status = response_data["system_status"]
                    print(f"📡 Ответ сервера: Система {'ВКЛ' if status.get('on') else 'ВЫКЛ'} | "
                          f"Режим: {'Ручной' if status.get('manual') else 'Авто'} | "
                          f"Мощность: {status.get('power', 0)}% | "
                          f"LED: {'ВКЛ' if status.get('led') else 'ВЫКЛ'}")
                
                return {
                    'success': True,
                    'response_time': response_time,
                    'data': response_data
                }
            else:
                self.failed_requests += 1
                return {
                    'success': False,
                    'response_time': response_time,
                    'error': f"HTTP {response.status_code}"
                }
                
        except requests.exceptions.Timeout:
            self.failed_requests += 1
            return {
                'success': False,
                'response_time': self.timeout * 1000,
                'error': 'Timeout'
            }
        except Exception as e:
            self.failed_requests += 1
            return {
                'success': False,
                'response_time': 0,
                'error': str(e)
            }
    
    def send_command(self, command, **kwargs):
        """Отправляет команду роботу"""
        command_data = self.robot_data.copy()
        command_data["command"] = command
        
        # Добавляем дополнительные параметры
        for key, value in kwargs.items():
            command_data[key] = value
            
        try:
            response = self.session.post(
                f"{self.base_url}/api/robot",
                json=command_data,
                timeout=self.timeout
            )
            
            if response.status_code == 200:
                print(f"✅ Команда '{command}' выполнена успешно")
                return response.json()
            else:
                print(f"❌ Ошибка выполнения команды '{command}': HTTP {response.status_code}")
                return None
                
        except Exception as e:
            print(f"❌ Ошибка отправки команды '{command}': {e}")
            return None
    
    def start_continuous_mode(self, interval=2.0):
        """Запускает непрерывный режим отправки данных"""
        print(f"🚀 Запуск непрерывного режима отправки данных робота")
        print(f"⏱️  Интервал: {interval:.1f} сек")
        print("💡 Нажмите Ctrl+C для остановки")
        print("-" * 60)
        
        self.running = True
        self.total_requests = 0
        self.successful_requests = 0
        self.failed_requests = 0
        self.start_time = time.time()
        
        while self.running:
            result = self.send_robot_data()
            self.total_requests += 1
            
            if result['success']:
                print(f"⏳ Время ответа: {result['response_time']:.1f} мс")
            else:
                print(f"❌ Ошибка: {result.get('error', 'Unknown error')}")
            
            print("-" * 40)
            time.sleep(interval)
    
    def interactive_mode(self):
        """Интерактивный режим управления роботом"""
        print("🎮 Интерактивный режим управления роботом")
        print("Доступные команды:")
        print("  led_on - включить LED")
        print("  led_off - выключить LED")
        print("  system_on - включить систему")
        print("  system_off - выключить систему")
        print("  manual_mode - ручной режим")
        print("  auto_mode - автоматический режим")
        print("  set_power N - установить мощность N%")
        print("  get_status - получить статус")
        print("  send_data - отправить данные робота")
        print("  quit - выйти")
        print("-" * 50)
        
        while True:
            try:
                command = input("🤖 Введите команду: ").strip().lower()
                
                if command == "quit":
                    break
                elif command == "send_data":
                    self.send_robot_data()
                elif command == "led_on":
                    self.send_command("led_on")
                elif command == "led_off":
                    self.send_command("led_off")
                elif command == "system_on":
                    self.send_command("system_on")
                elif command == "system_off":
                    self.send_command("system_off")
                elif command == "manual_mode":
                    self.send_command("manual_mode")
                elif command == "auto_mode":
                    self.send_command("auto_mode")
                elif command.startswith("set_power"):
                    try:
                        power = int(command.split()[1])
                        self.send_command("set_power", power=power)
                    except (IndexError, ValueError):
                        print("❌ Укажите мощность: set_power N")
                elif command == "get_status":
                    self.send_command("get_status")
                else:
                    print("❌ Неизвестная команда")
                    
            except KeyboardInterrupt:
                print("\n🛑 Выход из интерактивного режима")
                break
    
    def print_stats(self):
        """Выводит статистику"""
        elapsed = time.time() - self.start_time
        
        print("\n" + "="*60)
        print("📊 СТАТИСТИКА РОБОТА")
        print("="*60)
        print(f"⏱️  Время работы: {elapsed:.2f} сек")
        print(f"📡 Всего запросов: {self.total_requests}")
        print(f"✅ Успешных: {self.successful_requests}")
        print(f"❌ Неудачных: {self.failed_requests}")
        
        if self.total_requests > 0:
            success_rate = (self.successful_requests / self.total_requests) * 100
            requests_per_sec = self.total_requests / elapsed
            print(f"📈 Процент успеха: {success_rate:.1f}%")
            print(f"⚡ Запросов в секунду: {requests_per_sec:.2f}")
        
        print("="*60)

def main():
    print("🤖 Robot Client - Двусторонняя передача данных")
    print("="*50)
    
    # Настройка IP адреса
    ip = input("Введите IP адрес Arduino (по умолчанию 192.168.1.140): ").strip()
    if not ip:
        ip = "192.168.1.140"
    
    client = RobotClient(ip)
    
    # Проверка подключения
    print(f"\n🔍 Проверка подключения к {ip}...")
    test_result = client.send_robot_data()
    
    if test_result['success']:
        print("✅ Подключение успешно!")
        
        # Выбор режима
        print("\n🎯 Выберите режим:")
        print("1. Непрерывная отправка данных")
        print("2. Интерактивное управление")
        
        choice = input("Введите номер (1-2): ").strip()
        
        if choice == "1":
            interval = float(input("Интервал отправки в секундах (по умолчанию 2.0): ") or "2.0")
            try:
                client.start_continuous_mode(interval)
            except KeyboardInterrupt:
                print("\n⏹️  Остановка непрерывного режима")
            finally:
                client.print_stats()
        elif choice == "2":
            client.interactive_mode()
        else:
            print("❌ Неверный выбор")
    else:
        print("❌ Не удалось подключиться к Arduino")
        print("Проверьте IP адрес и работу сервера")

if __name__ == "__main__":
    main() 