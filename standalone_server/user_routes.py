"""
User Management Routes for EcoLogic
Separated from main app to keep it clean and focused
"""

import os
import logging
from fastapi import APIRouter, HTTPException, Depends
from fastapi.responses import JSONResponse, HTMLResponse
from fastapi.security import HTTPBasic, HTTPBasicCredentials
from pydantic import BaseModel
import pymysql
from dotenv import load_dotenv
from grafana_user_manager import create_user_in_grafana, delete_user_from_grafana, sync_all_users
import requests

load_dotenv()

# Setup logging
logger = logging.getLogger(__name__)

# Security for admin access
security = HTTPBasic()
 
# Database configuration
DEVICE_MANAGER_DB_HOST = os.getenv("DEVICE_MANAGER_DB_HOST", "192.168.1.160")
DEVICE_MANAGER_DB_PORT = int(os.getenv("DEVICE_MANAGER_DB_PORT", "3306"))
DEVICE_MANAGER_DB_NAME = os.getenv("DEVICE_MANAGER_DB_NAME", "ecologic")
DEVICE_MANAGER_DB_USER = os.getenv("DEVICE_MANAGER_DB_USER", "ecouser")
DEVICE_MANAGER_DB_PASSWORD = os.getenv("DEVICE_MANAGER_DB_PASSWORD", "password")

# Admin users who can access user management
ADMIN_USERS = ["admin", "spspider"]
BASIC_USER = os.getenv("BASIC_USER", "admin")
BASIC_PASS = os.getenv("BASIC_PASS", "admin123")

# Grafana configuration
GRAFANA_URL = os.getenv("GRAFANA_URL", "http://localhost:3000")
GRAFANA_ROOT_PATH = "/graf"  # Your Grafana sub-path

# User Management Models
class UserCreate(BaseModel):
    username: str
    password: str
    email: str = None

def get_db_connection():
    """Get MySQL database connection"""
    try:
        return pymysql.connect(
            host=DEVICE_MANAGER_DB_HOST,
            port=DEVICE_MANAGER_DB_PORT,
            user=DEVICE_MANAGER_DB_USER,
            passwd=DEVICE_MANAGER_DB_PASSWORD,
            db=DEVICE_MANAGER_DB_NAME,
            charset='utf8mb4',
            cursorclass=pymysql.cursors.DictCursor
        )
    except Exception as e:
        logger.error(f"Database connection failed: {e}")
        return None

def verify_admin(credentials: HTTPBasicCredentials = Depends(security)):
    """Verify admin access for user management"""
    # Check database users first
    conn = get_db_connection()
    if conn:
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT username FROM users WHERE username=%s AND password=%s", 
                           (credentials.username, credentials.password))
                user = cur.fetchone()
                if user and credentials.username in ADMIN_USERS:
                    return credentials.username
        finally:
            conn.close()
    
    # Fallback to system admin credentials
    if (credentials.username == BASIC_USER and credentials.password == BASIC_PASS and 
        credentials.username in ADMIN_USERS):
        return "admin"
    
    raise HTTPException(
        status_code=403, 
        detail=f"Access denied. User management requires admin privileges. Contact admin."
    )

# Create router for user management
router = APIRouter(prefix="/user-management")

# Add a setup endpoint for service account permissions
@router.post("/api/setup-permissions")
async def setup_grafana_permissions(admin_user: str = Depends(verify_admin)):
    """Setup Grafana service account permissions (Admin only) - DEPRECATED"""
    return {
        "success": False,
        "message": "This endpoint is deprecated. Use Auth Proxy integration instead."
    }

