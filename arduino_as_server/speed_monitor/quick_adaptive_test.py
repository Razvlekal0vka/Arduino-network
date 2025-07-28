#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Быстрый тест адаптивного тестера скорости
"""

import requests
import time
import json
import statistics

def quick_adaptive_test(ip="192.168.1.140"):
    """Быстрый тест адаптивного тестера"""
    base_url = f"http://{ip}:80"
    session = requests.Session()
    
    print("🚀 Быстрый адаптивный тест")
    print("="*40)
    
    # Тестовые параметры
    test_sizes = [50, 100, 200, 500]
    test_frequencies = [0.5, 1.0, 2.0]
    
    results = []
    
    for size in test_sizes:
        for freq in test_frequencies:
            print(f"\n🧪 Тест: {size} байт @ {freq:.1f} запр/сек")
            
            # Генерируем тестовые данные
            test_data = {
                "test": "data",
                "size": size,
                "timestamp": time.time(),
                "array": list(range(size // 10))
            }
            
            interval = 1.0 / freq
            test_duration = 10  # 10 секунд на тест
            start_time = time.time()
            response_times = []
            successful_requests = 0
            total_requests = 0
            
            while time.time() - start_time < test_duration:
                try:
                    request_start = time.time()
                    response = session.post(
                        f"{base_url}/api/control",
                        json=test_data,
                        timeout=5
                    )
                    request_end = time.time()
                    
                    if response.status_code == 200:
                        response_time = (request_end - request_start) * 1000
                        response_times.append(response_time)
                        successful_requests += 1
                    
                    total_requests += 1
                    time.sleep(interval)
                    
                except Exception as e:
                    print(f"❌ Ошибка: {e}")
                    break
            
            if response_times:
                avg_response_time = statistics.mean(response_times)
                success_rate = (successful_requests / total_requests) * 100
                bytes_per_sec = (size * successful_requests) / test_duration
                
                result = {
                    'size': size,
                    'frequency': freq,
                    'avg_response_time': avg_response_time,
                    'success_rate': success_rate,
                    'bytes_per_second': bytes_per_sec,
                    'successful_requests': successful_requests,
                    'total_requests': total_requests
                }
                
                results.append(result)
                
                print(f"✅ Успех: {success_rate:.1f}% | "
                      f"Время ответа: {avg_response_time:.1f} мс | "
                      f"Скорость: {bytes_per_sec:.1f} байт/сек")
            else:
                print(f"❌ Тест провален")
    
    # Анализ результатов
    print("\n" + "="*40)
    print("📊 РЕЗУЛЬТАТЫ БЫСТРОГО ТЕСТА")
    print("="*40)
    
    if results:
        # Находим лучшие параметры
        best_reliability = max(results, key=lambda x: x['success_rate'])
        best_speed = max(results, key=lambda x: x['bytes_per_second'])
        best_response = min(results, key=lambda x: x['avg_response_time'])
        
        print(f"🔒 Лучшая надежность: {best_reliability['size']} байт @ {best_reliability['frequency']:.1f} запр/сек")
        print(f"⚡ Лучшая скорость: {best_speed['size']} байт @ {best_speed['frequency']:.1f} запр/сек")
        print(f"⏱️  Лучшее время ответа: {best_response['size']} байт @ {best_response['frequency']:.1f} запр/сек")
        
        print("\n🤖 РЕКОМЕНДАЦИИ ДЛЯ РОБОТА:")
        print(f"• Используйте размер данных {best_reliability['size']} байт для надежности")
        print(f"• Частота {best_reliability['frequency']:.1f} запросов в секунду")
        print(f"• Ожидаемое время ответа: {best_reliability['avg_response_time']:.1f} мс")
        print(f"• Ожидаемая скорость: {best_reliability['bytes_per_second']:.1f} байт/сек")
    else:
        print("❌ Нет успешных результатов")

if __name__ == "__main__":
    ip = input("Введите IP адрес Arduino (по умолчанию 192.168.1.140): ").strip()
    if not ip:
        ip = "192.168.1.140"
    
    quick_adaptive_test(ip) 