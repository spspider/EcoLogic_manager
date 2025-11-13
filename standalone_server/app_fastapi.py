import os
import json
from datetime import datetime
from fastapi import FastAPI, Request, Depends, HTTPException, Form, Query
from fastapi.responses import JSONResponse, HTMLResponse, PlainTextResponse
from fastapi.security import HTTPBasic, HTTPBasicCredentials
from fastapi.staticfiles import StaticFiles
from starlette.middleware.cors import CORSMiddleware
import pymysql
from dotenv import load_dotenv
from typing import Optional
import secrets
import asyncio
from datetime import time as dt_time

load_dotenv()

DB_HOST = os.getenv("DB_HOST", "localhost")
DB_PORT = int(os.getenv("DB_PORT", "3306"))
DB_NAME = os.getenv("DB_NAME", "ecologic")
DB_USER = os.getenv("DB_USER", "ecouser")
DB_PASS = os.getenv("DB_PASSWORD", "ecopass")

BASIC_USER = os.getenv("BASIC_USER", "admin")
BASIC_PASS = os.getenv("BASIC_PASS", "admin123")

security = HTTPBasic()
app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["GET", "POST", "OPTIONS"],
    allow_headers=["*"],
)

app.mount("/api/static", StaticFiles(directory="../HTML_data/scripts"), name="static")



def get_conn():
    try:
        return pymysql.connect(
            host=DB_HOST, port=DB_PORT, user=DB_USER, 
            passwd=DB_PASS, db=DB_NAME, charset='utf8mb4', 
            cursorclass=pymysql.cursors.DictCursor
        )
    except:
        return None

def verify_basic(credentials: HTTPBasicCredentials = Depends(security)):
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT username FROM users WHERE username=%s AND password=%s", 
                           (credentials.username, credentials.password))
                user = cur.fetchone()
                if user:
                    return credentials.username
        finally:
            conn.close()
    
    # Fallback to admin credentials
    if credentials.username == BASIC_USER and credentials.password == BASIC_PASS:
        return "admin"
    
    raise HTTPException(status_code=401, detail="Unauthorized")

@app.post("/api/upload_pin_setup")
async def upload_pin_setup(request: Request):
    """
    API для Arduino: загрузка конфигурации пинов
    Arduino отправляет POST с JSON конфигурацией
    Параметры: device_id, token (в query string)
    """
    device_id = request.query_params.get("device_id", "default")
    token = request.query_params.get("token", "default_token")
    
    body = await request.body()
    try:
        pin_setup = json.loads(body.decode('utf-8'))
    except:
        return JSONResponse({"error": "invalid json"}, status_code=400)
    
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                sql = """
                INSERT INTO devices (device_id, token, pin_setup, last_seen)
                VALUES (%s, %s, %s, %s)
                ON DUPLICATE KEY UPDATE token = VALUES(token), pin_setup = VALUES(pin_setup), last_seen = VALUES(last_seen)
                """
                cur.execute(sql, (device_id, token, json.dumps(pin_setup), datetime.utcnow()))
                conn.commit()
        finally:
            conn.close()
    
    return {"ok": True}

@app.post("/api/cfg")
async def upload_config_short(request: Request):
    """
    Короткий endpoint для Arduino (экономия памяти)
    Параметры: id, tk, ip
    """
    device_id = request.query_params.get("id", "default")
    token = request.query_params.get("tk", "default_token")
    ip_address = request.query_params.get("ip", "unknown")
    
    body = await request.body()
    try:
        pin_setup = json.loads(body.decode('utf-8'))
    except:
        return {"err": 1}
    
    # Получаем owner из device_settings или ставим admin
    owner = "admin"
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT other_setup FROM device_settings WHERE device_id=%s", (device_id,))
                settings = cur.fetchone()
                if settings and settings.get("other_setup"):
                    other_data = json.loads(settings["other_setup"])
                    owner = other_data.get("user_name", "admin")
                
                sql = """
                INSERT INTO devices (device_id, token, pin_setup, ip_address, owner, last_seen)
                VALUES (%s, %s, %s, %s, %s, %s)
                ON DUPLICATE KEY UPDATE token = VALUES(token), pin_setup = VALUES(pin_setup), ip_address = VALUES(ip_address), last_seen = VALUES(last_seen)
                """
                cur.execute(sql, (device_id, token, json.dumps(pin_setup), ip_address, owner, datetime.utcnow()))
                conn.commit()
        finally:
            conn.close()
    
    return {"ok": 1}

