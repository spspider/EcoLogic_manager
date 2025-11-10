# Unified Scripts Documentation

## Overview
The scripts are now unified and work on both ESP8266 device and standalone server with automatic environment detection.

## Files Structure

### Unified Files (Identical for both environments):
1. **script_home.js** - Main control interface
   - Location ESP8266: `HTML_data/scripts/script_home.js`
   - Location Server: `standalone_server/src/scripts/script_home.js`

2. **condition.js** - Automation conditions
   - Location ESP8266: `HTML_data/scripts/condition.js`
   - Location Server: `standalone_server/src/scripts/condition.js`

## Auto-Detection

### script_home.js
```javascript
const IS_SERVER = window.DEVICE_ID !== undefined;
const API_PREFIX = IS_SERVER ? '/api' : '';
```

- **ESP8266**: `window.DEVICE_ID` is undefined → uses direct paths
- **Server**: `window.DEVICE_ID` is set → uses `/api` prefix

### condition.js
```javascript
const deviceId = window.DEVICE_ID || 'default';
```

- **ESP8266**: Reads from local files (`Condition0.txt`, `pin_setup.txt`)
- **Server**: Fetches from API (`/api/conditions`, `/api/pin_setup`)

## Key Features

### 1. localStorage Support
Both scripts now use localStorage for temporary data:
```javascript
function saveDataToLocalStorage() {
    localStorage.setItem('tableData', JSON.stringify(tableData));
}
```

### 2. DOM Elements
All UI elements created using DOM API:
```javascript
const element = document.createElement('input');
element.className = 'btn btn-block btn-success';
element.onclick = () => sendNewValue(element, id);
```

### 3. English Language
All buttons and labels in English:
- "Add condition" instead of "добавить условие"
- "Save" instead of "сохранить"
- "Delete" instead of "удалить"

### 4. Numeric Format (ESP8266)
Data stored as numbers to save memory:
```json
{
  "En": [1, 0],           // enabled: 1/0
  "type": [1, 2],         // index of CONDITION_TYPES
  "type_value": [360, 25], // minutes or value
  "act": [1, 1],          // index of ACTION_TYPES
  "actOn": ["2 1", "5 2 512"] // "pin value" or "pin 2 pwm"
}
```

## Maintenance

### Updating Both Versions
Since files are identical, you can:

1. **Manual sync**:
```bash
copy HTML_data\scripts\script_home.js standalone_server\src\scripts\script_home.js
copy HTML_data\scripts\condition.js standalone_server\src\scripts\condition.js
```

2. **Or edit one and copy**:
   - Edit ESP8266 version
   - Copy to server version
   - Or vice versa

### Testing
- **ESP8266**: Upload to device and test locally
- **Server**: Run FastAPI server and test with `?device_id=test`

## API Differences

### ESP8266 Endpoints:
- `/sendAJAX?data={...}` - Send/receive widget data
- `/function?data={...}` - Device functions
- `pin_setup.txt` - Pin configuration file
- `Condition0.txt` - Condition files

### Server Endpoints:
- `/api/ajax?data={...}&device_id={id}` - Send/receive widget data
- `/api/pin_setup?device_id={id}` - Get pin configuration
- `/api/conditions?device_id={id}` - Get/save conditions

## Benefits

1. **Single codebase** - Easier to maintain
2. **Auto-detection** - No manual configuration needed
3. **Modern code** - Uses DOM, localStorage, async/await
4. **Memory efficient** - Numeric format for ESP8266
5. **Bilingual ready** - Easy to add translations

## Migration Notes

### From Old Version:
- Old global arrays replaced with `widgetState` object
- Russian text replaced with English
- innerHTML replaced with DOM createElement
- Callback hell replaced with async/await where possible

### Data Format:
- **Old**: Mixed strings and numbers
- **New**: Consistent numeric indices with conversion functions