@router.get("/", response_class=HTMLResponse)
async def user_management_ui(admin_user: str = Depends(verify_admin)):
    """User Management Interface - Admin Only"""
    return HTMLResponse("""
    <!DOCTYPE html>
    <html>
    <head>
        <title>EcoLogic User Management</title>
        <style>
            body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }
            .container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
            h1 { color: #333; text-align: center; }
            .form-group { margin: 20px 0; }
            label { display: block; margin-bottom: 5px; font-weight: bold; }
            input { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
            button { background: #007bff; color: white; padding: 12px 20px; border: none; border-radius: 4px; cursor: pointer; margin-right: 10px; }
            button:hover { background: #0056b3; }
            .danger { background: #dc3545; }
            .danger:hover { background: #c82333; }
            .success { background: #28a745; }
            .success:hover { background: #218838; }
            .result { margin-top: 20px; padding: 15px; border-radius: 4px; }
            .result.success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
            .result.error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
            .section { margin: 30px 0; padding: 20px; border: 1px solid #ddd; border-radius: 4px; }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>🏠 EcoLogic User Management</h1>
            <p>👤 Logged in as: <strong>""" + admin_user + """</strong> | <a href="/api/device_selector">← Back to Device Manager</a></p>
            
            <div class="section">
                <h2>Create New User</h2>
                <p>Creates user in both MySQL and Grafana with personal dashboard</p>
                
                <form id="createUserForm">
                    <div class="form-group">
                        <label>Username:</label>
                        <input type="text" id="username" name="username" required>
                    </div>
                    
                    <div class="form-group">
                        <label>Password:</label>
                        <input type="password" id="password" name="password" required>
                    </div>
                    
                    <div class="form-group">
                        <label>Email (optional):</label>
                        <input type="email" id="email" name="email">
                    </div>
                    
                    <button type="submit">Create User</button>
                </form>
            </div>
            
            <div class="section">
                <h2>Delete User</h2>
                <p>Removes user from both MySQL and Grafana</p>
                
                <form id="deleteUserForm">
                    <div class="form-group">
                        <label>Username to delete:</label>
                        <input type="text" id="deleteUsername" name="username" required>
                    </div>
                    
                    <button type="submit" class="danger">Delete User</button>
                </form>
            </div>
            
            <div class="section">
                <h2>� Auto-Login to Grafana</h2>
                <p>Create user and automatically log them into Grafana via Auth Proxy</p>
                
                <form id="createAndLoginForm">
                    <div class="form-group">
                        <label>Username:</label>
                        <input type="text" id="proxyUsername" name="username" required>
                    </div>
                    
                    <div class="form-group">
                        <label>Password:</label>
                        <input type="password" id="proxyPassword" name="password" required>
                    </div>
                    
                    <div class="form-group">
                        <label>Email (optional):</label>
                        <input type="email" id="proxyEmail" name="email">
                    </div>
                    
                    <button type="submit">Create & Login to Grafana</button>
                </form>
            </div>
            
            <div class="section">
                <h2>Existing Users - Direct Grafana Access</h2>
                <p>Login existing users directly to Grafana via Auth Proxy</p>
                
                <form id="loginExistingForm">
                    <div class="form-group">
                        <label>Select User:</label>
                        <select id="existingUser" name="username" required>
                            <option value="">Loading users...</option>
                        </select>
                    </div>
                    
                    <button type="submit">Login to Grafana</button>
                </form>
            </div>
            
            <div class="section">
                <h2>Sync Existing Users</h2>
                <p>Create MySQL users in Grafana using Auth Proxy (recommended)</p>
                
                <button id="syncUsersBtn" class="success">Sync All Users via Auth Proxy</button>
            </div>
            
            <div id="result"></div>
        </div>
        
        <script>
            function showResult(message, isError = false) {
                const resultDiv = document.getElementById('result');
                resultDiv.className = 'result ' + (isError ? 'error' : 'success');
                resultDiv.innerHTML = message;
            }
            
            document.getElementById('createUserForm').addEventListener('submit', async (e) => {
                e.preventDefault();
                const formData = new FormData(e.target);
                
                try {
                    const response = await fetch('/user-management/api/users', {
                        method: 'POST',
                        headers: {'Content-Type': 'application/json'},
                        body: JSON.stringify({
                            username: formData.get('username'),
                            password: formData.get('password'),
                            email: formData.get('email') || null
                        })
                    });
                    
                    const result = await response.json();
                    showResult(result.message || 'User created successfully', !result.success);
                    
                    if (result.success) {
                        e.target.reset();
                    }
                } catch (error) {
                    showResult('Error: ' + error.message, true);
                }
            });
            
            document.getElementById('deleteUserForm').addEventListener('submit', async (e) => {
                e.preventDefault();
                const formData = new FormData(e.target);
                const username = formData.get('username');
                
                if (!confirm(`Are you sure you want to delete user '${username}'?`)) {
                    return;
                }
                
                try {
                    const response = await fetch(`/user-management/api/users/${username}`, {
                        method: 'DELETE'
                    });
                    
                    const result = await response.json();
                    showResult(result.message || 'User deleted successfully', !result.success);
                    
                    if (result.success) {
                        e.target.reset();
                    }
                } catch (error) {
                    showResult('Error: ' + error.message, true);
                }
            });
            
            document.getElementById('createAndLoginForm').addEventListener('submit', async (e) => {
                e.preventDefault();
                const formData = new FormData(e.target);
                
                try {
                    // First create user in MySQL
                    const response = await fetch('/user-management/api/users', {
                        method: 'POST',
                        headers: {'Content-Type': 'application/json'},
                        body: JSON.stringify({
                            username: formData.get('username'),
                            password: formData.get('password'),
                            email: formData.get('email') || null
                        })
                    });
                    
                    const result = await response.json();
                    
                    if (result.success) {
                        // Auto-login to Grafana
                        const grafanaUrl = `/user-management/api/grafana-login/${formData.get('username')}`;
                        window.open(grafanaUrl, '_blank');
                        showResult('User created and logged into Grafana!');
                        e.target.reset();
                    } else {
                        showResult(result.message || 'Failed to create user', true);
                    }
                } catch (error) {
                    showResult('Error: ' + error.message, true);
                }
            });
            
            document.getElementById('loginExistingForm').addEventListener('submit', async (e) => {
                e.preventDefault();
                const formData = new FormData(e.target);
                const username = formData.get('username');
                
                if (!username) {
                    showResult('Please select a user', true);
                    return;
                }
                
                // Direct login to Grafana
                const grafanaUrl = `/user-management/api/grafana-login/${username}`;
                window.open(grafanaUrl, '_blank');
                showResult(`Logged in ${username} to Grafana`);
            });
            
            // Load existing users
            async function loadExistingUsers() {
                try {
                    const response = await fetch('/user-management/api/users');
                    const result = await response.json();
                    
                    const select = document.getElementById('existingUser');
                    select.innerHTML = '<option value="">Select user...</option>';
                    
                    result.users.forEach(user => {
                        const option = document.createElement('option');
                        option.value = user;
                        option.textContent = user;
                        select.appendChild(option);
                    });
                } catch (error) {
                    document.getElementById('existingUser').innerHTML = '<option value="">Error loading users</option>';
                }
            }
            
            document.getElementById('syncUsersBtn').addEventListener('click', async () => {
                if (!confirm('This will sync all MySQL users to Grafana using Auth Proxy. Continue?')) {
                    return;
                }
                
                try {
                    const response = await fetch('/user-management/api/sync-auth-proxy', {
                        method: 'POST'
                    });
                    
                    const result = await response.json();
                    
                    if (result.success) {
                        let message = 'Auth Proxy sync completed!<br><br>';
                        result.results.forEach(user => {
                            message += `${user.username}: ${user.success ? '✅' : '❌'} ${user.message}<br>`;
                        });
                        showResult(message);
                    } else {
                        showResult(result.error || 'Sync failed', true);
                    }
                } catch (error) {
                    showResult('Error: ' + error.message, true);
                }
            });
            // Load users on page load
            loadExistingUsers();
    </body>
    </html>
    """)

