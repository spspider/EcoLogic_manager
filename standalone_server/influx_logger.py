"""
InfluxDB Logger for EcoLogic Device Data
Logs device pin states with user tagging for Grafana dashboard filtering
"""

import os
import json
from datetime import datetime
from typing import List, Dict, Optional
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS
import logging

logger = logging.getLogger(__name__)

class InfluxLogger:
    def __init__(self):
        self.token = os.getenv("INFLUXDB_TOKEN")
        self.org = os.getenv("INFLUXDB_ORG", "ecologic")
        self.bucket = os.getenv("INFLUXDB_BUCKET", "default")
        # Try INFLUXDB_HOST first, then DEVICE_MANAGER_DB_HOST, then default
        self.host = os.getenv("INFLUXDB_HOST", os.getenv("DEVICE_MANAGER_DB_HOST", "192.168.1.160"))
        self.port = int(os.getenv("INFLUXDB_PORT", "8086"))
        self.url = f"http://{self.host}:{self.port}"
        
        self.client = None
        self.write_api = None
        
        if self.token:
            try:
                self.client = InfluxDBClient(url=self.url, token=self.token, org=self.org)
                self.write_api = self.client.write_api(write_options=SYNCHRONOUS)
                logger.info(f"InfluxDB client initialized: {self.url} (org: {self.org}, bucket: {self.bucket})")
            except Exception as e:
                logger.error(f"Failed to initialize InfluxDB client: {e}")
                self.client = None
        else:
            logger.warning("INFLUXDB_TOKEN not found in environment variables")

    def is_available(self) -> bool:
        """Check if InfluxDB connection is available"""
        return self.client is not None and self.write_api is not None

    def log_device_status(self, device_id: str, owner: str, status_data: Dict, 
                         pin_setup: Optional[Dict] = None):
        """
        Log device pin status data to InfluxDB
        
        Args:
            device_id: Unique device identifier
            owner: Username who owns the device
            status_data: Status data containing 'stat' array
            pin_setup: Optional pin configuration for pin descriptions
        """
        if not self.is_available():
            logger.warning(f"InfluxDB not available for device {device_id}")
            return False

        try:
            stat_array = status_data.get("stat", [])
            timestamp = datetime.utcnow()
            
            # Debug logging: show received data
            # logger.info(f"📊 Logging device data: {device_id} (user: {owner}) - {len(stat_array)} pins: {stat_array}")
            
            # Get pin descriptions if available
            pin_descriptions = []
            if pin_setup and isinstance(pin_setup, dict):
                pin_descriptions = pin_setup.get("descr", [])
            
            points = []
            
            # Create a point for each pin
            for pin_index, value in enumerate(stat_array):
                try:
                    # Convert value to float for numeric storage
                    numeric_value = float(value)
                    
                    # Get pin description or use generic name
                    pin_name = f"pin_{pin_index}"
                    if pin_index < len(pin_descriptions) and pin_descriptions[pin_index]:
                        pin_name = pin_descriptions[pin_index].replace(" ", "_").lower()
                    
                    # Create InfluxDB point
                    point = (Point("device_pins")
                            .tag("user", owner)
                            .tag("device_id", device_id)
                            .tag("pin_index", str(pin_index))
                            .tag("pin_name", pin_name)
                            .field("value", numeric_value)
                            .field("raw_value", value)  # Keep original string value too
                            .time(timestamp, WritePrecision.S))
                    
                    points.append(point)
                    
                except (ValueError, TypeError) as e:
                    logger.warning(f"Could not convert pin {pin_index} value '{value}' to float: {e}")
                    continue
            
            # Write all points at once
            if points:
                self.write_api.write(bucket=self.bucket, org=self.org, record=points)
                # logger.info(f"✅ Logged {len(points)} data points to InfluxDB for device {device_id} (user: {owner})")
                return True
            else:
                logger.warning(f"⚠️ No valid data points to log for device {device_id}")
                return False
                
        except Exception as e:
            logger.error(f"❌ Failed to log device status to InfluxDB for {device_id}: {e}")
            
        return False

    def log_device_summary(self, device_id: str, owner: str, status_data: Dict, 
                          ip_address: str = "unknown"):
        """
        Log device summary/heartbeat data
        
        Args:
            device_id: Unique device identifier
            owner: Username who owns the device
            status_data: Status data
            ip_address: Device IP address
        """
        if not self.is_available():
            return False

        try:
            timestamp = datetime.utcnow()
            stat_array = status_data.get("stat", [])
            
            # Create summary point
            point = (Point("device_summary")
                    .tag("user", owner)
                    .tag("device_id", device_id)
                    .tag("ip_address", ip_address)
                    .field("pin_count", len(stat_array))
                    .field("online", 1)  # Device is responding
                    .time(timestamp, WritePrecision.S))
            
            self.write_api.write(bucket=self.bucket, org=self.org, record=point)
            logger.debug(f"Logged summary data for device {device_id} (user: {owner})")
            return True
            
        except Exception as e:
            logger.error(f"Failed to log device summary to InfluxDB: {e}")
            
        return False

    def close(self):
        """Close InfluxDB client connection"""
        if self.client:
            self.client.close()

# Global instance
influx_logger = InfluxLogger()

def log_device_data(device_id: str, owner: str, status_data: Dict, 
                   pin_setup: Optional[Dict] = None, ip_address: str = "unknown"):
    """
    Convenience function to log device data
    
    Args:
        device_id: Unique device identifier
        owner: Username who owns the device
        status_data: Status data containing 'stat' array
        pin_setup: Optional pin configuration
        ip_address: Device IP address
    """
    if influx_logger.is_available():
        # Log individual pin data
        influx_logger.log_device_status(device_id, owner, status_data, pin_setup)
        
        # Log summary data
        influx_logger.log_device_summary(device_id, owner, status_data, ip_address)
        
        return True
    return False