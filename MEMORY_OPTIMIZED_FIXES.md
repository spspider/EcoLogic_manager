# Исправления с оптимизацией памяти

## Основные изменения:

### 1. Глобальные флаги (EcoLogic_manager.ino)
```cpp
bool enable_http_requests = false;    // Отключить HTTP по умолчанию
bool enable_email_sending = false;    // Отключить email по умолчанию  
bool enable_geo_location = false;     // Отключить геолокацию по умолчанию
bool enable_mqtt_reconnect = true;    // MQTT включен
char no_internet_timer = 0;           // char вместо int для экономии
```

### 2. HTTP с таймаутом (w_position_fixed.ino)
- Добавлен `http.setTimeout(5000)`
- Проверка флага `enable_http_requests`
- Правильная обработка ошибок

### 3. MQTT без блокировки (a_pubClient_fixed.ino)
- Убран бесконечный цикл `while(!client.connected())`
- Ограничение попыток: максимум 3 попытки каждые 30 секунд
- Использование `char attemptCount` для экономии памяти

### 4. WiFi подключение асинхронное (a_CaptivePortalAdvanced_fixed.ino)
- Убраны блокирующие `delay(500)` и `while` циклы
- Статические переменные `bool connecting`, `char wifiAttempt`
- Проверка состояния по времени

### 5. Watchdog в основном цикле
- Перезагрузка если loop не выполняется 30 секунд
- Добавлен `yield()` для обработки WiFi

## Оптимизация памяти:

### Использование char вместо int:
- `char no_internet_timer` вместо `int`
- `char attemptCount` для MQTT попыток
- `char wifiAttempt` для WiFi попыток

### Использование bool для флагов:
- `bool enable_*` флаги (1 байт каждый)
- `bool connecting` для состояния WiFi

### Статические переменные:
- Минимальное использование глобальной памяти
- Локальные static переменные для состояний

## Как использовать исправленные файлы:

1. **Замените оригинальные файлы:**
   - `w_position.ino` → `w_position_fixed.ino`
   - `a_pubClient.ino` → `a_pubClient_fixed.ino`
   - `a_CaptivePortalAdvanced.ino` → `a_CaptivePortalAdvanced_fixed.ino`

2. **Включите функции в setup():**
```cpp
void setup() {
  // ... существующий код ...
  
  // Включить нужные функции
  enable_http_requests = true;  // Для HTTP запросов
  enable_email_sending = true;  // Для email
  enable_geo_location = true;   // Для геолокации
}
```

## Результат:
- Устройство не зависает на HTTP запросах
- MQTT не блокирует выполнение
- WiFi подключение не блокирует loop
- Watchdog перезагружает при зависании
- Экономия памяти за счет char и bool
- Все блокирующие операции отключены по умолчанию