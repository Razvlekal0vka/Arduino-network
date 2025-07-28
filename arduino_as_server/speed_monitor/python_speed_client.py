#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Python клиент для стресс-тестирования Arduino сервера
с графиками в реальном времени
"""

import requests
import json
import time
import threading
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import numpy as np
from collections import deque
import tkinter as tk
from tkinter import ttk
import queue
import signal
import sys

# Настройка стиля графиков
style.use('dark_background')
plt.rcParams['figure.figsize'] = (12, 8)

class SpeedMonitor:
    def __init__(self, max_points=100):
        self.max_points = max_points
        self.response_times = deque(maxlen=max_points)
        self.bytes_sent = deque(maxlen=max_points)
        self.bytes_received = deque(maxlen=max_points)
        self.requests_per_second = deque(maxlen=max_points)
        self.timestamps = deque(maxlen=max_points)
        
        # Статистика
        self.total_requests = 0
        self.total_bytes_sent = 0
        self.total_bytes_received = 0
        self.start_time = time.time()
        
        # Очередь для передачи данных между потоками
        self.data_queue = queue.Queue()
        
        # Флаги управления
        self.running = False
        self.test_mode = "status"  # status, sensors, stress, mixed
        
    def add_data_point(self, response_time, bytes_sent, bytes_received, requests_per_sec):
        """Добавляет точку данных"""
        timestamp = time.time() - self.start_time
        self.timestamps.append(timestamp)
        self.response_times.append(response_time)
        self.bytes_sent.append(bytes_sent)
        self.bytes_received.append(bytes_received)
        self.requests_per_second.append(requests_per_sec)
        
        # Обновляем общую статистику
        self.total_requests += 1
        self.total_bytes_sent += bytes_sent
        self.total_bytes_received += bytes_received

class ArduinoSpeedClient:
    def __init__(self, ip="192.168.1.140", port=80):
        self.base_url = f"http://{ip}:{port}"
        self.session = requests.Session()
        self.timeout = 2  # Уменьшил timeout
        
        # Настройки сессии
        self.session.headers.update({
            'User-Agent': 'ArduinoSpeedClient/1.0',
            'Accept': 'application/json, text/html, */*',
            'Connection': 'close'
        })
        
        # Монитор скорости
        self.monitor = SpeedMonitor()
        
        # Настройки тестирования
        self.test_intervals = {
            "status": 0.1,    # 10 запросов в секунду
            "sensors": 0.2,   # 5 запросов в секунду
            "stress": 1.0,    # 1 запрос в секунду
            "mixed": 0.5,     # 2 запроса в секунду
            "post": 0.3,      # 3 POST запроса в секунду
            "full": 0.4       # 2.5 запроса в секунду (смешанные)
        }
        
    def make_request(self, endpoint):
        """Выполняет запрос к Arduino серверу"""
        try:
            start_time = time.time()
            
            # Используем stream=True для больших ответов
            response = self.session.get(
                f"{self.base_url}{endpoint}", 
                timeout=self.timeout,
                stream=True
            )
            
            # Читаем только заголовки и первые байты
            content_length = response.headers.get('content-length')
            if content_length:
                content_length = int(content_length)
            else:
                content_length = 0
                
            # Читаем только первые 1024 байта
            content = response.raw.read(1024)
            end_time = time.time()
            
            response_time = (end_time - start_time) * 1000  # в миллисекундах
            
            # Правильный подсчет отправленных байт (включая заголовки)
            request_line = f"GET {endpoint} HTTP/1.1\r\n"
            headers = ""
            for key, value in response.request.headers.items():
                headers += f"{key}: {value}\r\n"
            headers += "\r\n"  # Пустая строка после заголовков
            
            bytes_sent = len(request_line.encode('utf-8')) + len(headers.encode('utf-8'))
            bytes_received = len(content) + (content_length - 1024 if content_length > 1024 else 0)
            
            return {
                'success': response.status_code == 200,
                'response_time': response_time,
                'bytes_sent': bytes_sent,
                'bytes_received': bytes_received,
                'status_code': response.status_code,
                'data': None  # Убираем парсинг JSON для больших ответов
            }
        except requests.exceptions.Timeout:
            return {
                'success': False,
                'response_time': self.timeout * 1000,
                'bytes_sent': 0,
                'bytes_received': 0,
                'status_code': 0,
                'error': 'Timeout'
            }
        except requests.exceptions.ConnectionError:
            return {
                'success': False,
                'response_time': 0,
                'bytes_sent': 0,
                'bytes_received': 0,
                'status_code': 0,
                'error': 'Connection Error'
            }
        except Exception as e:
            return {
                'success': False,
                'response_time': 0,
                'bytes_sent': 0,
                'bytes_received': 0,
                'status_code': 0,
                'error': str(e)
            }
    
    def make_post_request(self, endpoint, data):
        """Выполняет POST запрос с данными"""
        try:
            start_time = time.time()
            
            # Отправляем POST запрос с данными
            response = self.session.post(
                f"{self.base_url}{endpoint}",
                json=data,
                timeout=self.timeout,
                stream=True
            )
            
            # Читаем ответ
            content_length = response.headers.get('content-length')
            if content_length:
                content_length = int(content_length)
            else:
                content_length = 0
                
            content = response.raw.read(1024)
            end_time = time.time()
            
            response_time = (end_time - start_time) * 1000
            
            # Подсчет отправленных байт для POST запроса
            request_line = f"POST {endpoint} HTTP/1.1\r\n"
            headers = ""
            for key, value in response.request.headers.items():
                headers += f"{key}: {value}\r\n"
            headers += "\r\n"
            
            # Добавляем размер тела запроса
            post_data = json.dumps(data)
            bytes_sent = len(request_line.encode('utf-8')) + len(headers.encode('utf-8')) + len(post_data.encode('utf-8'))
            bytes_received = len(content) + (content_length - 1024 if content_length > 1024 else 0)
            
            return {
                'success': response.status_code == 200,
                'response_time': response_time,
                'bytes_sent': bytes_sent,
                'bytes_received': bytes_received,
                'status_code': response.status_code,
                'data': None
            }
        except Exception as e:
            return {
                'success': False,
                'response_time': 0,
                'bytes_sent': 0,
                'bytes_received': 0,
                'status_code': 0,
                'error': str(e)
            }
    
    def stress_test_worker(self, test_mode, duration=None):
        """Рабочий поток для стресс-тестирования"""
        endpoints = {
            "status": "/api/status",
            "sensors": "/api/sensors", 
            "mixed": ["/api/status", "/api/sensors"],
            "post": "/api/robot",  # POST запросы для отправки данных
            "full": ["/api/status", "/api/sensors", "/api/robot"]  # Полный набор
        }
        
        interval = self.test_intervals[test_mode]
        start_time = time.time()
        request_count = 0
        
        # Данные для POST запросов
        robot_data = {
            "id": "test_robot",
            "temperature": 22.5,
            "battery": 85,
            "x": 0.0,
            "y": 0.0,
            "angle": 0.0,
            "mode": "test",
            "status": "testing"
        }
        
        while self.monitor.running:
            if duration and (time.time() - start_time) > duration:
                break
                
            # Выбираем endpoint и тип запроса
            if test_mode == "mixed":
                endpoint = np.random.choice(endpoints[test_mode])
                result = self.make_request(endpoint)
            elif test_mode == "post":
                # Обновляем данные робота для разнообразия
                robot_data["temperature"] = 20 + np.random.random() * 10
                robot_data["battery"] = 80 + np.random.random() * 20
                robot_data["x"] = np.random.random() * 10 - 5
                robot_data["y"] = np.random.random() * 10 - 5
                robot_data["angle"] = np.random.random() * 360
                result = self.make_post_request(endpoints[test_mode], robot_data)
            elif test_mode == "full":
                # Случайный выбор между GET и POST
                if np.random.random() < 0.7:  # 70% GET запросов
                    endpoint = np.random.choice(endpoints[test_mode][:2])  # Только GET endpoints
                    result = self.make_request(endpoint)
                else:  # 30% POST запросов
                    robot_data["temperature"] = 20 + np.random.random() * 10
                    robot_data["battery"] = 80 + np.random.random() * 20
                    robot_data["x"] = np.random.random() * 10 - 5
                    robot_data["y"] = np.random.random() * 10 - 5
                    robot_data["angle"] = np.random.random() * 360
                    result = self.make_post_request(endpoints[test_mode][2], robot_data)
            else:
                endpoint = endpoints[test_mode]
                result = self.make_request(endpoint)
            
            request_count += 1
            
            # Вычисляем запросов в секунду
            elapsed = time.time() - start_time
            requests_per_sec = request_count / elapsed if elapsed > 0 else 0
            
            # Добавляем данные в монитор
            self.monitor.add_data_point(
                result['response_time'],
                result['bytes_sent'],
                result['bytes_received'],
                requests_per_sec
            )
            
            # Отправляем данные в очередь для графика
            try:
                self.monitor.data_queue.put({
                    'timestamp': time.time() - self.monitor.start_time,
                    'response_time': result['response_time'],
                    'bytes_sent': result['bytes_sent'],
                    'bytes_received': result['bytes_received'],
                    'requests_per_sec': requests_per_sec,
                    'success': result['success']
                }, timeout=0.1)  # Добавил timeout для очереди
            except queue.Full:
                pass  # Игнорируем если очередь полная
            
            time.sleep(interval)
    
    def start_stress_test(self, test_mode="mixed", duration=None):
        """Запускает стресс-тест"""
        print(f"🚀 Запуск стресс-теста: {test_mode}")
        print(f"⏱️  Интервал: {self.test_intervals[test_mode]:.2f} сек")
        if duration:
            print(f"⏰ Продолжительность: {duration} сек")
        
        self.monitor.running = True
        self.monitor.test_mode = test_mode
        
        # Запускаем рабочий поток
        worker_thread = threading.Thread(
            target=self.stress_test_worker,
            args=(test_mode, duration)
        )
        worker_thread.daemon = True
        worker_thread.start()
        
        return worker_thread
    
    def stop_stress_test(self):
        """Останавливает стресс-тест"""
        self.monitor.running = False
        print("⏹️  Стресс-тест остановлен")
    
    def get_server_stats(self):
        """Получает статистику сервера"""
        return self.make_request("/api/stats")
    
    def print_final_stats(self):
        """Выводит финальную статистику"""
        elapsed = time.time() - self.monitor.start_time
        
        print("\n" + "="*60)
        print("📊 ФИНАЛЬНАЯ СТАТИСТИКА СТРЕСС-ТЕСТА")
        print("="*60)
        print(f"⏱️  Время тестирования: {elapsed:.2f} сек")
        print(f"📡 Всего запросов: {self.monitor.total_requests}")
        print(f"📤 Всего байт отправлено: {self.monitor.total_bytes_sent:,}")
        print(f"📥 Всего байт получено: {self.monitor.total_bytes_received:,}")
        
        if elapsed > 0:
            avg_requests_per_sec = self.monitor.total_requests / elapsed
            avg_bytes_per_sec = (self.monitor.total_bytes_sent + self.monitor.total_bytes_received) / elapsed
            print(f"⚡ Средняя скорость запросов: {avg_requests_per_sec:.2f} запр/сек")
            print(f"💾 Средняя скорость передачи: {avg_bytes_per_sec:.2f} байт/сек")
            print(f"💾 Средняя скорость передачи: {avg_bytes_per_sec/1024:.2f} Кбит/сек")
        
        if self.monitor.response_times:
            avg_response_time = np.mean(self.monitor.response_times)
            min_response_time = np.min(self.monitor.response_times)
            max_response_time = np.max(self.monitor.response_times)
            print(f"⏳ Среднее время ответа: {avg_response_time:.2f} мс")
            print(f"⏳ Минимальное время ответа: {min_response_time:.2f} мс")
            print(f"⏳ Максимальное время ответа: {max_response_time:.2f} мс")
        
        print("="*60)

class RealTimePlotter:
    def __init__(self, monitor):
        self.monitor = monitor
        self.fig, self.axes = plt.subplots(2, 2, figsize=(15, 10))
        self.fig.suptitle('Arduino Speed Test Monitor', fontsize=16)
        
        # Настройка графиков
        self.setup_plots()
        
        # Обработка сигналов
        signal.signal(signal.SIGINT, self.signal_handler)
        
    def signal_handler(self, signum, frame):
        """Обработчик сигналов для graceful shutdown"""
        print("\n🛑 Получен сигнал прерывания. Завершение работы...")
        self.monitor.running = False
        plt.close('all')
        
    def setup_plots(self):
        """Настройка графиков"""
        # График времени ответа
        self.axes[0, 0].set_title('Время ответа (мс)')
        self.axes[0, 0].set_ylabel('Время (мс)')
        self.axes[0, 0].grid(True, alpha=0.3)
        
        # График скорости передачи
        self.axes[0, 1].set_title('Скорость передачи (байт/сек)')
        self.axes[0, 1].set_ylabel('Байт/сек')
        self.axes[0, 1].grid(True, alpha=0.3)
        
        # График запросов в секунду
        self.axes[1, 0].set_title('Запросов в секунду')
        self.axes[1, 0].set_ylabel('Запросы/сек')
        self.axes[1, 0].grid(True, alpha=0.3)
        
        # График объема данных
        self.axes[1, 1].set_title('Объем данных')
        self.axes[1, 1].set_ylabel('Байты')
        self.axes[1, 1].grid(True, alpha=0.3)
        
        plt.tight_layout()
    
    def update_plots(self, frame):
        """Обновление графиков"""
        try:
            # Проверяем, нужно ли остановиться
            if not self.monitor.running:
                plt.close('all')
                return self.axes.flat
            
            # Очищаем графики
            for ax in self.axes.flat:
                ax.clear()
            
            # Настраиваем графики заново
            self.setup_plots()
            
            if len(self.monitor.timestamps) > 0:
                timestamps = list(self.monitor.timestamps)
                response_times = list(self.monitor.response_times)
                bytes_sent = list(self.monitor.bytes_sent)
                bytes_received = list(self.monitor.bytes_received)
                requests_per_sec = list(self.monitor.requests_per_second)
                
                # График времени ответа
                self.axes[0, 0].plot(timestamps, response_times, 'r-', linewidth=2, label='Время ответа')
                self.axes[0, 0].plot(timestamps, response_times, 'ro', markersize=4)
                
                # График скорости передачи
                total_bytes = [s + r for s, r in zip(bytes_sent, bytes_received)]
                self.axes[0, 1].plot(timestamps, total_bytes, 'g-', linewidth=2, label='Скорость передачи')
                self.axes[0, 1].plot(timestamps, total_bytes, 'go', markersize=4)
                
                # График запросов в секунду
                self.axes[1, 0].plot(timestamps, requests_per_sec, 'b-', linewidth=2, label='Запросы/сек')
                self.axes[1, 0].plot(timestamps, requests_per_sec, 'bo', markersize=4)
                
                # График объема данных
                cumulative_sent = np.cumsum(bytes_sent)
                cumulative_received = np.cumsum(bytes_received)
                self.axes[1, 1].plot(timestamps, cumulative_sent, 'y-', linewidth=2, label='Отправлено')
                self.axes[1, 1].plot(timestamps, cumulative_received, 'm-', linewidth=2, label='Получено')
                self.axes[1, 1].plot(timestamps, cumulative_sent, 'yo', markersize=4)
                self.axes[1, 1].plot(timestamps, cumulative_received, 'mo', markersize=4)
                
                # Добавляем легенды
                for ax in self.axes.flat:
                    ax.legend()
        
        except Exception as e:
            print(f"Ошибка обновления графиков: {e}")
        
        return self.axes.flat
    
    def start_animation(self):
        """Запускает анимацию графиков"""
        ani = animation.FuncAnimation(
            self.fig, self.update_plots, interval=1000, blit=False
        )
        plt.show()
        return ani

def main():
    print("🚀 Arduino Speed Test Client")
    print("="*50)
    
    # Настройка IP адреса
    ip = input("Введите IP адрес Arduino (по умолчанию 192.168.1.140): ").strip()
    if not ip:
        ip = "192.168.1.140"
    
    client = ArduinoSpeedClient(ip)
    
    # Проверка подключения
    print(f"\n🔍 Проверка подключения к {ip}...")
    stats = client.get_server_stats()
    
    if stats['success']:
        print("✅ Подключение успешно!")
        
        # Выбор режима тестирования
        print("\n🎯 Выберите режим тестирования:")
        print("1. status - быстрые запросы статуса")
        print("2. sensors - запросы датчиков")
        print("3. mixed - смешанный режим (GET)")
        print("4. post - POST запросы с данными робота")
        print("5. full - полный режим (GET + POST)")
        
        mode_choice = input("Введите номер режима (1-5): ").strip()
        modes = {"1": "status", "2": "sensors", "3": "mixed", "4": "post", "5": "full"}
        test_mode = modes.get(mode_choice, "mixed")
        
        # Продолжительность теста
        duration_input = input("Продолжительность теста в секундах (Enter = бесконечно): ").strip()
        duration = float(duration_input) if duration_input else None
        
        # Запуск теста
        print(f"\n🚀 Запуск теста: {test_mode}")
        worker_thread = client.start_stress_test(test_mode, duration)
        
        # Запуск графиков
        plotter = RealTimePlotter(client.monitor)
        print("📊 Запуск графиков в реальном времени...")
        print("💡 Нажмите Ctrl+C для остановки")
        
        try:
            plotter.start_animation()
        except KeyboardInterrupt:
            print("\n⏹️  Остановка теста...")
            client.stop_stress_test()
            client.print_final_stats()
        
    else:
        print("❌ Не удалось подключиться к Arduino")
        print("Проверьте IP адрес и работу сервера")

if __name__ == "__main__":
    main() 