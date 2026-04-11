#!/usr/bin/env python3
"""
Quick Setup and Test Script for EcoLogic Grafana Multi-User
"""

import os
import sys
import requests
from dotenv import load_dotenv
import pymysql

def test_mysql_connection():
    """Test MySQL connection and show available users"""
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
            # Get users
            cur.execute("SELECT username, password FROM users LIMIT 5")
            users = cur.fetchall()
            
            print("✅ MySQL connection successful!")
            print(f"📊 Found {len(users)} users (showing first 5):")
            
            for user in users:
                print(f"   👤 {user['username']} (password: {user['password'][:3]}***)")
            
            # Get devices per user
            cur.execute("""
                SELECT owner, COUNT(*) as device_count 
                FROM devices 
                GROUP BY owner 
                ORDER BY device_count DESC 
                LIMIT 5
            """)
            device_stats = cur.fetchall()
            
            print(f"\n🖥️  Device ownership:")
            for stat in device_stats:
                print(f"   {stat['owner']}: {stat['device_count']} devices")
        
        conn.close()
        return True
        
    except Exception as e:
        print(f"❌ MySQL connection failed: {e}")
        return False

def test_grafana_connection():
    """Test Grafana connection"""
    print("\n📈 Testing Grafana Connection...")
    
    grafana_host = os.getenv("GRAFANA_HOST", "192.168.1.160")
    grafana_port = os.getenv("GRAFANA_PORT", "3000")
    grafana_url = f"http://{grafana_host}:{grafana_port}"
    
    try:
        response = requests.get(f"{grafana_url}/api/health", timeout=5)
        if response.status_code == 200:
            print(f"✅ Grafana is accessible at {grafana_url}")
            return True
        else:
            print(f"⚠️ Grafana responded with status {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ Cannot connect to Grafana: {e}")
        print(f"   Make sure Grafana is running at {grafana_url}")
        return False

def test_influxdb_connection():
    """Test InfluxDB connection"""
    print("\n📊 Testing InfluxDB Connection...")
    
    influx_host = os.getenv("INFLUXDB_HOST", "192.168.1.160")
    influx_port = os.getenv("INFLUXDB_PORT", "8086")
    influx_url = f"http://{influx_host}:{influx_port}"
    
    try:
        response = requests.get(f"{influx_url}/ping", timeout=5)
        if response.status_code == 204:
            print(f"✅ InfluxDB is accessible at {influx_url}")
            return True
        else:
            print(f"⚠️ InfluxDB responded with status {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ Cannot connect to InfluxDB: {e}")
        return False

def show_setup_instructions():
    """Show next steps for setup"""
    print("\n" + "="*60)
    print("🎯 EcoLogic Grafana Multi-User Setup")
    print("="*60)
    
    print("\n📋 Next Steps:")
    print("1. 🔧 Configure Grafana (run once):")
    print("   python configure_grafana.py")
    
    print("\n2. 📈 Enable Anonymous Access in Grafana:")
    print("   - Go to http://192.168.1.160:3000")
    print("   - Login as admin")
    print("   - Configuration → Authentication → Anonymous Access")
    print("   - Enable anonymous access, set role to 'Viewer'")
    
    print("\n3. 🚀 Start the Multi-User Proxy:")
    print("   python grafana_proxy.py")
    print("   # or")
    print("   ./start_grafana_proxy.sh")
    
    print("\n4. 🌐 Access User Interface:")
    print("   - Open: http://localhost:3001")
    print("   - Login with MySQL user credentials")
    print("   - Example: username 'spspider' with corresponding password")
    
    print("\n5. 📊 Create Dashboards (in Grafana admin):")
    print("   - Use variable $user in your queries")
    print("   - Filter data with: r.user == \"$user\"")
    
def main():
    load_dotenv()
    
    print("🏠 EcoLogic Grafana Multi-User Setup Test")
    print("="*50)
    
    # Test all connections
    mysql_ok = test_mysql_connection()
    grafana_ok = test_grafana_connection()  
    influx_ok = test_influxdb_connection()
    
    print(f"\n📊 Connection Test Results:")
    print(f"   MySQL:    {'✅' if mysql_ok else '❌'}")
    print(f"   Grafana:  {'✅' if grafana_ok else '❌'}")
    print(f"   InfluxDB: {'✅' if influx_ok else '❌'}")
    
    if mysql_ok and grafana_ok and influx_ok:
        print("\n🎉 All services are ready!")
        show_setup_instructions()
    else:
        print("\n⚠️ Some services are not accessible.")
        print("Please check your configuration and ensure all services are running.")
        
        if not mysql_ok:
            print("\n🗄️ MySQL Issues:")
            print("   - Check connection credentials in .env")
            print("   - Ensure MySQL is running on 192.168.1.160:3306")
            
        if not grafana_ok:
            print("\n📈 Grafana Issues:")
            print("   - Ensure Grafana is running")
            print("   - Check if port 3000 is accessible")
            
        if not influx_ok:
            print("\n📊 InfluxDB Issues:")
            print("   - Ensure InfluxDB is running")
            print("   - Check if port 8086 is accessible")

if __name__ == "__main__":
    main()