@app.post("/api/other")
async def upload_other_setup(request: Request):
    """
    Загрузка other_setup.txt с Arduino
    Параметры: id, tk
    """
    device_id = request.query_params.get("id", "default")
    token = request.query_params.get("tk", "default_token")
    
    body = await request.body()
    try:
        other_setup = json.loads(body.decode('utf-8'))
    except:
        return {"err": 1}
    
    owner = other_setup.get("user_name", "admin")
    
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                sql = """
                INSERT INTO device_settings (device_id, other_setup)
                VALUES (%s, %s)
                ON DUPLICATE KEY UPDATE other_setup = VALUES(other_setup)
                """
                cur.execute(sql, (device_id, json.dumps(other_setup)))
                
                cur.execute("UPDATE devices SET owner=%s WHERE device_id=%s", (owner, device_id))
                conn.commit()
        finally:
            conn.close()
    
    return {"ok": 1}

@app.post("/api/update_status")
async def update_status(payload: dict):
    """
    API для Arduino: обновление статуса всех датчиков
    Ожидает: {"device_id": "...", "token": "...", "status": {"stat": ["1.00", "0.00", "25.5", ...]}}
    """
    device_id = payload.get("device_id", "default")
    token = payload.get("token", "default_token")
    status = payload.get("status")
    
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                # Обновляем статус и время последнего обращения
                cur.execute("UPDATE devices SET status=%s, last_seen=%s WHERE device_id=%s AND token=%s",
                           (json.dumps(status), datetime.utcnow(), device_id, token))
                conn.commit()
        finally:
            conn.close()
    
    return {"ok": True}

@app.post("/api/sync")
async def sync_status(request: Request):
    """
    Оптимизированный API для Arduino: отправляет real status и получает desired status
    Arduino отправляет POST с JSON: {"stat": ["1.00", "0.00", "25.5", ...]}
    Возвращает: {"stat": ["1", "0", "25", ...], "upd": 1} - desired status с флагом обновления
    """
    device_id = request.query_params.get("id", "default")
    token = request.query_params.get("tk", "default_token")
    
    body = await request.body()
    try:
        real_status = json.loads(body.decode('utf-8'))
    except:
        return JSONResponse({"error": "invalid json"}, status_code=400)
    
    conn = get_conn()
    desired_status = {"stat": [], "upd": 0}
    
    if conn:
        try:
            with conn.cursor() as cur:
                # Обновляем реальный статус
                cur.execute(
                    "UPDATE devices SET real_status=%s, last_seen=%s WHERE device_id=%s AND token=%s",
                    (json.dumps(real_status), datetime.utcnow(), device_id, token)
                )
                
                # Проверяем флаг синхронизации от устройства
                if real_status.get("synced") == 1:
                    cur.execute("UPDATE devices SET device_synced=TRUE WHERE device_id=%s AND token=%s", (device_id, token))
                
                # Получаем желаемый статус и флаг обновления
                cur.execute("SELECT desired_status, has_updates FROM devices WHERE device_id=%s AND token=%s", (device_id, token))
                row = cur.fetchone()
                if row:
                    if row.get("desired_status"):
                        status = json.loads(row["desired_status"])
                        desired_status["stat"] = status.get("stat", [])
                    
                    desired_status["upd"] = 1 if row.get("has_updates") else 0
                    
                    # Если есть обновления, сбрасываем has_updates
                    if row.get("has_updates"):
                        cur.execute("UPDATE devices SET has_updates=FALSE WHERE device_id=%s AND token=%s", (device_id, token))
                
                conn.commit()
        finally:
            conn.close()
    
    return desired_status

