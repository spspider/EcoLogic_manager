-- Создание базы данных ecologic и пользователя ecouser
CREATE DATABASE IF NOT EXISTS ecologic;
USE ecologic;

-- Создание пользователя ecouser с доступом как от '%' (удалённо), так и от 'localhost'
CREATE USER IF NOT EXISTS 'ecouser'@'%' IDENTIFIED BY 'ecopass';
CREATE USER IF NOT EXISTS 'ecouser'@'localhost' IDENTIFIED BY 'ecopass';
GRANT ALL PRIVILEGES ON ecologic.* TO 'ecouser'@'%';
GRANT ALL PRIVILEGES ON ecologic.* TO 'ecouser'@'localhost';
FLUSH PRIVILEGES;

-- Таблица devices (объединённая версия)
CREATE TABLE IF NOT EXISTS devices (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(128) NOT NULL UNIQUE,
    token VARCHAR(128) NOT NULL,
    pin_setup JSON,
    real_status JSON,
    desired_status JSON,
    last_seen DATETIME,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- Таблица arduino_config
CREATE TABLE IF NOT EXISTS arduino_config (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(50) DEFAULT 'default',
    config_data JSON,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- Таблица pin_states
CREATE TABLE IF NOT EXISTS pin_states (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id VARCHAR(50) DEFAULT 'default',
    pin_states JSON,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- Создание дополнительных баз данных для WordPress
CREATE DATABASE IF NOT EXISTS wp_db;
CREATE DATABASE IF NOT EXISTS wp_db_voice;
CREATE DATABASE IF NOT EXISTS wp_db_gica;

-- Создание пользователя wp_user и предоставление ему прав
CREATE USER IF NOT EXISTS 'wp_user'@'%' IDENTIFIED BY 'wp_pass';
GRANT ALL PRIVILEGES ON wp_db.* TO 'wp_user'@'%';
GRANT ALL PRIVILEGES ON wp_db_voice.* TO 'wp_user'@'%';
GRANT ALL PRIVILEGES ON wp_db_gica.* TO 'wp_user'@'%';
FLUSH PRIVILEGES;