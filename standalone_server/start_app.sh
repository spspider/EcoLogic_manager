#!/bin/bash

# Start EcoLogic Device Manager with integrated User Management
# Clean and simple - everything runs on port 5005

echo "🚀 Starting EcoLogic Device Manager (with User Management)"
echo "📍 Main app: http://localhost:5005/"
echo "🏠 Device manager: http://localhost:5005/api/device_selector"
echo "👥 User management: http://localhost:5005/user-management/"
echo ""

cd "$(dirname "$0")"
python app_fastapi.py