@app.post("/api/register")
async def register_user(username: str = Form(), password: str = Form()):
    """
    Регистрация нового пользователя (без авторизации)
    """
    import re
    if len(username) < 3 or len(username) > 16 or len(password) < 6:
        return HTMLResponse("""
        <html><body style="font-family: Arial; margin: 50px;">
        <h2>Registration Error</h2>
        <p>Username: 3-16 characters, password: min 6 characters.</p>
        <p><a href="/api/register">Try again</a></p>
        </body></html>
        """)
    
    if not re.match(r'^[a-zA-Z0-9_]+$', username):
        return HTMLResponse("""
        <html><body style="font-family: Arial; margin: 50px;">
        <h2>Registration Error</h2>
        <p>Username can only contain letters, numbers and underscore.</p>
        <p><a href="/api/register">Try again</a></p>
        </body></html>
        """)
    
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                # Проверяем, существует ли пользователь
                cur.execute("SELECT username FROM users WHERE username=%s", (username,))
                if cur.fetchone():
                    return HTMLResponse("""
                    <html><body style="font-family: Arial; margin: 50px;">
                    <h2>Registration Error</h2>
                    <p>Username already exists. Please choose another.</p>
                    <p><a href="/api/register">Try again</a></p>
                    </body></html>
                    """)
                
                # Создаем пользователя
                cur.execute("INSERT INTO users (username, password) VALUES (%s, %s)", (username, password))
                conn.commit()
                return HTMLResponse("""
                <html><body style="font-family: Arial; margin: 50px;">
                <h2>Registration Successful!</h2>
                <p>User registered successfully. You can now login.</p>
                <p><a href="/api/device_selector">Go to login</a></p>
                </body></html>
                """)
        finally:
            conn.close()
    
    return HTMLResponse("""
    <html><body style="font-family: Arial; margin: 50px;">
    <h2>Registration Error</h2>
    <p>Database error. Please try again later.</p>
    <p><a href="/api/register">Try again</a></p>
    </body></html>
    """)

@app.get("/api/register", response_class=HTMLResponse)
async def register_form():
    return HTMLResponse("""
    <!DOCTYPE html>
    <html>
    <head><title>Register</title></head>
    <body style="font-family: Arial; margin: 50px;">
        <h2>Register New User</h2>
        <form method="post">
            <p><input name="username" placeholder="Username (3-16 chars, a-z 0-9 _)" pattern="[a-zA-Z0-9_]{3,16}" required></p>
            <p><input name="password" type="password" placeholder="Password (min 6 chars)" required></p>
            <p><button type="submit">Register</button></p>
        </form>
        <p><a href="/api/device_selector">Back to login</a></p>
    </body>
    </html>
    """)

@app.get("/api/config")
async def get_config(id: Optional[str] = "default", user: str = Depends(verify_basic)):
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                if user == "admin":
                    cur.execute("SELECT pin_setup, ip_address FROM devices WHERE device_id=%s", (id,))
                else:
                    cur.execute("SELECT pin_setup, ip_address FROM devices WHERE device_id=%s AND owner=%s", (id, user))
                row = cur.fetchone()
                if row and row.get("pin_setup"):
                    config = json.loads(row["pin_setup"])
                    config["ip_address"] = row.get("ip_address", "unknown")
                    return config
        finally:
            conn.close()
    
    return {}