@router.get("/api/grafana-login/{username}")
async def grafana_auto_login(username: str, admin_user: str = Depends(verify_admin)):
    """Auto-login user to Grafana via Auth Proxy headers"""
    try:
        # Verify user exists in MySQL
        conn = get_db_connection()
        if not conn:
            raise HTTPException(status_code=500, detail="Database connection failed")
        
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT username FROM users WHERE username=%s", (username,))
                if not cur.fetchone():
                    raise HTTPException(status_code=404, detail="User not found")
        finally:
            conn.close()
        
        # Create proxy session for Grafana
        headers = {
            'X-WEBAUTH-USER': username,
            'X-Real-IP': '127.0.0.1',
            'X-Forwarded-For': '127.0.0.1',
            'X-Forwarded-Proto': 'http',
            'Host': 'localhost:3000'
        }
        
        # Make request to Grafana to trigger user creation
        grafana_url = f"{GRAFANA_URL}{GRAFANA_ROOT_PATH}/api/user"
        
        try:
            response = requests.get(grafana_url, headers=headers, timeout=10)
            if response.status_code in [200, 201]:
                logger.info(f"✅ User {username} authenticated in Grafana via Auth Proxy")
            else:
                logger.warning(f"⚠️ Grafana response {response.status_code} for user {username}")
        except Exception as e:
            logger.error(f"❌ Grafana Auth Proxy request failed: {e}")
        
        # Redirect to Grafana with Auth headers
        from fastapi.responses import RedirectResponse
        response = RedirectResponse(
            url=f"{GRAFANA_URL}{GRAFANA_ROOT_PATH}/",
            status_code=302
        )
        
        # Set headers for Auth Proxy
        response.headers["X-WEBAUTH-USER"] = username
        
        return response
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"❌ Grafana login error: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/api/sync-auth-proxy")
async def sync_users_auth_proxy(admin_user: str = Depends(verify_admin)):
    """Sync all MySQL users to Grafana using Auth Proxy (better than API)"""
    try:
        # Get all MySQL users
        conn = get_db_connection()
        if not conn:
            raise HTTPException(status_code=500, detail="Database connection failed")
        
        users = []
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT username FROM users ORDER BY username")
                users = [row['username'] for row in cur.fetchall()]
        finally:
            conn.close()
        
        if not users:
            return {
                "success": True,
                "message": "No users found in MySQL",
                "results": []
            }
        
        results = []
        
        for username in users:
            try:
                # Create Auth Proxy headers
                headers = {
                    'X-WEBAUTH-USER': username,
                    'X-Real-IP': '127.0.0.1', 
                    'X-Forwarded-For': '127.0.0.1',
                    'X-Forwarded-Proto': 'http',
                    'Host': 'localhost:3000',
                    'User-Agent': 'EcoLogic-Manager/1.0'
                }
                
                # Make request to trigger user creation in Grafana
                grafana_url = f"{GRAFANA_URL}{GRAFANA_ROOT_PATH}/api/user"
                
                response = requests.get(grafana_url, headers=headers, timeout=15)
                
                if response.status_code in [200, 201]:
                    results.append({
                        "username": username,
                        "success": True,
                        "message": "User created/verified in Grafana via Auth Proxy"
                    })
                    logger.info(f"✅ Auth Proxy sync success: {username}")
                else:
                    results.append({
                        "username": username,
                        "success": False, 
                        "message": f"Auth Proxy failed: HTTP {response.status_code}"
                    })
                    logger.warning(f"⚠️ Auth Proxy sync failed for {username}: {response.status_code}")
                    
            except requests.exceptions.RequestException as e:
                results.append({
                    "username": username,
                    "success": False,
                    "message": f"Connection error: {str(e)}"
                })
                logger.error(f"❌ Auth Proxy request failed for {username}: {e}")
            except Exception as e:
                results.append({
                    "username": username,
                    "success": False,
                    "message": f"Error: {str(e)}"
                })
                logger.error(f"❌ Auth Proxy sync error for {username}: {e}")
        
        success_count = sum(1 for r in results if r['success'])
        
        logger.info(f"🔄 Auth Proxy sync completed: {success_count}/{len(results)} users synced")
        
        return {
            "success": True,
            "message": f"Auth Proxy sync completed: {success_count}/{len(results)} users",
            "results": results
        }
        
    except Exception as e:
        logger.error(f"❌ Auth Proxy sync error: {e}")
        raise HTTPException(status_code=500, detail=str(e))
