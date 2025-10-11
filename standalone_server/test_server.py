#!/usr/bin/env python3
"""
Тестовый скрипт для проверки работы сервера
"""
import requests
import json

def test_server():
    base_url = "http://127.0.0.1:5000"
    
    # Тест 1: Проверка главной страницы
    try:
        response = requests.get(f"{base_url}/", auth=('admin', 'admin123'))
        print(f"✅ Main page: {response.status_code}")
    except Exception as e:
        print(f"❌ Main page error: {e}")
        return
    
    # Тест 2: Проверка sendAJAX
    try:
        params = {
            'data': '{"t":127,"v":0}',
            'device_id': 'test_device'
        }
        response = requests.get(f"{base_url}/api/ajax", params=params, auth=('admin', 'admin123'))
        print(f"✅ API ajax status request: {response.status_code}")
        print(f"Response: {response.text}")
    except Exception as e:
        print(f"❌ sendAJAX error: {e}")
    
    # Тест 3: Проверка установки значения пина
    try:
        params = {
            'data': '{"t":0,"v":1}',
            'device_id': 'test_device'
        }
        response = requests.get(f"{base_url}/api/ajax", params=params, auth=('admin', 'admin123'))
        print(f"✅ API ajax pin control: {response.status_code}")
        print(f"Response: {response.text}")
    except Exception as e:
        print(f"❌ sendAJAX pin control error: {e}")

if __name__ == "__main__":
    test_server()