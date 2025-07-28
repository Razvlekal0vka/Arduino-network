#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Рабочий Python клиент для Arduino Sensor & Control
"""

import requests
import json
import time
from datetime import datetime


class ArduinoClient:
    def __init__(self, ip="192.168.1.140", port=80):
        self.base_url = f"http://{ip}:{port}"
        self.session = requests.Session()
        self.timeout = 3

        # Настройки для работы с локальной сетью
        self.session.headers.update({
            'User-Agent': 'ArduinoClient/1.0',
            'Accept': 'application/json, text/html, */*',
            'Connection': 'close'
        })

    def get_status(self):
        """Получить статус системы"""
        try:
            response = self.session.get(f"{self.base_url}/api/status", timeout=self.timeout)
            if response.status_code == 200:
                return response.json()
            else:
                print(f"❌ Ошибка получения статуса: {response.status_code}")
                return None
        except Exception as e:
            print(f"❌ Ошибка подключения: {e}")
            return None

    def get_sensors(self):
        """Получить данные датчиков"""
        try:
            response = self.session.get(f"{self.base_url}/api/sensors", timeout=self.timeout)
            if response.status_code == 200:
                return response.json()
            else:
                print(f"❌ Ошибка получения датчиков: {response.status_code}")
                return None
        except Exception as e:
            print(f"❌ Ошибка подключения: {e}")
            return None

    def send_command(self, cmd=None, power=None):
        """Отправить команду управления"""
        try:
            params = {}
            if cmd:
                params['cmd'] = cmd
            if power is not None:
                params['power'] = power

            if not params:
                print("❌ Не указаны параметры команды")
                return None

            response = self.session.get(f"{self.base_url}/api/control", params=params, timeout=self.timeout)
            if response.status_code == 200:
                return response.json()
            else:
                print(f"❌ Ошибка отправки команды: {response.status_code}")
                return None
        except Exception as e:
            print(f"❌ Ошибка подключения: {e}")
            return None

    def system_on(self):
        """Включить систему"""
        return self.send_command(cmd="on")

    def system_off(self):
        """Выключить систему"""
        return self.send_command(cmd="off")

    def set_manual_mode(self):
        """Установить ручной режим"""
        return self.send_command(cmd="manual")

    def set_auto_mode(self):
        """Установить автоматический режим"""
        return self.send_command(cmd="auto")

    def set_power(self, power):
        """Установить мощность (0-100%)"""
        if 0 <= power <= 100:
            return self.send_command(power=power)
        else:
            print("❌ Мощность должна быть от 0 до 100%")
            return None

    def print_status(self, status):
        """Красиво вывести статус"""
        if not status:
            return

        print("\n" + "=" * 50)
        print("📊 СТАТУС СИСТЕМЫ")
        print("=" * 50)
        print(f"🔄 Система: {'🟢 ВКЛЮЧЕНА' if status.get('on') else '🔴 ВЫКЛЮЧЕНА'}")
        print(f"⚙️  Режим: {'🔧 РУЧНОЙ' if status.get('manual') else '🤖 АВТОМАТИЧЕСКИЙ'}")
        print(f"💪 Мощность: {status.get('power', 0)}%")
        print("=" * 50)

    def print_sensors(self, sensors):
        """Красиво вывести данные датчиков"""
        if not sensors:
            return

        print("\n" + "=" * 50)
        print("📡 ДАННЫЕ ДАТЧИКОВ")
        print("=" * 50)
        print(f"🌡️  Температура: {sensors.get('t', 0):.1f}°C")
        print(f"💧 Влажность: {sensors.get('h', 0):.1f}%")
        print(f"🌪️  Давление: {sensors.get('p', 0):.2f} hPa")
        print("=" * 50)

    def monitor_sensors(self, interval=2, duration=None):
        """Мониторинг датчиков в реальном времени"""
        print(f"\n🔍 Запуск мониторинга датчиков (интервал: {interval} сек)")
        if duration:
            print(f"⏰ Продолжительность: {duration} сек")
        print("Для остановки нажмите Ctrl+C")
        print("-" * 50)

        start_time = time.time()
        try:
            while True:
                if duration and (time.time() - start_time) > duration:
                    break

                sensors = self.get_sensors()
                if sensors:
                    timestamp = datetime.now().strftime("%H:%M:%S")
                    print(f"[{timestamp}] 🌡️ {sensors.get('t', 0):.1f}°C | "
                          f"💧 {sensors.get('h', 0):.1f}% | "
                          f"🌪️ {sensors.get('p', 0):.2f} hPa")

                time.sleep(interval)

        except KeyboardInterrupt:
            print("\n⏹️  Мониторинг остановлен")

    def interactive_mode(self):
        """Интерактивный режим управления"""
        print("\n🎮 ИНТЕРАКТИВНЫЙ РЕЖИМ УПРАВЛЕНИЯ")
        print("=" * 50)
        print("Доступные команды:")
        print("  status    - показать статус системы")
        print("  sensors   - показать данные датчиков")
        print("  on        - включить систему")
        print("  off       - выключить систему")
        print("  manual    - ручной режим")
        print("  auto      - автоматический режим")
        print("  power N   - установить мощность N% (0-100)")
        print("  monitor   - мониторинг датчиков")
        print("  help      - показать эту справку")
        print("  quit      - выйти")
        print("=" * 50)

        while True:
            try:
                command = input("\n🔧 Введите команду: ").strip().lower()

                if command == "quit" or command == "exit":
                    print("👋 До свидания!")
                    break
                elif command == "help":
                    print("Доступные команды:")
                    print("  status    - показать статус системы")
                    print("  sensors   - показать данные датчиков")
                    print("  on        - включить систему")
                    print("  off       - выключить систему")
                    print("  manual    - ручной режим")
                    print("  auto      - автоматический режим")
                    print("  power N   - установить мощность N% (0-100)")
                    print("  monitor   - мониторинг датчиков")
                    print("  help      - показать эту справку")
                    print("  quit      - выйти")
                elif command == "status":
                    status = self.get_status()
                    self.print_status(status)
                elif command == "sensors":
                    sensors = self.get_sensors()
                    self.print_sensors(sensors)
                elif command == "on":
                    result = self.system_on()
                    if result:
                        print("✅ Система включена")
                        self.print_status(result)
                elif command == "off":
                    result = self.system_off()
                    if result:
                        print("✅ Система выключена")
                        self.print_status(result)
                elif command == "manual":
                    result = self.set_manual_mode()
                    if result:
                        print("✅ Установлен ручной режим")
                        self.print_status(result)
                elif command == "auto":
                    result = self.set_auto_mode()
                    if result:
                        print("✅ Установлен автоматический режим")
                        self.print_status(result)
                elif command.startswith("power "):
                    try:
                        power = int(command.split()[1])
                        result = self.set_power(power)
                        if result:
                            print(f"✅ Мощность установлена на {power}%")
                            self.print_status(result)
                    except (IndexError, ValueError):
                        print("❌ Укажите мощность: power N (где N = 0-100)")
                elif command == "monitor":
                    try:
                        duration = input("⏰ Продолжительность мониторинга (сек, Enter = бесконечно): ").strip()
                        duration = int(duration) if duration else None
                        self.monitor_sensors(duration=duration)
                    except ValueError:
                        print("❌ Неверный формат времени")
                else:
                    print("❌ Неизвестная команда. Введите 'help' для справки")

            except KeyboardInterrupt:
                print("\n👋 До свидания!")
                break
            except Exception as e:
                print(f"❌ Ошибка: {e}")


def main():
    print("🌐 Arduino Sensor & Control Client")
    print("=" * 40)

    # Настройка IP адреса
    ip = input("Введите IP адрес Arduino (по умолчанию 192.168.1.140): ").strip()
    if not ip:
        ip = "192.168.1.140"

    client = ArduinoClient(ip)

    # Проверка подключения
    print(f"\n🔍 Проверка подключения к {ip}...")
    status = client.get_status()
    if status:
        print("✅ Подключение успешно!")
        client.print_status(status)

        # Показать данные датчиков
        sensors = client.get_sensors()
        if sensors:
            client.print_sensors(sensors)

        # Запуск интерактивного режима
        client.interactive_mode()
    else:
        print("❌ Не удалось подключиться к Arduino")
        print("Проверьте:")
        print("  - IP адрес Arduino")
        print("  - Подключение к сети")
        print("  - Работу Arduino сервера")
        print("\n💡 Попробуйте открыть в браузере:")
        print(f"   http://{ip}")


if __name__ == "__main__":
    main()