@app.get("/api/ajax")
async def send_ajax(request: Request, user: str = Depends(verify_basic)):
    """
    Обратная совместимость с старым JS кодом
    Параметр data: {"t":127,"v":0} - запрос реального статуса
    Параметр data: {"t":id,"v":value} - установка желаемого значения пина
    """
    data_param = request.query_params.get("data")
    device_id = request.query_params.get("device_id", "default")
      
    if not data_param:
        return JSONResponse({"error": "missing data param"}, status_code=400)
    try:
        data = json.loads(data_param)
        
    except Exception as e:
        return JSONResponse({"error": f"invalid json: {e}"}, status_code=400)
    
    # Проверяем права доступа к устройству
    conn = get_conn()
    if conn and user != "admin":
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT owner FROM devices WHERE device_id=%s", (device_id,))
                device = cur.fetchone()
                if not device or device["owner"] != user:
                    return JSONResponse({"error": "access denied"}, status_code=403)
        finally:
            conn.close()

    
    conn = get_conn()
    
    if data['t'] == 127:  # Status request - возвращаем реальный статус с флагом обновлений
        if conn:
            try:
                with conn.cursor() as cur:
                    cur.execute("SELECT real_status, has_updates, device_synced FROM devices WHERE device_id=%s", (device_id,))
                    row = cur.fetchone()
                    if row and row.get("real_status"):
                        status_data = json.loads(row["real_status"])
                        # Блокируем обновления если устройство еще не синхронизировалось
                        status_data["has_updates"] = 1 if not row.get("device_synced", True) else 0
                        return JSONResponse(status_data)
            finally:
                conn.close()
        return JSONResponse({"stat": [], "has_updates": 0})
    # elif data['v'] == 0 and data['t'] < 127:  # Single pin read request
    #     if conn:
    #         try:
    #             with conn.cursor() as cur:
    #                 cur.execute("SELECT real_status FROM devices WHERE device_id=%s", (device_id,))
    #                 row = cur.fetchone()
    #                 if row and row.get("real_status"):
    #                     status_data = json.loads(row["real_status"])
    #                     if data['t'] < len(status_data.get("stat", [])):
    #                         return PlainTextResponse(status_data["stat"][data['t']])
    #         finally:
    #             conn.close()
    #     return PlainTextResponse("0")
    else:  # Pin control - обновляем желаемый статус
        if conn:
            try:
                with conn.cursor() as cur:
                    # Получаем текущий desired_status
                    cur.execute("SELECT desired_status, pin_setup FROM devices WHERE device_id=%s", (device_id,))
                    row = cur.fetchone()
                    
                    if row and row.get("desired_status"):
                        states = json.loads(row["desired_status"])
                        # если stat пуст или не совпадает по длине с pin_setup — пересоздать
                        if not states.get("stat") or len(states["stat"]) == 0:
                            pin_count = 10
                            if row.get("pin_setup"):
                                pin_setup = json.loads(row["pin_setup"])
                                pin_count = len(pin_setup.get("pin", [])) or pin_setup.get("numberChosed", 10)
                            states = {"stat": ["0"] * pin_count}
                    else:
                        pin_count = 10
                        if row and row.get("pin_setup"):
                            pin_setup = json.loads(row["pin_setup"])
                            pin_count = len(pin_setup.get("pin", [])) or pin_setup.get("numberChosed", 10)
                        states = {"stat": ["0"] * pin_count}
                   
                    # Обновляем желаемое значение пина
                    if data['t'] < len(states["stat"]):
                        states["stat"][data['t']] = str(data['v'])
                    
                    # Сохраняем в базу и устанавливаем флаги
                    cur.execute("UPDATE devices SET desired_status=%s, has_updates=TRUE, device_synced=FALSE WHERE device_id=%s", 
                               (json.dumps(states), device_id))
                    conn.commit()
            finally:
                conn.close()
        
        return PlainTextResponse(str(data['v']))

@app.get("/api/devices")
async def get_devices(user: str = Depends(verify_basic)):
    """
    Получение списка устройств пользователя
    """
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                if user == "admin":
                    cur.execute("""
                        SELECT device_id, last_seen, ip_address, owner,
                               JSON_LENGTH(pin_setup, '$.descr') as pin_count
                        FROM devices ORDER BY last_seen DESC
                    """)
                else:
                    cur.execute("""
                        SELECT device_id, last_seen, ip_address, owner,
                               JSON_LENGTH(pin_setup, '$.descr') as pin_count
                        FROM devices WHERE owner = %s ORDER BY last_seen DESC
                    """, (user,))
                devices = cur.fetchall()
                
                cur.execute("SELECT device_id, other_setup FROM device_settings")
                settings = {row["device_id"]: json.loads(row["other_setup"]).get("device_name", "") 
                           for row in cur.fetchall() if row.get("other_setup")}
                
                for device in devices:
                    device["device_name"] = settings.get(device["device_id"], "")
                
                return devices
        finally:
            conn.close()
    return []

