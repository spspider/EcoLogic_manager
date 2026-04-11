#!/usr/bin/env python3
"""
Tests for Auth Proxy integration with Grafana
"""
import requests
import json
import os
from dotenv import load_dotenv

load_dotenv()

# Configuration
BASE_URL = "http://localhost:5005"
ADMIN_USER = "admin"  
ADMIN_PASS = "admin123"
TEST_USER = "authproxytest"

def test_auth_proxy_integration():
    """Test complete Auth Proxy integration workflow"""
    
    print("🧪 Testing Auth Proxy Integration")
    print("=" * 50)
    
    # Test 1: Create MySQL user  
    print("1️⃣ Creating MySQL user...")
    create_data = {
        "username": TEST_USER,
        "password": "testpass123",
        "email": "test@ecologic.go.ro"
    }
    
    response = requests.post(
        f"{BASE_URL}/user-management/api/users",
        json=create_data,
        auth=(ADMIN_USER, ADMIN_PASS)
    )
    
    if response.status_code == 200:
        result = response.json()
        if result.get("success"):
            print(f"✅ MySQL user created: {TEST_USER}")
        else:
            print(f"❌ MySQL creation failed: {result.get('message')}")
            return
    else:
        print(f"❌ HTTP Error: {response.status_code}")
        return
    
    # Test 2: Test Auth Proxy login
    print("\\n2️⃣ Testing Auth Proxy auto-login...")  
    response = requests.get(
        f"{BASE_URL}/user-management/api/grafana-login/{TEST_USER}",
        auth=(ADMIN_USER, ADMIN_PASS),
        allow_redirects=False
    )
    
    if response.status_code == 302:
        print("✅ Auth Proxy redirect successful")
        print(f"📍 Redirect URL: {response.headers.get('Location')}")
        if 'X-WEBAUTH-USER' in response.headers:
            print(f"🔑 Auth header set: {response.headers['X-WEBAUTH-USER']}")
    else:
        print(f"❌ Auth Proxy failed: {response.status_code}")
    
    # Test 3: Test bulk sync
    print("\\n3️⃣ Testing bulk Auth Proxy sync...")
    response = requests.post(
        f"{BASE_URL}/user-management/api/sync-auth-proxy", 
        auth=(ADMIN_USER, ADMIN_PASS)
    )
    
    if response.status_code == 200:
        result = response.json()
        if result.get("success"):
            print(f"✅ Bulk sync completed: {result.get('message')}")
            
            # Show results
            for user_result in result.get("results", []):
                status = "✅" if user_result.get("success") else "❌"
                print(f"  {status} {user_result['username']}: {user_result.get('message', 'No message')}")
        else:
            print(f"❌ Bulk sync failed: {result.get('message')}")
    else:
        print(f"❌ Bulk sync HTTP error: {response.status_code}")
    
    # Cleanup
    print("\\n🧹 Cleaning up test user...")
    response = requests.delete(
        f"{BASE_URL}/user-management/api/users/{TEST_USER}",
        auth=(ADMIN_USER, ADMIN_PASS)
    )
    
    if response.status_code == 200:
        print(f"✅ Test user {TEST_USER} deleted")
    else:
        print(f"⚠️ Cleanup failed: {response.status_code}")
    
    print("\\n" + "=" * 50)
    print("🏁 Auth Proxy integration test completed!")

def test_connections():
    """Test basic connections"""
    print("🔍 Testing Connections")
    print("-" * 30)
    
    # Test server availability
    try:
        response = requests.get(f"{BASE_URL}/user-management/api/test", 
                              auth=(ADMIN_USER, ADMIN_PASS),
                              timeout=10)
        
        if response.status_code == 200:
            result = response.json()
            print(f"📊 MySQL: {'✅' if result.get('mysql') else '❌'}")
            print(f"📊 Grafana: {'✅' if result.get('grafana') else '❌'}")
            print(f"📊 Status: {result.get('status', 'unknown')}")
        else:
            print(f"❌ Connection test failed: {response.status_code}")
    except Exception as e:
        print(f"❌ Connection error: {e}")

if __name__ == "__main__":
    test_connections()
    print()
    test_auth_proxy_integration()