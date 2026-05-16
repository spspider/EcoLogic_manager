# EcoLogic Manager — GitHub Copilot Instructions

Read `COPILOT_INSTRUCTIONS.md` in the workspace root for the full project documentation.

## Quick Reference

### Folder roles
| Folder | Role | Edit? |
|--------|------|-------|
| `EcoLogic_manager/*.ino` | ESP8266 firmware (Arduino C++) | ✅ Yes |
| `HTML_data/` | Shared frontend (HTML/JS/CSS) | ✅ Yes |
| `EcoLogic_manager/data/` | **Generated** — gzipped HTMLs | ❌ No — run `make_gz.sh` |
| `EcoLogic_manager/build/` | **Generated** — compiled firmware | ❌ No |
| `standalone_server/` | FastAPI backend (Python) | ✅ Yes |
| `docker-build/` | Firmware Docker compiler | ✅ Yes |

### Critical rules
- Always edit `HTML_data/` — never `EcoLogic_manager/data/`
- `IS_SERVER = window.location.pathname.startsWith('/api/')` is the dual-mode detection pattern
- All new FastAPI routes must use `/api/` prefix (Nginx proxy requirement)
- `device_id` = ESP8266 MAC address without colons
- Feature flags in `EcoLogic_manager.ino` are compile-time `#define` switches

### Key files
- Firmware entry: `EcoLogic_manager/EcoLogic_manager.ino`
- Backend entry: `standalone_server/app_fastapi.py` (port 5005)
- Dual-mode detection: `HTML_data/scripts/helper_func.js`
- Secrets template: `standalone_server/.env.example`

### Production
- Server: `ssh recoder@192.168.1.160`
- App path: `/home/recoder/RECODER-COMPOSE/device-manager/`
- Public URL: `https://ecologic.go.ro`

### Excluded folders (not relevant to code)
- `proteous/` — PCB schematics
- `pictures/` — images only
- `HTML_data/backup/` — old backups
- `drawio-scheme/` — diagrams
- `standalone_server/debug/` — debug logs
