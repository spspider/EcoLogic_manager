# EcoLogic Manager — AI Agent Documentation

> **For:** GitHub Copilot, Roo, and any AI coding assistant.  
> **Purpose:** Understand the project topology, folder interconnections, and data-flow before suggesting code changes.

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Folder Map & What to Ignore](#2-folder-map--what-to-ignore)
3. [Component Deep-Dives](#3-component-deep-dives)
   - 3.1 [Firmware — ESP8266 Arduino sketch](#31-firmware--esp8266-arduino-sketch)
   - 3.2 [HTML_data — Shared Frontend](#32-html_data--shared-frontend)
   - 3.3 [make_gz — Build pipeline](#33-make_gz--build-pipeline)
   - 3.4 [standalone_server — FastAPI backend](#34-standalone_server--fastapi-backend)
   - 3.5 [docker-build — Firmware compiler](#35-docker-build--firmware-compiler)
4. [Critical Interconnections](#4-critical-interconnections)
5. [Production Infrastructure](#5-production-infrastructure)
6. [Data Flow Diagrams](#6-data-flow-diagrams)
7. [API Contract: Firmware ↔ Server](#7-api-contract-firmware--server)
8. [Frontend Dual-Mode Pattern](#8-frontend-dual-mode-pattern)
9. [Environment & Secrets](#9-environment--secrets)
10. [Typical Developer Workflows](#10-typical-developer-workflows)

---

## 1. Project Overview

EcoLogic Manager is an **IoT home automation system** built around the ESP8266 microcontroller.  
It has **three tiers**:

| Tier | Location | Technology |
|------|----------|-----------|
| **Firmware** (bare-metal) | `EcoLogic_manager/` | Arduino C++ (.ino) |
| **Frontend** (shared HTML/JS) | `HTML_data/` | Vanilla JS, served both from ESP8266 and FastAPI |
| **Backend API server** | `standalone_server/` | Python FastAPI, MySQL, InfluxDB, Grafana |

---

## 2. Folder Map & What to Ignore

```
EcoLogic_manager/           ← workspace root
│
├── EcoLogic_manager/       ← FIRMWARE (Arduino sketch)
│   ├── *.ino               ← C++ source files
│   ├── data/               ← ⚡ GENERATED: gzipped HTMLs uploaded to ESP8266 filesystem
│   └── build/              ← ⚡ GENERATED: compiled .bin (gitignored)
│
├── HTML_data/              ← SHARED FRONTEND (source of truth for all HTML/JS/CSS)
│   ├── *.htm / *.html      ← UI pages
│   ├── scripts/            ← JavaScript files
│   └── img/                ← images
│
├── standalone_server/      ← BACKEND API SERVER
│   ├── app_fastapi.py      ← Main FastAPI application
│   ├── user_routes.py      ← User management endpoints
│   ├── influx_logger.py    ← InfluxDB time-series logging
│   ├── grafana_user_manager.py ← Auto-provision Grafana users
│   ├── query_influx.py     ← InfluxDB query helpers
│   ├── src/                ← Server-only HTML (e.g. device_selector.html)
│   ├── Dockerfile          ← Production Docker image
│   ├── docker-compose.yaml ← Dev environment (MySQL + phpMyAdmin)
│   ├── requirements.txt    ← Python deps
│   ├── .env.example        ← Template for secrets
│   └── .env                ← ⛔ GITIGNORED — actual secrets
│
├── docker-build/           ← FIRMWARE BUILD TOOLCHAIN
│   ├── Dockerfile          ← Image: ubuntu + arduino-cli + esp8266 core 3.1.2
│   └── docker-compile-build.sh ← Run to compile .ino → .bin inside Docker
│
├── make_gz.sh              ← Bash: compress HTML_data/ → EcoLogic_manager/data/
├── make_gz.ps1             ← PowerShell: same as above (Windows)
├── git_push.bat            ← Quick git pull+add+commit+push
│
│  ── EXCLUDED from AI context ──
├── proteous/               ← PCB design files (not relevant to code)
├── pictures/               ← Images only
├── HTML_data/backup/       ← Old file backups
├── drawio-scheme/          ← Architecture diagrams
└── standalone_server/debug/← Debug logs
```

> **.gitignore also excludes:** `*.gz`, `*.bin`, `*.env`, `build/`, `__pycache__/`, `node_modules/`, `*.mp4`, `*.zip`, `*.rar`

---

## 3. Component Deep-Dives

### 3.1 Firmware — ESP8266 Arduino sketch

**Location:** `EcoLogic_manager/` (the inner folder, not the root)

**Entry point:** `EcoLogic_manager.ino` — contains `#define` feature flags and `#include` directives.

**Key feature flags (in `EcoLogic_manager.ino`):**

| Flag | Purpose |
|------|---------|
| `USE_LITTLEFS` | Use LittleFS (default). Mutually exclusive with `USE_SPIFFS` |
| `USE_DHT` | DHT temperature/humidity sensor on pin D4 |
| `USE_EMON` | Energy monitor (conflicts with `USE_IRUTILS`) |
| `USE_IRUTILS` | IR send/receive (conflicts with `USE_EMON`) |
| `USE_PUBSUBCLIENT` | MQTT client |
| `USE_DNS_SERVER` | Captive portal DNS |
| `ws2811_include` | WS2811/WS2812 LED strip |

**Source file responsibilities:**

| File | Role |
|------|------|
| `EcoLogic_manager.ino` | Feature flags, global includes, global variables |
| `a_CaptivePortalAdvanced.ino` | WiFi AP/STA mode, captive portal |
| `a_EEPROM_file.ino` | EEPROM read/write helpers |
| `a_pubClient.ino` | MQTT (PubSubClient) integration |
| `a_tIoiFSBrowser.ino` | LittleFS file browser HTTP handler |
| `arduino_client.ino` | **Syncs with standalone server** — uploads config, polls for commands |
| `b_LoadSettings.ino` | Parses JSON config from LittleFS (`pin_setup.txt`, `other_setup.txt`) |
| `d_SSDP.ino` | SSDP device discovery |
| `e_Time_alarm_string.ino` | Timer / alarm parsing |
| `f_WOL.ino` | Wake-on-LAN |
| `h_Webscoket_iot_json.ino` | WebSocket JSON API + AJAX status responses |
| `handleHttp.ino` | HTTP routes for the embedded web server |
| `i_compass.ino` | AS5600 compass sensor |
| `j_essential_function.ino` | Core helpers: `generate_device_id()`, `get_new_widjet_value()`, etc. |
| `picoMqtt.ino` / `a_pubClient.ino` | MQTT alternatives |
| `player.ino` / `player_mp3.ino` / `player_from_url.ino` | Audio playback |
| `sendEmail.ino` | Email notifications |
| `telegram.ino` | Telegram bot |
| `w_IR_recive.ino` | IR receiver |
| `w_position.ino` | Geolocation / GPS |
| `w433rcv.ino` | 433 MHz RF receiver |
| `ws2811.ino` | LED strip control |

**Key data structures (global arrays, N_WIDGETS = 12 by default):**

```cpp
uint8_t pin[N_WIDGETS];       // GPIO pin numbers
uint8_t pinmode[N_WIDGETS];   // 1=in, 2=out, 3=pwm, 4=adc, 5=dhtTemp, 6=dhtHum, ...
float   stat[N_WIDGETS];      // current sensor/output values
uint8_t defaultVal[N_WIDGETS];// default/inverted values
char    descr[N_WIDGETS][32]; // human-readable pin descriptions
```

**Device ID:** Auto-generated from MAC address (`generate_device_id()` in `j_essential_function.ino`).

**Config files on LittleFS:**

| File | Format | Purpose |
|------|--------|---------|
| `/pin_setup.txt` | JSON | Pin modes, descriptions, states |
| `/other_setup.txt` | JSON | WiFi SSID/pass, MQTT, server URL, user_name |
| `/activation1.txt` | text | Activation/schedule data |
| `/Condition0.txt` | JSON | Automation conditions |

---

### 3.2 HTML_data — Shared Frontend

**Location:** `HTML_data/`

This is the **single source of truth** for all UI files. The same files are used in **two contexts**:

1. **ESP8266 mode** — served directly from LittleFS at `http://<device-ip>/home.htm`
2. **Server mode** — served via FastAPI at `https://ecologic.go.ro/api/home?device_id=<id>`

**Key pages:**

| File | Purpose |
|------|---------|
| `home.htm` | Main control dashboard |
| `pin_setup.htm` | Configure GPIO pins |
| `other_setup.htm` | WiFi, MQTT, server URL settings |
| `condition.htm` | Automation rules editor |
| `wifi_setup.htm` | WiFi scan & connect |
| `IR_setup.htm` | IR remote setup |
| `graphs.htm` | Real-time sensor graphs |
| `homeassistant.htm` | Home Assistant integration |
| `ws2811.html` | LED strip control |
| `edit.htm` | File manager / editor |
| `help.htm` | Help page |

**Key scripts (`scripts/`):**

| Script | Purpose |
|--------|---------|
| `helper_func.js` | Shared utilities, `IS_SERVER` detection, navigation |
| `script_home.js` | Home page logic |
| `pin_setup.js` | Pin configuration UI |
| `condition.js` | Automation rules UI |
| `graphs.js` | Chart.js graph rendering |
| `wifi_setup.js` | WiFi UI |
| `homeassistant.js` | Home Assistant bridge |
| `ws2811.js` / `ws2812_set.js` | LED strip UI |
| `other_setup.js` | Settings page |
| `chart.min.js` | Chart.js library |
| `ace.min.js` | Ace code editor |

---

### 3.3 make_gz — Build pipeline

**Files:** `make_gz.sh` (Linux/WSL/Git Bash), `make_gz.ps1` (Windows PowerShell)

**What it does:**  
Reads files listed in `$fileList` from `HTML_data/` and compresses each one with `gzip -9` into `EcoLogic_manager/data/<name>.gz`.

**Flow:**
```
HTML_data/home.htm  →  gzip -9  →  EcoLogic_manager/data/home.htm.gz
HTML_data/scripts/script_home.js  →  EcoLogic_manager/data/scripts/script_home.js.gz
```

**When to run:** After editing any file in `HTML_data/`, run make_gz before uploading to the device.

**Upload to device:** Use Arduino IDE "ESP8266 LittleFS Data Upload" tool, or OTA.

> ⚠️ The `EcoLogic_manager/data/` folder is NOT the source — always edit `HTML_data/` instead.

---

### 3.4 standalone_server — FastAPI backend

**Location:** `standalone_server/`

**Entry point:** `app_fastapi.py`  
**Runs on port:** `5005`  
**Start command:** `python app_fastapi.py` (or `./start_app.sh`)

**What it does:**
- Receives pin configuration and status from all ESP8266 devices
- Stores device data in MySQL (`ecologic` database)
- Serves the same HTML_data frontend at `/api/*` routes
- Logs sensor data to InfluxDB (time-series)
- Auto-provisions Grafana users and dashboards
- User authentication (MySQL-backed + HTTP Basic fallback)

**Key endpoints:**

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/api/cfg` | Arduino uploads pin config + IP address |
| `POST` | `/api/other` | Arduino uploads other_setup.txt |
| `POST` | `/api/sync` | Arduino real-status → server, returns desired-status |
| `GET` | `/api/devices` | List all registered devices |
| `GET` | `/api/device_selector` | HTML device selector page |
| `GET` | `/api/home` | Home control page (proxied from HTML_data) |
| `GET` | `/api/pin_setup` | Pin setup page |
| `POST` | `/api/upload_pin_setup` | Upload pin config (legacy) |
| `/api/static/*` | | Static JS/CSS served from `HTML_data/scripts/` |

**Module breakdown:**

| File | Responsibility |
|------|----------------|
| `app_fastapi.py` | Main app, all Arduino-facing and UI endpoints, DB connection |
| `user_routes.py` | User CRUD endpoints, admin management |
| `influx_logger.py` | `InfluxLogger` class, logs per-pin numeric values |
| `grafana_user_manager.py` | `GrafanaUserManager`: create/delete Grafana users, orgs, datasources |
| `query_influx.py` | InfluxDB query helpers for dashboard data |
| `src/device_selector.html` | Server-only HTML (not shared with ESP8266) |

**MySQL database:** `ecologic`

| Table | Key columns |
|-------|------------|
| `devices` | `device_id`, `token`, `pin_setup` (JSON), `ip_address`, `owner`, `last_seen` |
| `device_settings` | `device_id`, `other_setup` (JSON), `condition` (JSON) |
| `users` | `username`, `password`, `email` |

**Authentication:** HTTP Basic Auth. Credentials checked against `users` table first, then falls back to `BASIC_USER`/`BASIC_PASS` env vars.

---

### 3.5 docker-build — Firmware compiler

**Location:** `docker-build/`

**Docker image:** `spspider/esp8266-clean:3.1.2` (published to Docker Hub)

**Image contents:** Ubuntu 20.04 + arduino-cli + ESP8266 core 3.1.2

**To build firmware:**
```bash
cd docker-build
./docker-compile-build.sh
```

This mounts:
- `../` as `/workspace` (the whole project)
- `project_libraries/` as `/opt/project_libs` (Arduino libraries)

Output: `EcoLogic_manager/build/EcoLogic_manager.ino.bin`

**To rebuild the Docker image:**
```bash
cd docker-build
docker build -t spspider/esp8266-clean:3.1.2 .
docker push spspider/esp8266-clean:3.1.2
```

---

## 4. Critical Interconnections

```
HTML_data/  ──(make_gz.sh)──►  EcoLogic_manager/data/  ──(LittleFS upload)──►  ESP8266
     │
     └──(Docker COPY)──►  standalone_server/  ──►  FastAPI serves /api/static/*
                                                    FastAPI serves /api/home, etc.
```

**Rule:** `HTML_data/` is edited once and deployed to **both** the device and the server.

**The `IS_SERVER` detection pattern** in every JavaScript file:
```javascript
const IS_SERVER = window.location.pathname.startsWith('/api/');
const API_PREFIX = IS_SERVER ? '/api' : '';
// All fetch() calls use API_PREFIX:
fetch(`${API_PREFIX}/pin_setup.txt`)
fetch(`${API_PREFIX}/function?data=...`)
```
When the page is accessed via the FastAPI server (`/api/...`), it adds the `/api` prefix to all XHR requests, passing `device_id` as a query param. When accessed directly on the ESP8266, no prefix is needed.

---

## 5. Production Infrastructure

**Server:** `192.168.1.160` (static LAN IP)  
**SSH access:** `ssh recoder@192.168.1.160`

**All services live under:** `/home/recoder/RECODER-COMPOSE/`

```
/home/recoder/RECODER-COMPOSE/
│
├── device-manager/
│   ├── docker-compose.yaml     ← EcoLogic FastAPI app (port 5005)
│   └── start.sh
│
├── nginx-reverse-proxy/
│   ├── docker-compose.yaml     ← Nginx (ports 80/443)
│   └── nginx.conf              ← Routes: /api/* → device-manager:5005
│                                          /graf/* → grafana:3000
│
├── InfluxDB_Grafana/
│   └── docker-compose.yaml     ← InfluxDB (port 8086) + Grafana (port 3000)
│
└── worpress/                   ← Contains the shared MySQL instance
    └── docker-compose.yaml     ← mysql:5.7 container: wordpress_db
                                   DB: ecologic, User: ecouser
                                   Volumes: db_data, init-db.sql
```

**Public domain:** `https://ecologic.go.ro`  
**Grafana public URL:** `https://ecologic.go.ro/graf`

**Production deployment workflow:**
1. SSH to `recoder@192.168.1.160`
2. Navigate to `/home/recoder/RECODER-COMPOSE/device-manager/`
3. Pull latest image: `docker-compose pull`
4. Restart: `docker-compose up -d`

---

## 6. Data Flow Diagrams

### Firmware → Server sync

```
ESP8266 boot
    │
    ├── uploadConfig_ecologicclient()
    │       POST /api/cfg?id=<mac>&tk=tk01&ip=<ip>   (pin_setup.txt as JSON body)
    │       POST /api/other?id=<mac>&tk=tk01          (other_setup.txt as JSON body)
    │
    └── loop_ecologicclient()  (every sync_interval seconds)
            POST /api/sync?id=<mac>&tk=tk01
                Body: {"stat": ["1.00", "0.00", "25.50", ...]}
                Response: {"stat": ["1", "0", "25", ...], "upd": 1}
                    └── if upd==1: apply desired state to pins
```

### User opens web UI (server mode)

```
Browser → https://ecologic.go.ro/api/device_selector
    │           (Nginx proxies to FastAPI:5005)
    │
    ├── GET /api/devices  →  FastAPI reads MySQL devices table
    │
    └── Click device → /api/home?device_id=<id>
            └── FastAPI serves home.htm (from HTML_data/)
                    └── JS detects IS_SERVER=true
                        All API calls prefixed with /api + device_id param
                        e.g. GET /api/pin_setup.txt?device_id=<id>
                             POST /api/function?data={...}&device_id=<id>
                             POST /api/sync?id=<id>
```

### HTML editing → Device update

```
1. Edit file in HTML_data/
2. Run: ./make_gz.sh   (or .\make_gz.ps1)
3. Open Arduino IDE
4. Tools → ESP8266 LittleFS Data Upload
5. Device reboots with new files
```

---

## 7. API Contract: Firmware ↔ Server

All requests from firmware use **short parameter names** to save ESP8266 memory:

| Param | Meaning |
|-------|---------|
| `id` | device_id (MAC address, no colons) |
| `tk` | token (currently hardcoded `"tk01"`) |
| `ip` | device LAN IP address |

**Sync payload (firmware → server):**
```json
{ "stat": ["1.00", "0.00", "25.50", "60.20", "0.00", ...] }
```
Array index corresponds to widget/pin index (0 to N_WIDGETS-1).

**Sync response (server → firmware):**
```json
{ "stat": ["1", "0", "25", "60", "0", ...], "upd": 1 }
```
`"upd": 1` means the desired state differs — firmware will apply changes.

---

## 8. Frontend Dual-Mode Pattern

Every JavaScript file that makes API calls follows this pattern:

```javascript
// Detect context
const IS_SERVER = window.location.pathname.startsWith('/api/');
const API_PREFIX = IS_SERVER ? '/api' : '';

// Use API_PREFIX for all requests
fetch(`${API_PREFIX}/pin_setup.txt${IS_SERVER ? '?device_id=' + deviceId : ''}`)
```

**Navigation links also change:**
- ESP8266 mode: `/home.htm`, `/pin_setup`, `/wifi`
- Server mode: `/api/home?device_id=<id>`, `/api/pin_setup?device_id=<id>`

The `helper_func.js` also checks `window.IS_SERVER` for cases where the server sets it explicitly via template injection.

---

## 9. Environment & Secrets

Template: `standalone_server/.env.example`  
Actual file: `standalone_server/.env` (**gitignored**)

| Variable | Default | Purpose |
|----------|---------|---------|
| `DEVICE_MANAGER_DB_HOST` | `192.168.1.160` | MySQL host |
| `DEVICE_MANAGER_DB_PORT` | `3306` | MySQL port |
| `DEVICE_MANAGER_DB_NAME` | `ecologic` | Database name |
| `DEVICE_MANAGER_DB_USER` | `ecouser` | DB user |
| `DEVICE_MANAGER_DB_PASSWORD` | `ecopass` | DB password |
| `BASIC_USER` | `admin` | HTTP Basic fallback user |
| `BASIC_PASS` | `admin123` | HTTP Basic fallback password |
| `INFLUXDB_TOKEN` | *(required)* | InfluxDB API token |
| `INFLUXDB_ORG` | `ecologic` | InfluxDB org |
| `INFLUXDB_BUCKET` | `default` | InfluxDB bucket |
| `INFLUXDB_HOST` | `192.168.1.160` | InfluxDB host |
| `INFLUXDB_PORT` | `8086` | InfluxDB port |
| `GRAFANA_HOST` | `192.168.1.160` | Grafana host |
| `GRAFANA_PORT` | `3000` | Grafana port |
| `GRAFANA_ADMIN_USER` | `admin` | Grafana admin user |
| `GRAFANA_ADMIN_PASSWORD` | `admin` | Grafana admin password |

---

## 10. Typical Developer Workflows

### Edit UI and test on device
```bash
# 1. Edit a file in HTML_data/
# 2. Compress and copy to firmware data folder
./make_gz.sh
# 3. Upload filesystem via Arduino IDE: Tools > ESP8266 LittleFS Data Upload
```

### Edit UI and test on server (local dev)
```bash
cd standalone_server
# No compression needed — FastAPI mounts HTML_data/ directly
python app_fastapi.py
# Visit http://localhost:5005/api/device_selector
```

### Compile firmware
```bash
cd docker-build
./docker-compile-build.sh
# Output: EcoLogic_manager/build/EcoLogic_manager.ino.bin
# Flash via OTA or USB
```

### Deploy to production
```bash
# Build and push Docker image
docker build -t spspider/ecologic-manager:latest -f standalone_server/Dockerfile .
docker push spspider/ecologic-manager:latest

# On production server:
ssh recoder@192.168.1.160
cd /home/recoder/RECODER-COMPOSE/device-manager
docker-compose pull && docker-compose up -d
```

### Quick git push
```batch
git_push.bat
```

---

## Notes for AI Agents

- **Never edit `EcoLogic_manager/data/`** — it is generated. Always edit `HTML_data/` instead.
- **`EcoLogic_manager/build/`** is gitignored compiled output — do not edit.
- The `device_id` is the ESP8266 **MAC address without colons** (e.g., `A1B2C3D4E5F6`).
- `N_WIDGETS` (default 12) controls array sizes — changing it requires recompile and re-upload.
- Feature flags in `EcoLogic_manager.ino` are `#define` — they are compile-time switches.
- When adding new API endpoints to `app_fastapi.py`, they should use the `/api/` prefix to work through Nginx.
- `standalone_server/src/` contains server-only HTML pages (not shared with ESP8266).
- MySQL container name on prod is `wordpress_db` (shared with WordPress infra).
- InfluxDB token must be obtained from `http://192.168.1.160:8086` → Load Data → API Tokens.
