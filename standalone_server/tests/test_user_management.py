#!/usr/bin/env python3
"""
Test Script for EcoLogic Grafana User Management System
Tests the complete flow of creating users in MySQL and Grafana
"""

import os
import sys
import requests
import json
from dotenv import load_dotenv
from grafana_user_manager import GrafanaUserManager
import pymysql

def test_mysql_connection():
    """Test MySQL connection"""
    print("🗄️  Testing MySQL Connection...")
    
    load_dotenv()
    
    try:
        conn = pymysql.connect(
            host=os.getenv("DEVICE_MANAGER_DB_HOST", "192.168.1.160"),
            port=int(os.getenv("DEVICE_MANAGER_DB_PORT", "3306")),
            user=os.getenv("DEVICE_MANAGER_DB_USER", "ecouser"),
            passwd=os.getenv("DEVICE_MANAGER_DB_PASSWORD", "password"),
            db=os.getenv("DEVICE_MANAGER_DB_NAME", "ecologic"),
            charset='utf8mb4',
            cursorclass=pymysql.cursors.DictCursor
        )
        
        with conn.cursor() as cur:
            cur.execute("SELECT COUNT(*) as count FROM users")
            user_count = cur.fetchone()['count']
            
        conn.close()
        print(f"✅ MySQL connected - {user_count} users in database")
        return True
        
    except Exception as e:
        print(f"❌ MySQL connection failed: {e}")
        return False

def test_grafana_connection():
    """Test Grafana connection and authentication"""
    print("📈 Testing Grafana Connection...")
    
    try:
        manager = GrafanaUserManager()
        if manager.authenticate():
            print("✅ Grafana admin authentication successful")
            return True
        else:
            print("❌ Grafana admin authentication failed")
            return False
            
    except Exception as e:
        print(f"❌ Grafana connection error: {e}")
        return False

def test_user_creation():
    """Test creating a user through the API"""
    print("👤 Testing User Creation...")
    
    test_username = "test_grafana_user"
    test_password = "test123456"
    
    try:
        # Create user
        manager = GrafanaUserManager()
        result = manager.create_grafana_user(test_username, test_password)
        
        if result['success']:
            print(f"✅ Test user '{test_username}' created successfully")
            print(f"   Grafana User ID: {result.get('user_id')}")
            print(f"   Organization ID: {result.get('org_id')}")
            
            # Clean up - delete test user
            delete_result = manager.delete_grafana_user(test_username)
            if delete_result['success']:
                print("✅ Test user cleaned up successfully")
            else:
                print(f"⚠️ Failed to clean up test user: {delete_result.get('error')}")
                
            return True
        else:
            print(f"❌ Test user creation failed: {result.get('error')}")
            return False
            
    except Exception as e:
        print(f"❌ User creation test error: {e}")
        return False

def test_api_endpoints():
    """Test User Management API endpoints"""
    print("🔌 Testing API Endpoints...")
    
    # Test if API is running
    try:
        response = requests.get("http://localhost:3002/api/test", timeout=5)
        if response.status_code == 200:
            result = response.json()
            mysql_ok = result.get('mysql', False)
            grafana_ok = result.get('grafana', False)
            
            print(f"✅ User Management API is running")
            print(f"   MySQL: {'✅' if mysql_ok else '❌'}")
            print(f"   Grafana: {'✅' if grafana_ok else '❌'}")
            
            return mysql_ok and grafana_ok
        else:
            print(f"⚠️ API responded with status {response.status_code}")
            return False
            
    except Exception as e:
        print(f"❌ API not accessible: {e}")
        print("   Start the API with: python user_management_api.py")
        return False

def show_usage_instructions():
    """Show how to use the system"""
    print("\n" + "="*60)
    print("🎯 EcoLogic Grafana User Management - Ready!")
    print("="*60)
    
    print("\n📋 How to Use:")
    
    print("\n1. 🚀 Start User Management API:")
    print("   python user_management_api.py")
    print("   # or")
    print("   ./start_user_management.sh")
    
    print("\n2. 🌐 Web Interface:")
    print("   Open: http://localhost:3002")
    print("   - Create new users (MySQL + Grafana)")
    print("   - Delete users from both systems")
    print("   - Sync existing MySQL users to Grafana")
    
    print("\n3. 📊 User Experience:")
    print("   - Each user gets personal Grafana organization")
    print("   - Automatic dashboard with their device data")
    print("   - Data filtered by username automatically")
    print("   - Direct Grafana login: http://192.168.1.160:3000")
    
    print("\n4. 🔧 API Endpoints:")
    print("   POST /api/users - Create user")
    print("   DELETE /api/users/{username} - Delete user") 
    print("   GET /api/users - List all users")
    print("   POST /api/sync-users - Sync MySQL users to Grafana")
    
    print("\n5. 📈 For Existing Users:")
    print("   Run sync to create Grafana accounts for existing MySQL users:")
    print("   curl -X POST http://localhost:3002/api/sync-users")

def main():
    load_dotenv()
    
    print("🏠 EcoLogic Grafana User Management Test")
    print("="*50)
    
    # Test all components
    mysql_ok = test_mysql_connection()
    grafana_ok = test_grafana_connection()
    
    if mysql_ok and grafana_ok:
        user_test_ok = test_user_creation()
        api_test_ok = test_api_endpoints()
        
        print(f"\n📊 Test Results:")
        print(f"   MySQL:           {'✅' if mysql_ok else '❌'}")
        print(f"   Grafana:         {'✅' if grafana_ok else '❌'}")
        print(f"   User Creation:   {'✅' if user_test_ok else '❌'}")
        print(f"   API Endpoints:   {'✅' if api_test_ok else '⚠️'}") 
        
        if mysql_ok and grafana_ok and user_test_ok:
            print("\n🎉 All core components working!")
            show_usage_instructions()
        else:
            print("\n⚠️ Some tests failed, check configuration")
    else:
        print(f"\n📊 Connection Test Results:")
        print(f"   MySQL:    {'✅' if mysql_ok else '❌'}")
        print(f"   Grafana:  {'✅' if grafana_ok else '❌'}")
        
        print("\n❌ Prerequisites not met")
        
        if not mysql_ok:
            print("\n🗄️ MySQL Issues:")
            print("   - Check connection credentials in .env")
            print("   - Ensure MySQL is running on 192.168.1.160:3306")
            
        if not grafana_ok:
            print("\n📈 Grafana Issues:")
            print("   - Ensure Grafana is running on 192.168.1.160:3000")
            print("   - Check admin credentials in .env file")
            print("   - Default: admin/admin")

if __name__ == "__main__":
    main()