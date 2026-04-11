"""
Grafana User Management API
Creates and manages Grafana users automatically when users are created in MySQL

IMPORTANT: Service Account Permissions Required
=============================================
The Grafana service account (ecologic-admin) needs the following permissions:
- users:create (to create new users)
- users:read (to read user information) 
- users:write (to update users)
- users:delete (to delete users)
- orgs:create (to create organizations)
- orgs:read (to read org info)
- orgs:write (to update organizations)
- datasources:create (to create data sources)
- datasources:read (to read data sources)
- dashboards:create (to create dashboards)
- dashboards:read (to read dashboards)

Without these permissions, Grafana integration will fail but MySQL user creation will still work.
"""

import os
import requests
import json
import logging
from dotenv import load_dotenv
from typing import Optional, Dict, List
import pymysql

load_dotenv()

logger = logging.getLogger(__name__)

class GrafanaUserManager:
    def __init__(self):
        self.grafana_host = os.getenv("GRAFANA_HOST", "192.168.1.160")
        self.grafana_port = os.getenv("GRAFANA_PORT", "3000") 
        self.grafana_url = f"http://{self.grafana_host}:{self.grafana_port}"
        self.service_username = os.getenv("GRAFANA_SERVICE_USERNAME", "ecologic-admin")
        self.service_token = os.getenv("GRAFANA_SERVICE_USER_TOKEN")
        
        # Setup headers for API token authentication
        self.headers = {
            "Authorization": f"Bearer {self.service_token}",
            "Content-Type": "application/json"
        }
        
        self.session = requests.Session()
        self.session.headers.update(self.headers)
        self.authenticated = False
        
        # MySQL connection info
        self.db_host = os.getenv("DEVICE_MANAGER_DB_HOST", "192.168.1.160")
        self.db_port = int(os.getenv("DEVICE_MANAGER_DB_PORT", "3306"))
        self.db_name = os.getenv("DEVICE_MANAGER_DB_NAME", "ecologic")
        self.db_user = os.getenv("DEVICE_MANAGER_DB_USER", "ecouser")
        self.db_password = os.getenv("DEVICE_MANAGER_DB_PASSWORD", "password")

    def authenticate(self) -> bool:
        """Test authentication with Grafana service token"""
        try:
            if not self.service_token:
                logger.error("❌ No Grafana service token configured")
                return False
            
            # Test the token by getting current user info
            response = self.session.get(f"{self.grafana_url}/api/user")
            if response.status_code == 200:
                user_info = response.json()
                self.authenticated = True
                logger.info(f"✅ Authenticated with Grafana as service user: {user_info.get('login', 'unknown')}")
                return True
            else:
                logger.error(f"❌ Failed to authenticate with Grafana service token: {response.status_code} - {response.text}")
                return False
                
        except Exception as e:
            logger.error(f"❌ Authentication error: {e}")
            return False    
    def find_service_account_id(self) -> Optional[int]:
        """Find the service account ID by searching for it"""
        try:
            if not self.service_username:
                return None
                
            response = self.session.get(f"{self.grafana_url}/api/serviceaccounts/search")
            if response.status_code == 200:
                service_accounts = response.json().get('serviceAccounts', [])
                
                # Look for service account by name or login
                for sa in service_accounts:
                    login = sa.get('login', '')
                    name = sa.get('name', '')
                    
                    # Check if the login contains our service username or matches exactly
                    if (self.service_username in login or 
                        login == self.service_username or 
                        name == self.service_username):
                        logger.info(f"✅ Found service account '{name}' (login: '{login}') with ID: {sa.get('id')}")
                        return sa.get('id')
                
                # Log all available service accounts for debugging
                logger.info(f"Available service accounts: {[{sa.get('name'): sa.get('login')} for sa in service_accounts]}")
                logger.warning(f"⚠️ Service account with name/login containing '{self.service_username}' not found")
                return None
            else:
                logger.error(f"❌ Failed to search service accounts: {response.status_code} - {response.text}")
                return None
                
        except Exception as e:
            logger.error(f"❌ Error finding service account: {e}")
            return None
    
    def grant_admin_role(self, service_account_id: int, org_id: int = 1) -> bool:
        """Grant Admin role to service account"""
        try:
            # First, check what organizations exist
            orgs_response = self.session.get(f"{self.grafana_url}/api/orgs")
            if orgs_response.status_code == 200:
                orgs = orgs_response.json()
                logger.info(f"Available organizations: {[org.get('name') + ' (ID: ' + str(org.get('id')) + ')' for org in orgs]}")
            
            # Try the default approach first 
            role_data = {"role": "Admin"}
            
            response = self.session.post(
                f"{self.grafana_url}/api/orgs/{org_id}/serviceaccounts/{service_account_id}/role",
                json=role_data
            )
            
            if response.status_code in [200, 201]:
                logger.info(f"✅ Granted Admin role to service account ID: {service_account_id}")
                return True
            elif response.status_code == 409:
                logger.info(f"ℹ️ Service account {service_account_id} already has Admin role")
                return True
            elif response.status_code == 404:
                # Try alternative API endpoint for global admin role
                logger.warning("⚠️ Org-specific role assignment failed, trying global admin assignment...")
                
                global_response = self.session.patch(
                    f"{self.grafana_url}/api/serviceaccounts/{service_account_id}",
                    json={"role": "Admin", "isGrafanaAdmin": True}
                )
                
                if global_response.status_code in [200, 201]:
                    logger.info(f"✅ Granted global Admin role to service account ID: {service_account_id}")
                    return True
                else:
                    logger.error(f"❌ Failed to grant global Admin role: {global_response.status_code} - {global_response.text}")
                    return False
            else:
                logger.error(f"❌ Failed to grant Admin role: {response.status_code} - {response.text}")
                return False
                
        except Exception as e:
            logger.error(f"❌ Error granting Admin role: {e}")
            return False
    
    def grant_specific_permissions(self, service_account_id: int) -> bool:
        """Grant specific permissions needed for user management"""
        required_permissions = [
            "users:create", "users:read", "users:write", "users:delete",
            "orgs:create", "orgs:read", "orgs:write",
            "datasources:create", "datasources:read",
            "dashboards:create", "dashboards:read", "dashboards:write"
        ]
        
        success_count = 0
        for permission in required_permissions:
            try:
                permission_data = {
                    "permission": permission,
                    "scope": "*"  # Global scope
                }
                
                response = self.session.post(
                    f"{self.grafana_url}/api/accesscontrol/serviceaccounts/{service_account_id}/permissions",
                    json=permission_data
                )
                
                if response.status_code in [200, 201, 409]:  # 409 means already exists
                    logger.info(f"✅ Granted permission '{permission}' to service account {service_account_id}")
                    success_count += 1
                else:
                    logger.warning(f"⚠️ Failed to grant permission '{permission}': {response.status_code} - {response.text}")
                    
            except Exception as e:
                logger.error(f"❌ Error granting permission '{permission}': {e}")
        
        logger.info(f"📊 Successfully granted {success_count}/{len(required_permissions)} permissions")
        return success_count > 0
    
    def setup_service_account_permissions(self) -> bool:
        """Setup service account with proper permissions"""
        try:
            logger.info("🔧 Setting up service account permissions...")
            
            # Find service account ID
            sa_id = self.find_service_account_id()
            if not sa_id:
                logger.error("❌ Cannot setup permissions: Service account ID not found")
                return False
            
            # Grant Admin role first
            role_granted = self.grant_admin_role(sa_id)
            
            # Grant specific permissions
            permissions_granted = self.grant_specific_permissions(sa_id)
            
            if role_granted or permissions_granted:
                logger.info("✅ Service account permissions setup completed")
                return True
            else:
                logger.error("❌ Failed to setup service account permissions")
                return False
                
        except Exception as e:
            logger.error(f"❌ Error setting up service account permissions: {e}")
            return False
    def create_user_organization(self, username: str) -> Optional[int]:
        """Create a separate organization for the user"""
        if not self.authenticated and not self.authenticate():
            return None
            
        try:
            org_data = {
                "name": f"EcoLogic-{username}"
            }
            
            response = self.session.post(f"{self.grafana_url}/api/orgs", json=org_data)
            
            if response.status_code == 200:
                org_info = response.json()
                org_id = org_info.get("orgId")
                logger.info(f"✅ Created organization 'EcoLogic-{username}' with ID {org_id}")
                return org_id
            else:
                logger.error(f"❌ Failed to create organization: {response.text}")
                return None
                
        except Exception as e:
            logger.error(f"❌ Organization creation error: {e}")
            return None

    def create_grafana_user(self, username: str, password: str, email: str = None) -> Dict:
        """Create user in Grafana"""
        if not self.authenticated and not self.authenticate():
            return {"success": False, "error": "Authentication failed"}
            
        try:
            # Create user
            user_data = {
                "name": username,
                "email": email or f"{username}@ecologic.local",
                "login": username,
                "password": password,
                "OrgId": 1  # Initially add to main org
            }
            
            response = self.session.post(f"{self.grafana_url}/api/admin/users", json=user_data)
            
            if response.status_code == 200:
                user_info = response.json()
                user_id = user_info.get("id")
                logger.info(f"✅ Created Grafana user '{username}' with ID {user_id}")
                
                # Create organization for user
                org_id = self.create_user_organization(username)
                
                if org_id:
                    # Add user to their organization as Admin
                    self.add_user_to_org(user_id, org_id, "Admin")
                    
                    # Remove user from main organization
                    self.remove_user_from_org(user_id, 1)
                    
                    # Setup data sources for the organization
                    self.setup_org_datasources(org_id)
                    
                    # Create default dashboard for user
                    self.create_user_dashboard(org_id, username)
                
                return {
                    "success": True, 
                    "user_id": user_id, 
                    "org_id": org_id,
                    "message": f"User '{username}' created successfully"
                }
            else:
                error_msg = response.text
                logger.error(f"❌ Failed to create user: {error_msg}")
                return {"success": False, "error": error_msg}
                
        except Exception as e:
            logger.error(f"❌ User creation error: {e}")
            return {"success": False, "error": str(e)}

    def add_user_to_org(self, user_id: int, org_id: int, role: str = "Viewer"):
        """Add user to organization with specified role"""
        try:
            org_user_data = {
                "loginOrEmail": user_id,
                "role": role
            }
            
            response = self.session.post(f"{self.grafana_url}/api/orgs/{org_id}/users", json=org_user_data)
            
            if response.status_code == 200:
                logger.info(f"✅ Added user {user_id} to organization {org_id} with role {role}")
                return True
            else:
                logger.error(f"❌ Failed to add user to org: {response.text}")
                return False
                
        except Exception as e:
            logger.error(f"❌ Error adding user to org: {e}")
            return False

    def remove_user_from_org(self, user_id: int, org_id: int):
        """Remove user from organization"""
        try:
            response = self.session.delete(f"{self.grafana_url}/api/orgs/{org_id}/users/{user_id}")
            
            if response.status_code == 200:
                logger.info(f"✅ Removed user {user_id} from organization {org_id}")
                return True
            else:
                logger.error(f"❌ Failed to remove user from org: {response.text}")
                return False
                
        except Exception as e:
            logger.error(f"❌ Error removing user from org: {e}")
            return False

    def setup_org_datasources(self, org_id: int):
        """Setup InfluxDB data source for organization"""
        try:
            # Switch to organization context
            self.session.post(f"{self.grafana_url}/api/user/using/{org_id}")
            
            datasource_config = {
                "name": "InfluxDB-EcoLogic",
                "type": "influxdb",
                "url": f"http://{os.getenv('INFLUXDB_HOST', '192.168.1.160')}:8086",
                "access": "proxy",
                "basicAuth": False,
                "jsonData": {
                    "version": "Flux",
                    "organization": os.getenv("INFLUXDB_ORG", "ecologic"),
                    "defaultBucket": os.getenv("INFLUXDB_BUCKET", "default"),
                    "maxSeries": 1000
                },
                "secureJsonData": {
                    "token": os.getenv("INFLUXDB_TOKEN", "")
                }
            }
            
            response = self.session.post(f"{self.grafana_url}/api/datasources", json=datasource_config)
            
            if response.status_code == 200:
                logger.info(f"✅ Created InfluxDB data source for organization {org_id}")
                return True
            else:
                logger.error(f"❌ Failed to create data source: {response.text}")
                return False
                
        except Exception as e:
            logger.error(f"❌ Error creating data source: {e}")
            return False

    def create_user_dashboard(self, org_id: int, username: str):
        """Create default dashboard for user"""
        try:
            # Switch to organization context
            self.session.post(f"{self.grafana_url}/api/user/using/{org_id}")
            
            dashboard_config = {
                "dashboard": {
                    "title": f"EcoLogic Dashboard - {username}",
                    "tags": ["ecologic", "user-dashboard"],
                    "timezone": "browser",
                    "panels": [
                        {
                            "id": 1,
                            "title": "My Device Status",
                            "type": "stat",
                            "gridPos": {"h": 8, "w": 12, "x": 0, "y": 0},
                            "targets": [
                                {
                                    "datasource": "InfluxDB-EcoLogic",
                                    "query": f'''
from(bucket: "default")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "device_pins")
  |> filter(fn: (r) => r.user == "{username}")
  |> filter(fn: (r) => r._field == "value")
  |> last()
                                    ''',
                                    "refId": "A"
                                }
                            ],
                            "fieldConfig": {
                                "defaults": {
                                    "color": {"mode": "palette-classic"},
                                    "custom": {
                                        "displayMode": "list",
                                        "orientation": "horizontal"
                                    }
                                }
                            }
                        },
                        {
                            "id": 2,
                            "title": "Device Activity Timeline",
                            "type": "timeseries",
                            "gridPos": {"h": 8, "w": 12, "x": 12, "y": 0},
                            "targets": [
                                {
                                    "datasource": "InfluxDB-EcoLogic",
                                    "query": f'''
from(bucket: "default")
  |> range(start: -6h)
  |> filter(fn: (r) => r._measurement == "device_pins")
  |> filter(fn: (r) => r.user == "{username}")
  |> filter(fn: (r) => r._field == "value")
                                    ''',
                                    "refId": "A"
                                }
                            ]
                        },
                        {
                            "id": 3,
                            "title": "Device Summary",
                            "type": "table",
                            "gridPos": {"h": 6, "w": 24, "x": 0, "y": 8},
                            "targets": [
                                {
                                    "datasource": "InfluxDB-EcoLogic",
                                    "query": f'''
from(bucket: "default")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "device_summary")
  |> filter(fn: (r) => r.user == "{username}")
  |> last()
                                    ''',
                                    "refId": "A"
                                }
                            ]
                        }
                    ],
                    "refresh": "30s",
                    "time": {"from": "now-6h", "to": "now"}
                },
                "folderId": 0,
                "overwrite": True
            }
            
            response = self.session.post(f"{self.grafana_url}/api/dashboards/db", json=dashboard_config)
            
            if response.status_code == 200:
                result = response.json()
                dashboard_url = result.get("url", "")
                logger.info(f"✅ Created dashboard for {username}: {dashboard_url}")
                return True
            else:
                logger.error(f"❌ Failed to create dashboard: {response.text}")
                return False
                
        except Exception as e:
            logger.error(f"❌ Error creating dashboard: {e}")
            return False

    def delete_grafana_user(self, username: str) -> Dict:
        """Delete user and their organization from Grafana"""
        if not self.authenticated and not self.authenticate():
            return {"success": False, "error": "Authentication failed"}
            
        try:
            # Find user
            response = self.session.get(f"{self.grafana_url}/api/users/lookup?loginOrEmail={username}")
            
            if response.status_code == 200:
                user_info = response.json()
                user_id = user_info.get("id")
                
                # Find user's organization
                org_response = self.session.get(f"{self.grafana_url}/api/orgs/name/EcoLogic-{username}")
                org_id = None
                if org_response.status_code == 200:
                    org_info = org_response.json()
                    org_id = org_info.get("id")
                
                # Delete organization first
                if org_id:
                    org_delete_response = self.session.delete(f"{self.grafana_url}/api/orgs/{org_id}")
                    if org_delete_response.status_code == 200:
                        logger.info(f"✅ Deleted organization EcoLogic-{username}")
                
                # Delete user
                user_delete_response = self.session.delete(f"{self.grafana_url}/api/admin/users/{user_id}")
                
                if user_delete_response.status_code == 200:
                    logger.info(f"✅ Deleted Grafana user '{username}'")
                    return {"success": True, "message": f"User '{username}' deleted successfully"}
                else:
                    return {"success": False, "error": "Failed to delete user"}
            else:
                return {"success": False, "error": "User not found"}
                
        except Exception as e:
            logger.error(f"❌ User deletion error: {e}")
            return {"success": False, "error": str(e)}

    def get_mysql_connection(self):
        """Get MySQL database connection"""
        try:
            return pymysql.connect(
                host=self.db_host,
                port=self.db_port,
                user=self.db_user,
                passwd=self.db_password,
                db=self.db_name,
                charset='utf8mb4',
                cursorclass=pymysql.cursors.DictCursor
            )
        except Exception as e:
            logger.error(f"Database connection failed: {e}")
            return None

    def sync_mysql_users_to_grafana(self):
        """Sync all MySQL users to Grafana (one-time setup)"""
        conn = self.get_mysql_connection()
        if not conn:
            return {"success": False, "error": "Database connection failed"}
        
        results = []
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT username, password FROM users")
                mysql_users = cur.fetchall()
                
                for user in mysql_users:
                    username = user['username']
                    password = user['password']
                    
                    result = self.create_grafana_user(username, password)
                    results.append({
                        "username": username,
                        "success": result['success'],
                        "message": result.get('message', result.get('error'))
                    })
                    
        finally:
            conn.close()
            
        return {"success": True, "results": results}

# Global instance
grafana_manager = GrafanaUserManager()

def create_user_in_grafana(username: str, password: str, email: str = None) -> Dict:
    """Convenience function to create user in Grafana"""
    return grafana_manager.create_grafana_user(username, password, email)

def delete_user_from_grafana(username: str) -> Dict:
    """Convenience function to delete user from Grafana"""
    return grafana_manager.delete_grafana_user(username)

def sync_all_users() -> Dict:
    """Sync all MySQL users to Grafana"""
    return grafana_manager.sync_mysql_users_to_grafana()