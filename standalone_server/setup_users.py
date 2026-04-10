#!/usr/bin/env python3
"""
Скрипт для настройки пользовательской системы в EcoLogic Manager
"""
import pymysql
from dotenv import load_dotenv
import os

load_dotenv()

DEVICE_MANAGER_DB_HOST = os.getenv("DEVICE_MANAGER_DB_HOST", "localhost")
DEVICE_MANAGER_DB_PORT = int(os.getenv("DEVICE_MANAGER_DB_PORT", "3306"))
DEVICE_MANAGER_DB_NAME = os.getenv("DEVICE_MANAGER_DB_NAME", "ecologic")
DEVICE_MANAGER_DB_USER = os.getenv("DEVICE_MANAGER_DB_USER", "ecouser")
DB_PASS = os.getenv("DEVICE_MANAGER_DB_PASSWORD", "ecopass")

def setup_database():
    try:
        conn = pymysql.connect(
            host=DEVICE_MANAGER_DB_HOST, port=DEVICE_MANAGER_DB_PORT, user=DEVICE_MANAGER_DB_USER, 
            passwd=DB_PASS, db=DEVICE_MANAGER_DB_NAME, charset='utf8mb4'
        )
        
        with conn.cursor() as cur:
            # Читаем и выполняем SQL скрипт
            with open('update_db.sql', 'r', encoding='utf-8') as f:
                sql_commands = f.read().split(';')
                
            for command in sql_commands:
                command = command.strip()
                if command:
                    try:
                        cur.execute(command)
                        print(f"✓ Executed: {command[:50]}...")
                    except Exception as e:
                        print(f"✗ Error: {e}")
            
            conn.commit()
            print("\n✓ Database setup completed successfully!")
            
    except Exception as e:
        print(f"✗ Database connection error: {e}")
    finally:
        if 'conn' in locals():
            conn.close()

if __name__ == "__main__":
    setup_database()