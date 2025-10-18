-- Создание таблицы пользователей
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Добавление поля owner в таблицу devices (проверка вручную)
-- MySQL не поддерживает IF NOT EXISTS для ADD COLUMN, поэтому нужно проверять отдельно
-- Ниже пример, как это можно сделать программно (например, в скрипте на Python или PHP):
-- Проверить, существует ли столбец 'owner', и только потом выполнять ALTER TABLE

-- Добавление индекса для быстрого поиска устройств по владельцу
SELECT COUNT(*) AS column_exists
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_NAME = 'devices' AND COLUMN_NAME = 'owner';

-- Добавление админа по умолчанию (если не существует)
ALTER TABLE devices ADD COLUMN owner VARCHAR(50) DEFAULT 'admin';

-- Создание таблицы для настроек устройств (other_setup)
CREATE TABLE IF NOT EXISTS device_settings (
    device_id VARCHAR(50) PRIMARY KEY,
    other_setup JSON,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- Обновление существующих устройств - назначаем их админу
UPDATE devices SET owner = 'admin' WHERE owner IS NULL OR owner = '';

-- Создание индексов
CREATE INDEX IF NOT EXISTS idx_devices_owner ON devices(owner);