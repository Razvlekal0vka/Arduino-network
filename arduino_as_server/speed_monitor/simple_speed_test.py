#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Упрощенный клиент для тестирования скорости Arduino сервера
"""

import requests
import time
import threading
from datetime import datetime
import statistics
import signal
import sys

class SimpleSpeedTest:
    def __init__(self, ip="192.168.1.140", port=80):
        self.base_url = f"http://{ip}:{port}"
        self.session = requests.Session()
        self.timeout = 2  # Уменьшил timeout с 5 до 2 секунд
        
        # Настройки сессии
        self.session.headers.update({
            'User-Agent': 'ArduinoSpeedTest/1.0',
            'Accept': 'application/json, text/html, */*',
            'Connection': 'close'
        })
        
        # Статистика
        self.response_times = []
        self.bytes_sent = []
        self.bytes_received = []
        self.successful_requests = 0
        self.failed_requests = 0
        self.start_time = None
        self.running = True
        
        # Обработка сигналов для graceful shutdown
        signal.signal(signal.SIGINT, self.signal_handler)
        
    def signal_handler(self, signum, frame):
        """Обработчик сигналов для graceful shutdown"""
        print("\n🛑 Получен сигнал прерывания. Завершение работы...")
        self.running = False
        
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
            
            # Читаем только заголовки и первые байты для определения размера
            content_length = response.headers.get('content-length')
            if content_length:
                content_length = int(content_length)
            else:
                content_length = 0
                
            # Читаем только первые 1024 байта для определения успешности
            content = response.raw.read(1024)
            end_time = time.time()
            
            response_time = (end_time - start_time) * 1000  # в миллисекундах
            bytes_sent = len(response.request.body) if response.request.body else 0
            bytes_received = len(content) + (content_length - 1024 if content_length > 1024 else 0)
            
            return {
                'success': response.status_code == 200,
                'response_time': response_time,
                'bytes_sent': bytes_sent,
                'bytes_received': bytes_received,
                'status_code': response.status_code
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
    
    def run_test(self, endpoint, num_requests=100, interval=0.1):
        """Запускает тест с указанными параметрами"""
        print(f"🚀 Запуск теста: {endpoint}")
        print(f"📊 Количество запросов: {num_requests}")
        print(f"⏱️  Интервал: {interval:.2f} сек")
        print("-" * 50)
        
        self.start_time = time.time()
        self.response_times = []
        self.bytes_sent = []
        self.bytes_received = []
        self.successful_requests = 0
        self.failed_requests = 0
        
        for i in range(num_requests):
            if not self.running:
                print("\n🛑 Тест прерван пользователем")
                break
                
            result = self.make_request(endpoint)
            
            if result['success']:
                self.successful_requests += 1
                self.response_times.append(result['response_time'])
                self.bytes_sent.append(result['bytes_sent'])
                self.bytes_received.append(result['bytes_received'])
                
                # Выводим прогресс каждые 10 запросов
                if (i + 1) % 10 == 0:
                    elapsed = time.time() - self.start_time
                    requests_per_sec = (i + 1) / elapsed
                    avg_response_time = statistics.mean(self.response_times)
                    print(f"📈 Прогресс: {i + 1}/{num_requests} | "
                          f"Скорость: {requests_per_sec:.2f} запр/сек | "
                          f"Среднее время ответа: {avg_response_time:.2f} мс")
            else:
                self.failed_requests += 1
                if result.get('error') != 'Timeout':  # Не показываем timeout ошибки
                    print(f"❌ Ошибка запроса {i + 1}: {result.get('error', 'Unknown error')}")
            
            time.sleep(interval)
        
        self.print_results()
    
    def run_stress_test(self, duration=60):
        """Запускает стресс-тест на указанное время"""
        print(f"🔥 Запуск стресс-теста на {duration} секунд")
        print("-" * 50)
        
        endpoints = ["/api/status", "/api/sensors"]  # Убрал /api/stress из-за больших данных
        self.start_time = time.time()
        self.response_times = []
        self.bytes_sent = []
        self.bytes_received = []
        self.successful_requests = 0
        self.failed_requests = 0
        
        request_count = 0
        start_time = time.time()
        
        while time.time() - start_time < duration and self.running:
            # Выбираем случайный endpoint
            import random
            endpoint = random.choice(endpoints)
            
            result = self.make_request(endpoint)
            request_count += 1
            
            if result['success']:
                self.successful_requests += 1
                self.response_times.append(result['response_time'])
                self.bytes_sent.append(result['bytes_sent'])
                self.bytes_received.append(result['bytes_received'])
            else:
                self.failed_requests += 1
            
            # Выводим статистику каждые 10 секунд
            if request_count % 50 == 0:
                elapsed = time.time() - start_time
                requests_per_sec = request_count / elapsed
                if self.response_times:
                    avg_response_time = statistics.mean(self.response_times)
                    print(f"📊 Запросов: {request_count} | "
                          f"Скорость: {requests_per_sec:.2f} запр/сек | "
                          f"Среднее время ответа: {avg_response_time:.2f} мс")
            
            time.sleep(0.1)  # 10 запросов в секунду
        
        if not self.running:
            print("\n🛑 Стресс-тест прерван пользователем")
        
        self.print_results()
    
    def print_results(self):
        """Выводит результаты теста"""
        if not self.response_times:
            print("❌ Нет успешных запросов для анализа")
            return
        
        elapsed = time.time() - self.start_time
        total_requests = self.successful_requests + self.failed_requests
        
        print("\n" + "="*60)
        print("📊 РЕЗУЛЬТАТЫ ТЕСТА СКОРОСТИ")
        print("="*60)
        print(f"⏱️  Время тестирования: {elapsed:.2f} сек")
        print(f"📡 Всего запросов: {total_requests}")
        print(f"✅ Успешных запросов: {self.successful_requests}")
        print(f"❌ Неудачных запросов: {self.failed_requests}")
        print(f"📈 Процент успеха: {(self.successful_requests/total_requests)*100:.1f}%")
        
        # Статистика времени ответа
        avg_response_time = statistics.mean(self.response_times)
        min_response_time = min(self.response_times)
        max_response_time = max(self.response_times)
        median_response_time = statistics.median(self.response_times)
        
        print(f"\n⏳ ВРЕМЯ ОТВЕТА:")
        print(f"   Среднее: {avg_response_time:.2f} мс")
        print(f"   Медиана: {median_response_time:.2f} мс")
        print(f"   Минимальное: {min_response_time:.2f} мс")
        print(f"   Максимальное: {max_response_time:.2f} мс")
        
        # Статистика скорости
        requests_per_sec = self.successful_requests / elapsed
        total_bytes_sent = sum(self.bytes_sent)
        total_bytes_received = sum(self.bytes_received)
        bytes_per_sec = (total_bytes_sent + total_bytes_received) / elapsed
        
        print(f"\n⚡ СКОРОСТЬ:")
        print(f"   Запросов в секунду: {requests_per_sec:.2f}")
        print(f"   Байт отправлено: {total_bytes_sent:,}")
        print(f"   Байт получено: {total_bytes_received:,}")
        print(f"   Скорость передачи: {bytes_per_sec:.2f} байт/сек")
        print(f"   Скорость передачи: {bytes_per_sec/1024:.2f} Кбит/сек")
        
        # Процентили времени ответа
        sorted_times = sorted(self.response_times)
        p50 = sorted_times[int(len(sorted_times) * 0.5)]
        p90 = sorted_times[int(len(sorted_times) * 0.9)]
        p95 = sorted_times[int(len(sorted_times) * 0.95)]
        p99 = sorted_times[int(len(sorted_times) * 0.99)]
        
        print(f"\n📊 ПРОЦЕНТИЛИ ВРЕМЕНИ ОТВЕТА:")
        print(f"   50-й процентиль: {p50:.2f} мс")
        print(f"   90-й процентиль: {p90:.2f} мс")
        print(f"   95-й процентиль: {p95:.2f} мс")
        print(f"   99-й процентиль: {p99:.2f} мс")
        
        print("="*60)

def main():
    print("🚀 Arduino Speed Test - Упрощенная версия")
    print("="*50)
    
    # Настройка IP адреса
    ip = input("Введите IP адрес Arduino (по умолчанию 192.168.1.140): ").strip()
    if not ip:
        ip = "192.168.1.140"
    
    tester = SimpleSpeedTest(ip)
    
    # Проверка подключения
    print(f"\n🔍 Проверка подключения к {ip}...")
    test_result = tester.make_request("/api/status")
    
    if test_result['success']:
        print("✅ Подключение успешно!")
        
        # Выбор типа теста
        print("\n🎯 Выберите тип теста:")
        print("1. Тест статуса (/api/status)")
        print("2. Тест датчиков (/api/sensors)")
        print("3. Стресс-тест (смешанный)")
        print("4. Стресс-тест на время")
        
        choice = input("Введите номер (1-4): ").strip()
        
        if choice == "1":
            num_requests = int(input("Количество запросов (по умолчанию 100): ") or "100")
            tester.run_test("/api/status", num_requests)
        elif choice == "2":
            num_requests = int(input("Количество запросов (по умолчанию 100): ") or "100")
            tester.run_test("/api/sensors", num_requests)
        elif choice == "3":
            duration = int(input("Продолжительность в секундах (по умолчанию 60): ") or "60")
            tester.run_stress_test(duration)
        elif choice == "4":
            duration = int(input("Продолжительность в секундах (по умолчанию 60): ") or "60")
            tester.run_stress_test(duration)
        else:
            print("❌ Неверный выбор")
    else:
        print("❌ Не удалось подключиться к Arduino")
        print("Проверьте IP адрес и работу сервера")

if __name__ == "__main__":
    main() 