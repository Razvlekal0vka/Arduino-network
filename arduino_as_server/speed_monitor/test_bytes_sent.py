#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Тестовый скрипт для проверки подсчета отправленных байт
"""

import requests
import json
import time

def test_get_request():
    """Тестирует GET запрос и подсчет байт"""
    print("🔍 Тестирование GET запроса...")
    
    # Создаем сессию
    session = requests.Session()
    session.headers.update({
        'User-Agent': 'TestClient/1.0',
        'Accept': 'application/json',
        'Connection': 'close'
    })
    
    # Выполняем GET запрос
    start_time = time.time()
    response = session.get("http://192.168.1.140/api/status", timeout=2)
    end_time = time.time()
    
    # Подсчитываем отправленные байт
    request_line = "GET /api/status HTTP/1.1\r\n"
    headers = ""
    for key, value in response.request.headers.items():
        headers += f"{key}: {value}\r\n"
    headers += "\r\n"
    
    bytes_sent = len(request_line.encode('utf-8')) + len(headers.encode('utf-8'))
    bytes_received = len(response.content)
    response_time = (end_time - start_time) * 1000
    
    print(f"📤 Отправлено байт: {bytes_sent}")
    print(f"📥 Получено байт: {bytes_received}")
    print(f"⏳ Время ответа: {response_time:.2f} мс")
    print(f"📊 Статус: {response.status_code}")
    
    return bytes_sent, bytes_received

def test_post_request():
    """Тестирует POST запрос и подсчет байт"""
    print("\n🔍 Тестирование POST запроса...")
    
    # Создаем сессию
    session = requests.Session()
    session.headers.update({
        'User-Agent': 'TestClient/1.0',
        'Accept': 'application/json',
        'Content-Type': 'application/json',
        'Connection': 'close'
    })
    
    # Данные для отправки
    robot_data = {
        "id": "test_robot",
        "temperature": 22.5,
        "battery": 85,
        "x": 1.5,
        "y": -2.3,
        "angle": 45.0,
        "mode": "test",
        "status": "testing"
    }
    
    # Выполняем POST запрос
    start_time = time.time()
    response = session.post("http://192.168.1.140/api/robot", json=robot_data, timeout=2)
    end_time = time.time()
    
    # Подсчитываем отправленные байт
    request_line = "POST /api/robot HTTP/1.1\r\n"
    headers = ""
    for key, value in response.request.headers.items():
        headers += f"{key}: {value}\r\n"
    headers += "\r\n"
    
    post_data = json.dumps(robot_data)
    bytes_sent = len(request_line.encode('utf-8')) + len(headers.encode('utf-8')) + len(post_data.encode('utf-8'))
    bytes_received = len(response.content)
    response_time = (end_time - start_time) * 1000
    
    print(f"📤 Отправлено байт: {bytes_sent}")
    print(f"📥 Получено байт: {bytes_received}")
    print(f"⏳ Время ответа: {response_time:.2f} мс")
    print(f"📊 Статус: {response.status_code}")
    print(f"📋 Отправленные данные: {post_data}")
    
    return bytes_sent, bytes_received

def main():
    print("🚀 Тест подсчета отправленных байт")
    print("="*50)
    
    try:
        # Тест GET запроса
        get_sent, get_received = test_get_request()
        
        # Тест POST запроса
        post_sent, post_received = test_post_request()
        
        print("\n" + "="*50)
        print("📊 СРАВНЕНИЕ РЕЗУЛЬТАТОВ")
        print("="*50)
        print(f"GET запрос:")
        print(f"  📤 Отправлено: {get_sent} байт")
        print(f"  📥 Получено: {get_received} байт")
        print(f"  📈 Соотношение: {get_sent/get_received*100:.1f}%")
        
        print(f"\nPOST запрос:")
        print(f"  📤 Отправлено: {post_sent} байт")
        print(f"  📥 Получено: {post_received} байт")
        print(f"  📈 Соотношение: {post_sent/post_received*100:.1f}%")
        
        print(f"\n📊 Разница:")
        print(f"  POST отправляет на {post_sent - get_sent} байт больше чем GET")
        
    except requests.exceptions.ConnectionError:
        print("❌ Ошибка подключения к Arduino")
        print("Проверьте IP адрес и работу сервера")
    except Exception as e:
        print(f"❌ Ошибка: {e}")

if __name__ == "__main__":
    main() 