@app.get("/api/device_selector", response_class=HTMLResponse)
async def device_selector(user: str = Depends(verify_basic)):
    with open("src/device_selector.html", "r", encoding="utf-8") as f:
        html = f.read()
    
    # Добавляем подсказку с именем пользователя и убираем ссылку регистрации
    html = html.replace('<h1>Select Device to Control</h1>', 
                       f'<h1>Select Device to Control</h1><div style="background: #e8f4fd; padding: 15px; border-radius: 5px; margin-bottom: 20px;"><strong>Device Registration:</strong> To register new device to your account, change name in \"connection\" settings, power on, and connect device to WiFi network</code></div>')
    html = html.replace('<p><a href="/api/register">Register new user</a></p>', '')
    
    return HTMLResponse(html)

@app.get("/", response_class=HTMLResponse)
@app.get("/login", response_class=HTMLResponse)
async def login_page():
    return HTMLResponse("""
    <!DOCTYPE html>
    <html>
    <head><title>EcoLogic Manager - Login</title></head>
    <body style="font-family: Arial; margin: 50px;">
        <h1>EcoLogic Manager</h1>
        <h2>Login</h2>
        <p>Please use your browser's authentication or <a href="/api/register">register a new account</a></p>
        <p><a href="/api/device_selector">Continue to device selection</a></p>
        <hr>
        <h3>Clear Authentication Data:</h3>
        <p>If you need to clear saved login data:</p>
        <ul>
            <li><strong>Chrome/Edge:</strong> Settings → Privacy → Clear browsing data → Passwords and other sign-in data</li>
            <li><strong>Firefox:</strong> Settings → Privacy → Clear Data → Saved Logins and Passwords</li>
            <li><strong>Or use Incognito/Private mode</strong></li>
        </ul>
    </body>
    </html>
    """)

@app.get("/api/pin_setup")
async def get_pin_setup(device_id: str = "default", user: str = Depends(verify_basic)):
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                if user == "admin":
                    cur.execute("SELECT pin_setup FROM devices WHERE device_id=%s", (device_id,))
                else:
                    cur.execute("SELECT pin_setup FROM devices WHERE device_id=%s AND owner=%s", (device_id, user))
                row = cur.fetchone()
                if row and row.get("pin_setup"):
                    return json.loads(row["pin_setup"])
        finally:
            conn.close()
    return {}

@app.get("/api/conditions")
async def get_conditions(device_id: str = "default", user: str = Depends(verify_basic)):
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                if user == "admin":
                    cur.execute("SELECT conditions FROM devices WHERE device_id=%s", (device_id,))
                else:
                    cur.execute("SELECT conditions FROM devices WHERE device_id=%s AND owner=%s", (device_id, user))
                row = cur.fetchone()
                if row and row.get("conditions"):
                    return json.loads(row["conditions"])
        finally:
            conn.close()
    return []

@app.post("/api/conditions")
async def save_conditions(request: Request, device_id: str = "default", user: str = Depends(verify_basic)):
    body = await request.body()
    try:
        conditions = json.loads(body.decode('utf-8'))
    except:
        return JSONResponse({"error": "invalid json"}, status_code=400)
    
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                if user == "admin":
                    cur.execute("UPDATE devices SET conditions=%s WHERE device_id=%s", 
                               (json.dumps(conditions), device_id))
                else:
                    cur.execute("UPDATE devices SET conditions=%s WHERE device_id=%s AND owner=%s", 
                               (json.dumps(conditions), device_id, user))
                conn.commit()
        finally:
            conn.close()
    return {"ok": True}

