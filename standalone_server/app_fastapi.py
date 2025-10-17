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

app.mount("/api/static", StaticFiles(directory="src/scripts"), name="static")



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
    if credentials.username != BASIC_USER or credentials.password != BASIC_PASS:
        raise HTTPException(status_code=401, detail="Unauthorized")
    return credentials.username

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
    
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                sql = """
                INSERT INTO devices (device_id, token, pin_setup, ip_address, last_seen)
                VALUES (%s, %s, %s, %s, %s)
                ON DUPLICATE KEY UPDATE token = VALUES(token), pin_setup = VALUES(pin_setup), ip_address = VALUES(ip_address), last_seen = VALUES(last_seen)
                """
                cur.execute(sql, (device_id, token, json.dumps(pin_setup), ip_address, datetime.utcnow()))
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

@app.get("/api/config")
async def get_config(id: Optional[str] = "default"):
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                cur.execute("SELECT pin_setup, ip_address FROM devices WHERE device_id=%s", (id,))
                row = cur.fetchone()
                if row and row.get("pin_setup"):
                    config = json.loads(row["pin_setup"])
                    config["ip_address"] = row.get("ip_address", "unknown")
                    return config
        finally:
            conn.close()
    
    return {}

@app.get("/api/ajax")
async def send_ajax(request: Request):
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
async def get_devices():
    """
    Получение списка всех устройств в системе
    """
    conn = get_conn()
    if conn:
        try:
            with conn.cursor() as cur:
                cur.execute("""
                    SELECT device_id, last_seen, ip_address,
                           JSON_LENGTH(pin_setup, '$.descr') as pin_count
                    FROM devices 
                    ORDER BY last_seen DESC
                """)
                devices = cur.fetchall()
                return devices
        finally:
            conn.close()
    return []

@app.get("/api/device_selector", response_class=HTMLResponse)
async def device_selector(user: str = Depends(verify_basic)):
    with open("src/device_selector.html", "r", encoding="utf-8") as f:
        html = f.read()
    return HTMLResponse(html)

@app.get("/api/home", response_class=HTMLResponse)
async def index(user: str = Depends(verify_basic), device_id: Optional[str] = None):
    if not device_id:
        # Перенаправляем на страницу выбора устройства
        return HTMLResponse('<script>window.location.href="/api/device_selector"</script>')
    
    with open("src/home.htm", "r", encoding="utf-8") as f:
        html = f.read()
    
    # Исправляем пути к статическим файлам
    html = html.replace('src="scripts/', 'src="/api/static/')
    html = html.replace('href="scripts/', 'href="/api/static/')
    
    # Добавляем device_id в JavaScript
    html = html.replace('</head>', f'<script>window.DEVICE_ID = "{device_id}";</script></head>')
    return HTMLResponse(html)

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=5001)