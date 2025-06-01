# MySpectralSensor

# 🌿 ESP32S3-Based Spectral Sensor System

This project is a **low-cost, modular spectral measurement system** using the **SparkFun AS7265x Spectral Sensor** and **ESP32S3**, capable of performing reflectance and transmittance measurements. It streams real-time calibrated data to **InfluxDB** and supports a web-based interface via a **Java/React backend**.

---

## 📦 Features

- Captures **calibrated spectral data** from 410–940 nm
- Measures both **reflectance and transmittance** using active LED control
- Integrated **servo-based clamshell** for sample clamping
- Supports **WiFi TCP commands** and **serial control**
- Sends data to **InfluxDB** for visualization and analysis
- State machine-driven automation with measurement cycles
- Designed for **leaf sensing**, colored paper evaluation, and material analysis

---

## 🛠️ Hardware Requirements

- **ESP32-S3** microcontroller (with WiFi)
- **SparkFun AS7265x Triad Spectral Sensor**
- **Custom PCB** with:
  - White + IR LEDs (MOSFET controlled)
  - Servo motor for mechanical clamping
- **3D-printed case**
- **Power Supply** (5V)
- **WiFi network access**

---

## 🧪 Measurement Flow

1. `s` command: Full automatic measurement cycle  
   - Reflectance & Transmittance (denominator/dark)  
   - Transmittance (denominator, illuminated)  
2. `p` command: Leaf inserted  
   - Reflectance (nominator, illuminated)  
   - Reflectance & Transmittance (nominator/dark)  
   - Transmittance (nominator, illuminated)  
3. `f` command: Resets to ready state

---

## 🌐 InfluxDB Integration

The system writes measurements to InfluxDB Cloud or local server.

### InfluxDB Config (defined in `#define` macros)
- `INFLUXDB_URL`: Server URL (e.g., `http://192.168.x.x:8086`)
- `INFLUXDB_TOKEN`: Auth token
- `INFLUXDB_ORG`: Organization ID
- `INFLUXDB_BUCKET`: Target database (e.g., `From_ESP32S3`)

---

## 📡 Network Setup

- Connects to WiFi using:
  ```cpp
  #define WIFI_SSID "Spectral_Sensor_Lasha"
  #define WIFI_PASSWORD "12345678"