@app.get("/api/{page}", response_class=HTMLResponse)
async def page_handler(page: str, request: Request, user: str = Depends(verify_basic), device_id: Optional[str] = None):
    # Исключаем специальные эндпоинты
    if page in ["ajax", "devices", "device_selector", "register", "static", "sync", "pin_setup", "conditions"]:
        raise HTTPException(status_code=404)
    
    if not device_id:
        return HTMLResponse('<script>window.location.href="/api/device_selector"</script>')
    
    with open(f"../HTML_data/{page}.htm", "r", encoding="utf-8") as f:
        html = f.read()
    
    html = html.replace('src="scripts/', 'src="/api/static/')
    html = html.replace('href="scripts/', 'href="/api/static/')
    html = html.replace('</head>', f'<script>window.DEVICE_ID = "{device_id}";</script></head>')
    return HTMLResponse(html)

async def check_conditions():
    """Фоновая задача для проверки условий"""
    while True:
        await asyncio.sleep(10)  # Проверка каждые 10 секунд
        conn = get_conn()
        if not conn:
            continue
        
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT device_id, conditions, real_status, desired_status, pin_setup FROM devices WHERE conditions IS NOT NULL")
                devices = cur.fetchall()
                
                for device in devices:
                    device_id = device["device_id"]
                    conditions = json.loads(device["conditions"]) if device.get("conditions") else []
                    real_status = json.loads(device["real_status"]) if device.get("real_status") else {"stat": []}
                    desired_status = json.loads(device["desired_status"]) if device.get("desired_status") else {"stat": []}
                    pin_setup = json.loads(device["pin_setup"]) if device.get("pin_setup") else {}
                    
                    # Проверяем каждое условие
                    for cond in conditions:
                        if not cond.get("enabled"):
                            continue
                        
                        condition_met = False
                        cond_type = cond.get("conditionType")
                        cond_value = cond.get("conditionValue")
                        
                        source_pin = cond.get("sourcePin")
                        
                        # Проверка условия по времени
                        if cond_type == "on time reached" and cond_value:
                            now = datetime.now().strftime("%H:%M")
                            if now == cond_value:
                                condition_met = True
                        
                        # Проверка условий сравнения (equal, greater, less)
                        elif cond_type in ["equal", "greater than", "less than"] and source_pin is not None:
                            try:
                                pin_idx = int(source_pin)
                                if pin_idx < len(real_status.get("stat", [])):
                                    current_val = float(real_status["stat"][pin_idx])
                                    target_val = float(cond_value)
                                    
                                    if cond_type == "equal" and current_val == target_val:
                                        condition_met = True
                                    elif cond_type == "greater than" and current_val > target_val:
                                        condition_met = True
                                    elif cond_type == "less than" and current_val < target_val:
                                        condition_met = True
                            except (ValueError, IndexError):
                                pass
                        
                        # Проверка условия таймера
                        elif cond_type == "timer":
                            pass
                        
                        # Выполняем действие если условие выполнено
                        if condition_met:
                            action_type = cond.get("actionType")
                            action_pin = cond.get("actionPin")
                            action_value = cond.get("actionValue")
                            
                            if action_type == "switch pin" and action_pin:
                                pin_idx = int(action_pin)
                                if pin_idx < len(desired_status["stat"]):
                                    if action_value == "PWM":
                                        desired_status["stat"][pin_idx] = str(cond.get("pwmValue", "0"))
                                    else:
                                        desired_status["stat"][pin_idx] = "1" if action_value == "1" else "0"
                                    
                                    # Обновляем desired_status в БД
                                    cur.execute("UPDATE devices SET desired_status=%s, has_updates=TRUE WHERE device_id=%s",
                                               (json.dumps(desired_status), device_id))
                                    conn.commit()
        finally:
            conn.close()

@app.on_event("startup")
async def startup_event():
    asyncio.create_task(check_conditions())

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=5001)