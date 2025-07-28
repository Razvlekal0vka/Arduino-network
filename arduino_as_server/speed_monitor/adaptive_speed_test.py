#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Адаптивный тестер скорости для Arduino
Автоматически изменяет объем и частоту данных для поиска оптимальных параметров
"""

import requests
import time
import json
import statistics
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime
import threading
from collections import deque

class AdaptiveSpeedTester:
    def __init__(self, ip="192.168.1.140", port=80):
        self.base_url = f"http://{ip}:{port}"
        self.session = requests.Session()
        self.timeout = 10
        
        # Настройки сессии
        self.session.headers.update({
            'User-Agent': 'AdaptiveSpeedTester/1.0',
            'Accept': 'application/json, text/html, */*',
            'Connection': 'close'
        })
        
        # Результаты тестов
        self.test_results = []
        self.optimal_params = {}
        
        # Параметры тестирования
        self.data_sizes = [50, 100, 200, 500, 1000, 2000, 5000]  # байт
        self.frequencies = [0.1, 0.2, 0.5, 1.0, 2.0, 5.0, 10.0]  # запросов в секунду
        self.test_duration = 30  # секунд на тест
        
    def generate_test_data(self, size):
        """Генерирует тестовые данные указанного размера"""
        if size <= 100:
            # Простые данные
            return {"test": "data", "size": size, "timestamp": time.time()}
        elif size <= 500:
            # Средние данные
            data = {"test": "data", "size": size, "timestamp": time.time()}
            data["array"] = list(range(size // 10))
            return data
        else:
            # Большие данные
            data = {"test": "data", "size": size, "timestamp": time.time()}
            data["large_array"] = list(range(size // 20))
            data["metadata"] = {"description": "Large test data", "version": "1.0"}
            return data
    
    def make_adaptive_request(self, endpoint, data_size=None):
        """Выполняет адаптивный запрос с измерением времени"""
        try:
            start_time = time.time()
            
            if data_size:
                # Отправляем данные на сервер
                test_data = self.generate_test_data(data_size)
                response = self.session.post(
                    f"{self.base_url}{endpoint}",
                    json=test_data,
                    timeout=self.timeout
                )
            else:
                # Обычный GET запрос
                response = self.session.get(
                    f"{self.base_url}{endpoint}",
                    timeout=self.timeout
                )
            
            end_time = time.time()
            
            response_time = (end_time - start_time) * 1000  # в миллисекундах
            bytes_sent = len(json.dumps(test_data).encode()) if data_size else 0
            bytes_received = len(response.content)
            
            return {
                'success': response.status_code == 200,
                'response_time': response_time,
                'bytes_sent': bytes_sent,
                'bytes_received': bytes_received,
                'status_code': response.status_code,
                'data_size': data_size
            }
        except Exception as e:
            return {
                'success': False,
                'response_time': 0,
                'bytes_sent': 0,
                'bytes_received': 0,
                'status_code': 0,
                'error': str(e),
                'data_size': data_size
            }
    
    def run_single_test(self, data_size, frequency, duration=None):
        """Запускает один тест с заданными параметрами"""
        if duration is None:
            duration = self.test_duration
            
        print(f"🧪 Тест: размер={data_size} байт, частота={frequency:.1f} запр/сек")
        
        interval = 1.0 / frequency
        start_time = time.time()
        results = []
        
        while time.time() - start_time < duration:
            result = self.make_adaptive_request("/api/control", data_size)
            results.append(result)
            
            if not result['success']:
                print(f"❌ Ошибка: {result.get('error', 'Unknown error')}")
                break
            
            time.sleep(interval)
        
        # Анализируем результаты
        successful_results = [r for r in results if r['success']]
        
        if not successful_results:
            return None
        
        # Вычисляем статистику
        response_times = [r['response_time'] for r in successful_results]
        total_bytes_sent = sum(r['bytes_sent'] for r in successful_results)
        total_bytes_received = sum(r['bytes_received'] for r in successful_results)
        
        test_stats = {
            'data_size': data_size,
            'frequency': frequency,
            'duration': duration,
            'total_requests': len(results),
            'successful_requests': len(successful_results),
            'success_rate': len(successful_results) / len(results) * 100,
            'avg_response_time': statistics.mean(response_times),
            'min_response_time': min(response_times),
            'max_response_time': max(response_times),
            'std_response_time': statistics.stdev(response_times) if len(response_times) > 1 else 0,
            'total_bytes_sent': total_bytes_sent,
            'total_bytes_received': total_bytes_received,
            'bytes_per_second': (total_bytes_sent + total_bytes_received) / duration,
            'requests_per_second': len(successful_results) / duration
        }
        
        return test_stats
    
    def run_comprehensive_test(self):
        """Запускает комплексный тест всех комбинаций параметров"""
        print("🚀 Запуск комплексного адаптивного теста")
        print("=" * 60)
        
        all_results = []
        
        for data_size in self.data_sizes:
            for frequency in self.frequencies:
                print(f"\n📊 Тестирование: {data_size} байт @ {frequency:.1f} запр/сек")
                
                result = self.run_single_test(data_size, frequency, 20)  # 20 сек на тест
                
                if result:
                    all_results.append(result)
                    print(f"✅ Успех: {result['success_rate']:.1f}% | "
                          f"Время ответа: {result['avg_response_time']:.1f} мс | "
                          f"Скорость: {result['bytes_per_second']:.1f} байт/сек")
                else:
                    print(f"❌ Тест провален")
                
                time.sleep(2)  # Пауза между тестами
        
        self.test_results = all_results
        self.analyze_results()
    
    def analyze_results(self):
        """Анализирует результаты и находит оптимальные параметры"""
        if not self.test_results:
            print("❌ Нет результатов для анализа")
            return
        
        print("\n" + "="*60)
        print("📊 АНАЛИЗ РЕЗУЛЬТАТОВ")
        print("="*60)
        
        # Группируем результаты по размеру данных
        size_groups = {}
        for result in self.test_results:
            size = result['data_size']
            if size not in size_groups:
                size_groups[size] = []
            size_groups[size].append(result)
        
        # Анализируем каждый размер данных
        optimal_params = {}
        
        for size, results in size_groups.items():
            print(f"\n📦 Размер данных: {size} байт")
            print("-" * 40)
            
            # Сортируем по частоте
            results.sort(key=lambda x: x['frequency'])
            
            best_result = None
            best_score = 0
            
            for result in results:
                # Вычисляем общий скор (комбинация скорости и надежности)
                reliability_score = result['success_rate'] / 100
                speed_score = min(result['bytes_per_second'] / 1000, 1.0)  # нормализуем до 1000 байт/сек
                response_score = max(0, 1 - result['avg_response_time'] / 1000)  # нормализуем до 1000 мс
                
                total_score = reliability_score * 0.4 + speed_score * 0.4 + response_score * 0.2
                
                print(f"  Частота: {result['frequency']:.1f} запр/сек | "
                      f"Успех: {result['success_rate']:.1f}% | "
                      f"Время: {result['avg_response_time']:.1f} мс | "
                      f"Скорость: {result['bytes_per_second']:.1f} байт/сек | "
                      f"Скор: {total_score:.3f}")
                
                if total_score > best_score:
                    best_score = total_score
                    best_result = result
            
            if best_result:
                optimal_params[size] = best_result
                print(f"  🏆 ОПТИМАЛЬНО: {best_result['frequency']:.1f} запр/сек")
        
        self.optimal_params = optimal_params
        self.print_recommendations()
    
    def print_recommendations(self):
        """Выводит рекомендации для робота"""
        print("\n" + "="*60)
        print("🤖 РЕКОМЕНДАЦИИ ДЛЯ РОБОТА")
        print("="*60)
        
        if not self.optimal_params:
            print("❌ Не удалось найти оптимальные параметры")
            return
        
        print("📋 Оптимальные параметры по размерам данных:")
        print()
        
        for size, params in self.optimal_params.items():
            print(f"📦 {size} байт:")
            print(f"   ⚡ Частота: {params['frequency']:.1f} запросов в секунду")
            print(f"   ⏱️  Время ответа: {params['avg_response_time']:.1f} мс")
            print(f"   💾 Скорость: {params['bytes_per_second']:.1f} байт/сек")
            print(f"   ✅ Надежность: {params['success_rate']:.1f}%")
            print()
        
        # Общие рекомендации
        print("🎯 ОБЩИЕ РЕКОМЕНДАЦИИ:")
        
        # Находим лучшие параметры для разных сценариев
        best_reliability = max(self.optimal_params.values(), key=lambda x: x['success_rate'])
        best_speed = max(self.optimal_params.values(), key=lambda x: x['bytes_per_second'])
        best_response = min(self.optimal_params.values(), key=lambda x: x['avg_response_time'])
        
        print(f"🔒 Для максимальной надежности: {best_reliability['data_size']} байт @ {best_reliability['frequency']:.1f} запр/сек")
        print(f"⚡ Для максимальной скорости: {best_speed['data_size']} байт @ {best_speed['frequency']:.1f} запр/сек")
        print(f"⏱️  Для минимального времени ответа: {best_response['data_size']} байт @ {best_response['frequency']:.1f} запр/сек")
        
        # Рекомендации для робота
        print("\n🤖 РЕКОМЕНДАЦИИ ДЛЯ РОБОТА:")
        print("• Используйте размер данных 100-500 байт для обычных операций")
        print("• Частота 0.5-2.0 запросов в секунду обеспечивает стабильность")
        print("• Для критических операций используйте меньший размер данных")
        print("• Для массовой передачи данных используйте больший размер")
    
    def plot_results(self):
        """Создает графики результатов"""
        if not self.test_results:
            print("❌ Нет данных для построения графиков")
            return
        
        # Подготавливаем данные для графиков
        sizes = [r['data_size'] for r in self.test_results]
        frequencies = [r['frequency'] for r in self.test_results]
        response_times = [r['avg_response_time'] for r in self.test_results]
        success_rates = [r['success_rate'] for r in self.test_results]
        bytes_per_sec = [r['bytes_per_second'] for r in self.test_results]
        
        # Создаем графики
        fig, axes = plt.subplots(2, 2, figsize=(15, 12))
        fig.suptitle('Адаптивный тест скорости Arduino', fontsize=16)
        
        # График 1: Время ответа vs Размер данных
        scatter1 = axes[0, 0].scatter(sizes, response_times, c=frequencies, cmap='viridis', s=50)
        axes[0, 0].set_xlabel('Размер данных (байт)')
        axes[0, 0].set_ylabel('Время ответа (мс)')
        axes[0, 0].set_title('Время ответа vs Размер данных')
        axes[0, 0].grid(True, alpha=0.3)
        plt.colorbar(scatter1, ax=axes[0, 0], label='Частота (запр/сек)')
        
        # График 2: Процент успеха vs Частота
        scatter2 = axes[0, 1].scatter(frequencies, success_rates, c=sizes, cmap='plasma', s=50)
        axes[0, 1].set_xlabel('Частота (запр/сек)')
        axes[0, 1].set_ylabel('Процент успеха (%)')
        axes[0, 1].set_title('Надежность vs Частота')
        axes[0, 1].grid(True, alpha=0.3)
        plt.colorbar(scatter2, ax=axes[0, 1], label='Размер данных (байт)')
        
        # График 3: Скорость передачи vs Размер данных
        scatter3 = axes[1, 0].scatter(sizes, bytes_per_sec, c=frequencies, cmap='cool', s=50)
        axes[1, 0].set_xlabel('Размер данных (байт)')
        axes[1, 0].set_ylabel('Скорость (байт/сек)')
        axes[1, 0].set_title('Скорость передачи vs Размер данных')
        axes[1, 0].grid(True, alpha=0.3)
        plt.colorbar(scatter3, ax=axes[1, 0], label='Частота (запр/сек)')
        
        # График 4: 3D график всех параметров
        ax3d = fig.add_subplot(2, 2, 4, projection='3d')
        scatter4 = ax3d.scatter(sizes, frequencies, response_times, c=success_rates, cmap='viridis', s=50)
        ax3d.set_xlabel('Размер данных (байт)')
        ax3d.set_ylabel('Частота (запр/сек)')
        ax3d.set_zlabel('Время ответа (мс)')
        ax3d.set_title('3D анализ параметров')
        plt.colorbar(scatter4, ax=ax3d, label='Процент успеха (%)')
        
        plt.tight_layout()
        plt.show()
    
    def save_results(self, filename="adaptive_test_results.json"):
        """Сохраняет результаты в файл"""
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump({
                'test_results': self.test_results,
                'optimal_params': self.optimal_params,
                'timestamp': datetime.now().isoformat()
            }, f, indent=2, ensure_ascii=False)
        print(f"💾 Результаты сохранены в {filename}")

def main():
    print("🤖 Адаптивный тестер скорости для Arduino")
    print("="*50)
    
    # Настройка IP адреса
    ip = input("Введите IP адрес Arduino (по умолчанию 192.168.1.140): ").strip()
    if not ip:
        ip = "192.168.1.140"
    
    tester = AdaptiveSpeedTester(ip)
    
    # Проверка подключения
    print(f"\n🔍 Проверка подключения к {ip}...")
    test_result = tester.make_adaptive_request("/api/status")
    
    if test_result['success']:
        print("✅ Подключение успешно!")
        
        # Запуск комплексного теста
        print("\n🚀 Запуск адаптивного теста...")
        print("Это займет несколько минут...")
        
        tester.run_comprehensive_test()
        
        # Показываем графики
        print("\n📊 Создание графиков...")
        tester.plot_results()
        
        # Сохраняем результаты
        tester.save_results()
        
    else:
        print("❌ Не удалось подключиться к Arduino")
        print("Проверьте IP адрес и работу сервера")

if __name__ == "__main__":
    main() 