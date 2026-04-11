#!/usr/bin/env python3
"""
InfluxDB Setup and Test Script for EcoLogic Manager
This script helps set up and test the InfluxDB integration
"""

import os
import sys
from dotenv import load_dotenv
from influx_logger import InfluxLogger

def test_influx_connection():
    """Test the InfluxDB connection and configuration"""
    print("🔍 Testing InfluxDB Configuration...")
    print("=" * 50)
    
    # Load environment variables
    load_dotenv()
    
    # Check required environment variables
    required_vars = ["INFLUXDB_TOKEN", "INFLUXDB_ORG", "DEVICE_MANAGER_DB_HOST"]
    missing_vars = []
    
    for var in required_vars:
        value = os.getenv(var)
        if not value:
            missing_vars.append(var)
        else:
            if var == "INFLUXDB_TOKEN":
                print(f"✅ {var}: {'*' * 20}...{value[-4:]}")
            else:
                print(f"✅ {var}: {value}")
    
    if missing_vars:
        print(f"\n❌ Missing environment variables: {', '.join(missing_vars)}")
        print("Please check your .env file")
        return False
    
    # Test InfluxDB connection
    print(f"\n🔗 Testing InfluxDB connection...")
    influx_logger = InfluxLogger()
    
    if not influx_logger.is_available():
        print("❌ InfluxDB connection failed!")
        print("Please check:")
        print("1. InfluxDB is running on the specified host")
        print("2. INFLUXDB_TOKEN is valid")
        print("3. Network connectivity")
        return False
    
    print("✅ InfluxDB connection successful!")
    
    # Test data logging
    print(f"\n📝 Testing data logging...")
    test_data = {
        "stat": ["1.0", "0.5", "25.3", "0", "1"]
    }
    
    test_pin_setup = {
        "descr": ["relay_1", "dimmer_1", "temperature", "sensor_1", "output_1"]
    }
    
    success = influx_logger.log_device_status(
        device_id="test_device",
        owner="test_user", 
        status_data=test_data,
        pin_setup=test_pin_setup
    )
    
    if success:
        print("✅ Test data logged successfully!")
        print("You can now check your Grafana dashboard to see the test data.")
    else:
        print("❌ Failed to log test data")
        return False
    
    # Test summary logging
    success = influx_logger.log_device_summary(
        device_id="test_device",
        owner="test_user",
        status_data=test_data,
        ip_address="192.168.1.100"
    )
    
    if success:
        print("✅ Test summary logged successfully!")
    else:
        print("❌ Failed to log test summary")
    
    influx_logger.close()
    
    print(f"\n🎉 All tests passed!")
    print(f"\n📊 Grafana Dashboard Setup:")
    print(f"1. Go to http://192.168.1.160:3000")
    print(f"2. Add InfluxDB data source:")
    print(f"   - URL: http://influxdb:8086")
    print(f"   - Organization: {os.getenv('INFLUXDB_ORG')}")
    print(f"   - Token: {os.getenv('INFLUXDB_TOKEN')}")
    print(f"   - Default Bucket: {os.getenv('INFLUXDB_BUCKET', 'default')}")
    print(f"3. Create dashboard with query:")
    print(f"   from(bucket: \"default\")")
    print(f"   |> range(start: -1h)")
    print(f"   |> filter(fn: (r) => r._measurement == \"device_pins\")")
    print(f"   |> filter(fn: (r) => r.user == \"your_username\")")
    
    return True

def create_sample_queries():
    """Create sample Grafana queries"""
    print("📊 Sample Grafana Queries")
    print("=" * 50)
    
    queries = {
        "All pins for a specific user": """
from(bucket: "default")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "device_pins")
  |> filter(fn: (r) => r.user == "username_here")
  |> filter(fn: (r) => r._field == "value")
        """,
        
        "Specific device data": """
from(bucket: "default")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "device_pins")
  |> filter(fn: (r) => r.user == "username_here")
  |> filter(fn: (r) => r.device_id == "device_id_here")
  |> filter(fn: (r) => r._field == "value")
        """,
        
        "Device online status": """
from(bucket: "default")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "device_summary")
  |> filter(fn: (r) => r.user == "username_here")
  |> filter(fn: (r) => r._field == "online")
        """,
        
        "Temperature sensors (example)": """
from(bucket: "default")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "device_pins")
  |> filter(fn: (r) => r.user == "username_here")
  |> filter(fn: (r) => r.pin_name =~ /temp/)
  |> filter(fn: (r) => r._field == "value")
        """
    }
    
    for name, query in queries.items():
        print(f"\n{name}:")
        print(query)

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "queries":
        create_sample_queries()
    else:
        if test_influx_connection():
            print(f"\n💡 Run 'python test_influx.py queries' to see sample Grafana queries")
        else:
            sys.exit(1)