@router.post("/api/users")
async def create_user_api(user: UserCreate, admin_user: str = Depends(verify_admin)):
    """Create user in MySQL only (Grafana via Auth Proxy)"""
    try:
        # Validate input
        if len(user.username) < 3 or len(user.username) > 16:
            raise HTTPException(status_code=400, detail="Username must be 3-16 characters")
        
        if len(user.password) < 6:
            raise HTTPException(status_code=400, detail="Password must be at least 6 characters")
        
        # Check if user already exists in MySQL
        conn = get_db_connection()
        if not conn:
            raise HTTPException(status_code=500, detail="Database connection failed")
        
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT username FROM users WHERE username=%s", (user.username,))
                if cur.fetchone():
                    raise HTTPException(status_code=400, detail="Username already exists")
                
                # Create user in MySQL
                cur.execute("INSERT INTO users (username, password) VALUES (%s, %s)", 
                           (user.username, user.password))
                conn.commit()
                
                logger.info(f"✅ Created MySQL user: {user.username}")
                
        finally:
            conn.close()
        
        # Note: Grafana user will be created automatically via Auth Proxy on first login
        return {
            "success": True,
            "message": f"User '{user.username}' created in MySQL. Grafana account will be created automatically on first login via Auth Proxy."
        }
            
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"❌ User creation error: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.delete("/api/users/{username}")
async def delete_user_api(username: str, admin_user: str = Depends(verify_admin)):
    """Delete user from both MySQL and Grafana via API"""
    try:
        # Delete from MySQL
        conn = get_db_connection()
        if not conn:
            raise HTTPException(status_code=500, detail="Database connection failed")
        
        mysql_deleted = False
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT username FROM users WHERE username=%s", (username,))
                if not cur.fetchone():
                    raise HTTPException(status_code=404, detail="User not found")
                
                cur.execute("DELETE FROM users WHERE username=%s", (username,))
                conn.commit()
                mysql_deleted = True
                logger.info(f"✅ Deleted MySQL user: {username}")
                
        finally:
            conn.close()
        
        # Delete from Grafana
        grafana_result = delete_user_from_grafana(username)
        
        if grafana_result['success']:
            logger.info(f"✅ Deleted Grafana user: {username}")
            return {
                "success": True,
                "message": f"User '{username}' deleted successfully from both MySQL and Grafana"
            }
        else:
            logger.warning(f"⚠️ Grafana user deletion failed: {username}")
            return {
                "success": True,  # MySQL deletion succeeded
                "message": f"User '{username}' deleted from MySQL. Grafana deletion failed: {grafana_result.get('error')}"
            }
            
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"❌ User deletion error: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/api/users")
async def list_users_api(admin_user: str = Depends(verify_admin)):
    """List all MySQL users via API"""
    conn = get_db_connection()
    if not conn:
        raise HTTPException(status_code=500, detail="Database connection failed")
    
    try:
        with conn.cursor() as cur:
            cur.execute("SELECT username FROM users ORDER BY username")
            users = [row['username'] for row in cur.fetchall()]
            return {"users": users, "count": len(users)}
    finally:
        conn.close()

@router.post("/api/sync-users")
async def sync_users_api(admin_user: str = Depends(verify_admin)):
    """Sync all MySQL users to Grafana via Auth Proxy (recommended method)"""
    return await sync_users_auth_proxy(admin_user)

@router.get("/api/test")
async def test_connections_api(admin_user: str = Depends(verify_admin)):
    """Test MySQL and Grafana connections via API"""
    try:
        # Test MySQL
        conn = get_db_connection()
        mysql_ok = conn is not None
        if conn:
            conn.close()
        
        # Test Grafana
        from grafana_user_manager import grafana_manager
        grafana_ok = grafana_manager.authenticate()
        
        return {
            "mysql": mysql_ok,
            "grafana": grafana_ok,
            "status": "ok" if mysql_ok and grafana_ok else "partial"
        }
        
    except Exception as e:
        logger.error(f"❌ Connection test error: {e}")
        raise HTTPException(status_code=500, detail=str(e))