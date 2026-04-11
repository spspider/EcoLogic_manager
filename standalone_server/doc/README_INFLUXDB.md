# InfluxDB Integration for EcoLogic Manager

This integration adds automatic data logging to InfluxDB for all device sensor data, enabling powerful Grafana dashboards with user-based filtering.

## Features

- **User-based data separation**: All data is tagged with the device owner's username
- **Individual pin logging**: Each device pin is logged separately with descriptions
- **Device summary data**: Online status, IP addresses, and heartbeat data
- **Grafana-ready**: Data structure optimized for Grafana queries and dashboards

## Quick Setup

### 1. Install Dependencies

```bash
# Navigate to the standalone_server directory
cd standalone_server

# Install the new dependency
pip install influxdb-client==1.38.0

# Or install all dependencies
pip install -r requirements.txt
```

### 2. Configure Environment Variables

1. Copy the example environment file:
   ```bash
   cp .env.example .env
   ```

2. Get your InfluxDB token:
   - Go to http://192.168.1.160:8086
   - Login with `admin`/`adminpass`
   - Navigate to **Load Data → API Tokens**
   - Click **Generate API Token** → **All Access API Token**
   - Copy the generated token

3. Edit `.env` file and replace `your_influxdb_token_here` with your actual token

### 3. Test the Integration

```bash
python test_influx.py
```

This will:
- Verify your InfluxDB connection
- Test data logging
- Send sample data to InfluxDB
- Provide Grafana setup instructions

### 4. Restart Your FastAPI Server

```bash
python app_fastapi.py
```

## Data Structure

The integration creates two measurements in InfluxDB:

### `device_pins` measurement
- **Tags**: `user`, `device_id`, `pin_index`, `pin_name`
- **Fields**: `value` (numeric), `raw_value` (original string)
- **Use**: Individual sensor/actuator data

### `device_summary` measurement
- **Tags**: `user`, `device_id`, `ip_address`  
- **Fields**: `pin_count`, `online`
- **Use**: Device status and connectivity

## Grafana Dashboard Setup

### 1. Add InfluxDB Data Source

1. Go to http://192.168.1.160:3000
2. Login to Grafana
3. Go to **Configuration → Data Sources**
4. Click **Add data source** → **InfluxDB**
5. Configure:
   - **URL**: `http://influxdb:8086`
   - **Organization**: `ecologic` 
   - **Token**: Your InfluxDB token
   - **Default Bucket**: `default`
6. Click **Save & Test**

### 2. Create User-Specific Dashboards

Use these sample queries as starting points:

#### All devices for a specific user:
```flux
from(bucket: "default")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "device_pins")
  |> filter(fn: (r) => r.user == "your_username")
  |> filter(fn: (r) => r._field == "value")
```

#### Specific device data:
```flux
from(bucket: "default")
  |> range(start: -1h)  
  |> filter(fn: (r) => r._measurement == "device_pins")
  |> filter(fn: (r) => r.user == "your_username")
  |> filter(fn: (r) => r.device_id == "your_device_id")
  |> filter(fn: (r) => r._field == "value")
```

#### Device online status:
```flux
from(bucket: "default")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "device_summary")  
  |> filter(fn: (r) => r.user == "your_username")
  |> filter(fn: (r) => r._field == "online")
```

### 3. Dashboard Variables

Create dashboard variables for dynamic filtering:

1. **User variable**:
   ```flux
   from(bucket: "default")
     |> range(start: -24h)
     |> filter(fn: (r) => r._measurement == "device_pins")
     |> distinct(column: "user")
   ```

2. **Device variable**:
   ```flux
   from(bucket: "default")
     |> range(start: -24h)
     |> filter(fn: (r) => r._measurement == "device_pins")  
     |> filter(fn: (r) => r.user == "$user")
     |> distinct(column: "device_id")
   ```

## Troubleshooting

### Connection Issues
- Ensure InfluxDB container is running: `docker ps | grep influxdb`
- Check InfluxDB logs: `docker logs influxdb`
- Verify token permissions in InfluxDB UI

### No Data in Grafana
- Run `python test_influx.py` to send test data
- Check InfluxDB Data Explorer for incoming data
- Verify device is sending data to `/api/sync` endpoint

### Performance
- Data is logged every time a device syncs (typically every 10-30 seconds)
- No duplicate prevention - each sync creates new data points
- Consider retention policies for large datasets

## Sample Data

After running the test script, you should see data like:

```
device_pins:
- user="test_user", device_id="test_device", pin_index="0", pin_name="relay_1" → value=1.0
- user="test_user", device_id="test_device", pin_index="1", pin_name="dimmer_1" → value=0.5
- user="test_user", device_id="test_device", pin_index="2", pin_name="temperature" → value=25.3

device_summary:  
- user="test_user", device_id="test_device", ip_address="192.168.1.100" → online=1, pin_count=5
```

This structure allows you to create powerful, user-specific dashboards in Grafana!