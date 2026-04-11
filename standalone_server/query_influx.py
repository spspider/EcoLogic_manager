#!/usr/bin/env python3
"""
InfluxDB Data Query Script for EcoLogic Manager
Query and display recent device data from InfluxDB
"""

import os
from dotenv import load_dotenv
from influxdb_client import InfluxDBClient
from datetime import datetime, timedelta

def query_influxdb_data():
    """Query recent data from InfluxDB"""
    load_dotenv()
    
    token = os.getenv("INFLUXDB_TOKEN")
    org = os.getenv("INFLUXDB_ORG", "ecologic")
    bucket = os.getenv("INFLUXDB_BUCKET", "default")
    # Try INFLUXDB_HOST first, then DEVICE_MANAGER_DB_HOST, then default
    host = os.getenv("INFLUXDB_HOST", os.getenv("DEVICE_MANAGER_DB_HOST", "192.168.1.160"))
    port = int(os.getenv("INFLUXDB_PORT", "8086"))
    url = f"http://{host}:{port}"
    
    if not token:
        print("❌ INFLUXDB_TOKEN not found in .env file")
        return
    
    print(f"🔍 Querying InfluxDB: {url}")
    print(f"📊 Organization: {org}, Bucket: {bucket}")
    print("=" * 60)
    
    try:
        client = InfluxDBClient(url=url, token=token, org=org)
        query_api = client.query_api()
        
        # Query recent device_pins data (last 1 hour)
        query_pins = f'''
        from(bucket: "{bucket}")
          |> range(start: -1h)
          |> filter(fn: (r) => r._measurement == "device_pins")
          |> sort(columns: ["_time"], desc: true)
          |> limit(n: 20)
        '''
        
        print("📍 Recent device pin data (last 20 entries):")
        result_pins = query_api.query(query=query_pins)
        
        pin_count = 0
        for table in result_pins:
            for record in table.records:
                pin_count += 1
                time = record.get_time().strftime("%H:%M:%S")
                user = record.values.get("user", "unknown")
                device_id = record.values.get("device_id", "unknown")
                pin_name = record.values.get("pin_name", "unknown")
                pin_index = record.values.get("pin_index", "unknown")
                value = record.get_value()
                
                print(f"  {time} | {user:10} | {device_id:12} | pin{pin_index:2}:{pin_name:12} = {value}")
        
        if pin_count == 0:
            print("  ❌ No device pin data found")
        else:
            print(f"  ✅ Found {pin_count} pin data entries")
        
        print()
        
        # Query recent device_summary data
        query_summary = f'''
        from(bucket: "{bucket}")
          |> range(start: -1h)
          |> filter(fn: (r) => r._measurement == "device_summary")
          |> sort(columns: ["_time"], desc: true)
          |> limit(n: 10)
        '''
        
        print("📊 Recent device summary data (last 10 entries):")
        result_summary = query_api.query(query=query_summary)
        
        summary_count = 0
        for table in result_summary:
            for record in table.records:
                summary_count += 1
                time = record.get_time().strftime("%H:%M:%S")
                user = record.values.get("user", "unknown")
                device_id = record.values.get("device_id", "unknown")
                ip_address = record.values.get("ip_address", "unknown")
                field = record.get_field()
                value = record.get_value()
                
                print(f"  {time} | {user:10} | {device_id:12} | {ip_address:15} | {field} = {value}")
        
        if summary_count == 0:
            print("  ❌ No device summary data found")
        else:
            print(f"  ✅ Found {summary_count} summary data entries")
        
        print()
        
        # Query unique users and devices
        query_users = f'''
        from(bucket: "{bucket}")
          |> range(start: -24h)
          |> filter(fn: (r) => r._measurement == "device_pins")
          |> distinct(column: "user")
        '''
        
        users_result = query_api.query(query=query_users)
        users = []
        for table in users_result:
            for record in table.records:
                users.append(record.get_value())
        
        if users:
            print(f"👥 Users with data in last 24h: {', '.join(users)}")
        else:
            print("👥 No users found with data in last 24h")
        
        # Query unique devices
        query_devices = f'''
        from(bucket: "{bucket}")
          |> range(start: -24h)
          |> filter(fn: (r) => r._measurement == "device_pins")
          |> distinct(column: "device_id")
        '''
        
        devices_result = query_api.query(query=query_devices)
        devices = []
        for table in devices_result:
            for record in table.records:
                devices.append(record.get_value())
        
        if devices:
            print(f"🖥️  Devices with data in last 24h: {', '.join(devices)}")
        else:
            print("🖥️  No devices found with data in last 24h")
        
        client.close()
        
    except Exception as e:
        print(f"❌ Error querying InfluxDB: {e}")

if __name__ == "__main__":
    query_influxdb_data()