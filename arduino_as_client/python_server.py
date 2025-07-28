#!/usr/bin/env python3
import socket

# Настройки сервера
HOST = '192.168.1.103'  # IP вашего ноутбука
PORT = 8080

# Создаем сокет
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

try:
    # Привязываем сокет к адресу
    server_socket.bind((HOST, PORT))
    server_socket.listen(1)
    
    print(f"Сервер запущен на {HOST}:{PORT}")
    print("Ожидание подключений...")
    
    while True:
        # Принимаем подключение
        client_socket, address = server_socket.accept()
        print(f"Подключение от {address}")
        
        try:
            # Получаем данные
            data = client_socket.recv(1024).decode('utf-8')
            print(f"Получено: {data.strip()}")
            
            # Отправляем ответ
            response = "Hello from Python server!\n"
            client_socket.send(response.encode('utf-8'))
            print(f"Отправлено: {response.strip()}")
            
        except Exception as e:
            print(f"Ошибка при обработке клиента: {e}")
        finally:
            client_socket.close()
            print("Клиент отключен")
            
except KeyboardInterrupt:
    print("\nСервер остановлен")
except Exception as e:
    print(f"Ошибка сервера: {e}")
finally:
    server_socket.close() 