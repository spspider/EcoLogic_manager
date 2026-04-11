# EcoLogic Grafana User Management System

Система автоматического создания пользователей Grafana при регистрации в MySQL базе данных.

## 🎯 Возможности

- ✅ **Автоматическое создание пользователей** в Grafana при создании в MySQL
- ✅ **Персональные организации** - каждый пользователь получает свою организацию
- ✅ **Персональные дашборды** - автоматически создаются с фильтрацией по пользователю  
- ✅ **Изоляция данных** - пользователи видят только свои устройства
- ✅ **Web интерфейс** для управления пользователями
- ✅ **REST API** для интеграции с другими системами
- ✅ **Синхронизация** существующих MySQL пользователей в Grafana

## 🏗️ Архитектура

```
[Web UI] → [User Management API] → [MySQL] + [Grafana API]
                                      ↓         ↓
                              [User Table]  [User + Org + Dashboard]
```

## 🚀 Быстрый старт

### 1. Настройка переменных окружения

Добавьте в `.env`:
```dotenv
# Grafana Configuration  
GRAFANA_HOST=192.168.1.160
GRAFANA_PORT=3000
GRAFANA_ADMIN_USER=admin
GRAFANA_ADMIN_PASSWORD=admin
```

### 2. Установка зависимостей

```bash
pip install requests
```

### 3. Тестирование системы

```bash
python test_user_management.py
```

### 4. Запуск API

```bash
python user_management_api.py
# или
./start_user_management.sh
```

### 5. Использование Web интерфеса

Откройте http://localhost:3002

## 📱 Web интерфейс

### Создание пользователя
- Вводите username, password, email (опционально)
- Автоматически создается:  
  - Запись в MySQL таблице `users`
  - Пользователь в Grafana
  - Персональная организация `EcoLogic-{username}`
  - Дашборд с данными пользователя
  - InfluxDB data source в организации

### Удаление пользователя
- Удаляется из MySQL и Grafana
- Удаляется организация со всеми дашбордами

### Синхронизация существующих пользователей
- Создает Grafana аккаунты для всех MySQL пользователей
- Безопасно пропускает уже существующих

## 🔌 REST API

### Создание пользователя
```bash
curl -X POST http://localhost:3002/api/users \
  -H "Content-Type: application/json" \
  -d '{"username": "newuser", "password": "password123", "email": "user@example.com"}'
```

### Удаление пользователя  
```bash
curl -X DELETE http://localhost:3002/api/users/username
```

### Список пользователей
```bash
curl http://localhost:3002/api/users
```

### Синхронизация всех пользователей
```bash
curl -X POST http://localhost:3002/api/sync-users
```

### Тест подключений
```bash
curl http://localhost:3002/api/test
```

## 📊 Структура Grafana

### Организации
Каждый пользователь получает организацию: `EcoLogic-{username}`

### Дашборды  
Автоматически создается дашборд с панелями:
- **Device Status** - текущие значения датчиков
- **Activity Timeline** - временные графики
- **Device Summary** - сводная таблица устройств

### Запросы InfluxDB
Все запросы автоматически фильтруются по пользователю:
```flux
from(bucket: "default")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "device_pins")
  |> filter(fn: (r) => r.user == "username")  // Автоматическая фильтрация!
  |> filter(fn: (r) => r._field == "value")
```

## 🔐 Безопасность

- Каждый пользователь имеет доступ только к своей организации
- Автоматическая фильтрация данных по username
- Admin права только у системы управления
- Пользователи получают роль Admin только в своей организации

## 🛠️ Интеграция с существующей системой

### В app_fastapi.py можно добавить:

```python
from user_management_api import create_user

@app.post("/api/register")
async def register_user(username: str = Form(), password: str = Form()):
    # Создание в MySQL + Grafana автоматически
    result = await create_user(UserCreate(username=username, password=password))
    
    if result['success']:
        return {"message": "User created successfully"}
    else:
        raise HTTPException(status_code=400, detail=result['error'])
```

## 🔧 Troubleshooting

### Пользователь не может войти в Grafana
```bash
# Проверьте, создан ли пользователь
curl http://localhost:3002/api/users
```

### Пользователь не видит свои данные
- Убедитесь, что данные в InfluxDB имеют тег `user` с именем пользователя  
- Проверьте, что data source настроен корректно в организации пользователя

### API не отвечает
```bash
# Проверьте статус
python test_user_management.py
```

### Grafana admin недоступен
- Проверьте credentials в `.env`
- По умолчанию: admin/admin
- Убедитесь, что Grafana запущен на правильном порту

## 📈 Мониторинг

Все операции логируются:
```bash
# Просмотр логов
tail -f user_management.log
```

Статистика через API:
```bash
curl http://localhost:3002/api/test
```

## 🚀 Производственное развертывание

1. **Обновите пароли** в `.env`
2. **Настройте HTTPS** для API
3. **Ограничьте доступ** к API на уровне сети
4. **Настройте backup** организаций Grafana
5. **Мониторинг** процесса создания пользователей

Система готова к работе и автоматически поддерживает синхронизацию пользователей между MySQL и Grafana! 🎉