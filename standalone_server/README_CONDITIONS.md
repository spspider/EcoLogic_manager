# Условия (Conditions) - Документация

## Обзор
Система условий позволяет автоматизировать управление устройствами на основе времени, значений датчиков и других параметров.

## Структура данных

### Формат условия в БД (JSON):
```json
[
  {
    "enabled": true,
    "conditionType": "on time reached",
    "conditionValue": "12:00",
    "actionType": "switch pin",
    "actionValue": "1",
    "actionPin": "2",
    "pwmValue": "200"
  }
]
```

## Типы условий (conditionType)
- `none` - нет условия
- `on time reached` - по достижению времени (conditionValue: "HH:MM")
- `equal` - равно значению
- `greater than` - больше чем
- `less than` - меньше чем
- `timer` - таймер в секундах

## Типы действий (actionType)
- `no` - нет действия
- `switch pin` - переключить пин (требует actionPin, actionValue)
- `switch button` - переключить кнопку
- `switch remote button` - переключить удаленную кнопку
- `send Email` - отправить Email
- `switch condition` - включить/выключить другое условие
- `switch mqtt request` - отправить MQTT запрос
- `turn on 8211 strip` - включить WS2811 ленту
- `WOL` - Wake on LAN
- `set timer` - установить таймер
- `move slider` - передвинуть слайдер

## Значения пинов (actionValue)
- `0` - выключить
- `1` - включить
- `PWM` - PWM режим (требует pwmValue: 0-1023)

## API Endpoints

### GET /api/conditions?device_id={id}
Получить список условий для устройства

### POST /api/conditions?device_id={id}
Сохранить условия для устройства
Body: JSON массив условий

### GET /api/pin_setup?device_id={id}
Получить конфигурацию пинов устройства

## Фоновая задача
Сервер проверяет условия каждые 10 секунд и автоматически выполняет действия при выполнении условий.

## Миграция БД
Выполните SQL миграцию:
```bash
mysql -u ecouser -p ecologic < migrations/add_conditions_column.sql
```

## Пример использования

### Включить пин 2 в 6:00 утра:
```json
{
  "enabled": true,
  "conditionType": "on time reached",
  "conditionValue": "06:00",
  "actionType": "switch pin",
  "actionValue": "1",
  "actionPin": "2"
}
```

### Выключить пин 2 в 22:00:
```json
{
  "enabled": true,
  "conditionType": "on time reached",
  "conditionValue": "22:00",
  "actionType": "switch pin",
  "actionValue": "0",
  "actionPin": "2"
}
```

### Установить PWM на пине 5:
```json
{
  "enabled": true,
  "conditionType": "on time reached",
  "conditionValue": "12:00",
  "actionType": "switch pin",
  "actionValue": "PWM",
  "actionPin": "5",
  "pwmValue": "512"
}
```
