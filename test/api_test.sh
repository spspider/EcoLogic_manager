#!/bin/bash

# API Test Script for EcoLogic Manager
# Device ID: esp2A006C_BCDDC22A006C
# Token: tk01
# Base URL: http://localhost:5001



echo "=== EcoLogic Manager API Test Script ==="
echo "Device ID: esp2A006C_BCDDC22A006C"
echo "Token: tk01"
echo "Base URL: https://ecologic.pp.ua"
echo ""

# 1. Upload pin configuration
echo "1. Upload pin configuration (/api/cfg)"
curl -X POST "https://ecologic.pp.ua/api/cfg?id=esp2A006C_BCDDC22A006C&tk=tk01" \
  -H "Content-Type: application/json" \
  -d '{
    "numberChosed": 3,
    "pin": [0, 16, 2],
    "pinmode": [2, 2, 5],
    "descr": ["LED", "Relay", "Temperature"],
    "widget": [1, 1, 6],
    "defaultVal": [0, 0, 0]
  }'

echo -e "\n"

# 2. Get device configuration
echo "2. Get device configuration (/api/config)"
curl -X GET "https://ecologic.pp.ua/api/config?id=esp2A006C_BCDDC22A006C"

echo -e "\n"

# 3. Update real status from Arduino
echo "3. Update real status (/api/real)"
curl -X POST "https://ecologic.pp.ua/api/real?id=esp2A006C_BCDDC22A006C&tk=tk01" \
  -H "Content-Type: application/json" \
  -d '{
    "stat": ["0", "1", "23.5"]
  }'

echo -e "\n"

# 4. Get desired status for Arduino
echo "4. Get desired status (/api/desired)"
curl -X GET "https://ecologic.pp.ua/api/desired?id=esp2A006C_BCDDC22A006C&tk=tk01"

echo -e "\n"

# 5. AJAX request - get status (t=127)
echo "5. AJAX get status (/api/ajax)"
curl -X GET "https://ecologic.pp.ua/api/ajax?device_id=esp2A006C_BCDDC22A006C&data=%7B%22t%22%3A127%2C%22v%22%3A0%7D"

echo -e "\n"

# 6. AJAX request - set pin value (t=0, v=1)
echo "6. AJAX set pin 0 to value 1 (/api/ajax)"
curl -X GET "https://ecologic.pp.ua/api/ajax?device_id=esp2A006C_BCDDC22A006C&data=%7B%22t%22%3A0%2C%22v%22%3A1%7D"

echo -e "\n"

# 7. Get devices list
echo "7. Get devices list (/api/devices)"
curl -X GET "https://ecologic.pp.ua/api/devices"

echo -e "\n"

# 8. Get home page for device
echo "8. Get home page (/api/home)"
curl -X GET "https://ecologic.pp.ua/api/home?device_id=esp2A006C_BCDDC22A006C"

echo -e "\n"

# 9. Get device selector page
echo "9. Get device selector (/api/device_selector)"
curl -X GET "https://ecologic.pp.ua/api/device_selector"

echo -e "\n"

echo "=== Test completed ==="