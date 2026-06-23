# 📡 Micro Radar (ESP32 CYD Edition)

A tiny, premium, open-source flight radar for your desk. This version has been customized and upgraded from the [original round ESP32 micro-radar](https://github.com/AnthonySturdy/micro-radar) to run on the **ESP32 Cheap Yellow Display (CYD)**—a 3.2" 320x240 landscape color screen with interactive capacitive touch controls.

<p align="center">
  <img src="https://github.com/user-attachments/assets/2ccb2063-d15c-4180-8e3c-ae3a81c814ff" alt="Micro Radar" width="400"/>
</p>

---

## ✨ Features (CYD Edition Upgrades)

* **Interactive Capacitive Touch Controls**:
  - **Focus Flight**: Tap on any tracked aircraft to select it.
  - **Live Dashboard**: Selecting an aircraft shows its Callign, Altitude, Speed, Distance from home, Heading, and flight path.
  - **Interactive Zoom**: Tap the radar grid to zoom in or zoom out dynamically.
  - **Clear Selection**: Tap the screen background to deselect the current aircraft.
* **Airport Route Lookup**:
  - Automatically queries the OpenSky Network Route API on-demand when an aircraft is focused.
  - Renders takeoff and landing airport codes directly on the dashboard (e.g. `EGKK->EGAA`).
* **Custom Map Paths (Vector Boundaries)**:
  - Supports entering coordinate lists to draw detailed coastlines, national borders, or restricted airspace in bright green vector lines.
* **Altitude Unit Toggle**:
  - Switch between **Meters (m)** and **Feet (ft)** directly via the settings page.
* **Dual-Core Safety & Stability**:
  - Built-in configuration cache prevents continuous NVS flash wear and watchdog resets.
  - Smart port-release logic resolves port 80 conflicts between WiFiManager and the settings server.

---
## 🚀 Getting Started

If you want to modify or flash the source code:

1. Install [VS Code](https://code.visualstudio.com/) and the [PlatformIO IDE extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide).
2. Open this folder in VS Code. PlatformIO will automatically fetch all toolchains and library dependencies.
3. Plug in your ESP32 CYD board.
4. Click the **Upload** button (→ arrow) in the bottom status bar.

---

## 🔧 First-Time Setup & WiFi Configuration

1. On first boot (or after erasing flash), the device creates a Wi-Fi hotspot named **`MicroRadar-Setup`**.
2. Connect to this hotspot from your phone or laptop.
3. A setup page will display automatically (or navigate to `http://192.168.4.1` in your browser).
4. Select your home network, enter your WiFi password, and click **Save**.
5. The device will save credentials and automatically perform a **clean reboot** to connect.

---

## ⚙️ Settings Configuration

Once connected to your home network, open any browser on the same network and navigate to:
👉 **[http://microradar.local](http://microradar.local)** (or the IP address shown on the screen setup log, e.g., `http://10.10.0.38`)

Here you can customize:
* **Latitude & Longitude**: The center point coordinates of your desk radar.
* **Radar Radius**: Scan diameter in degrees (max `2.5` degrees to comply with rate limiting).
* **OpenSky API Credentials**: Your client ID and secret (highly recommended to avoid strict rate-limiting bounds).
* **Altitude Unit**: Toggle between **Meters (m)** and **Feet (ft)**.
* **Display Toggles**: Enable/disable visual sweeps, directional headings, or aircraft labels.
* **Map Paths**: Paste JSON coordinate sequences to draw custom maps.

### Custom Map Example (Northern Ireland Coastline)
Copy and paste this outline into the **Map Paths** field on the settings page to draw Belfast Lough, the border, and Lough Neagh:
```json
[
  [
    [55.19, -6.95],
    [55.17, -6.71],
    [55.24, -6.51],
    [55.20, -6.25],
    [55.19, -6.06],
    [55.12, -6.04],
    [54.99, -5.99],
    [54.85, -5.80],
    [54.75, -5.71],
    [54.71, -5.80],
    [54.68, -5.87],
    [54.66, -5.90],
    [54.61, -5.92],
    [54.64, -5.84],
    [54.66, -5.73],
    [54.66, -5.67],
    [54.68, -5.61],
    [54.64, -5.53],
    [54.49, -5.44],
    [54.38, -5.55],
    [54.26, -5.60],
    [54.21, -5.88],
    [54.11, -5.89],
    [54.06, -6.00],
    [54.10, -6.25]
  ],
  [
    [54.10, -6.25],
    [54.15, -6.65],
    [54.25, -7.00],
    [54.12, -7.60],
    [54.48, -8.08],
    [54.75, -7.80],
    [55.04, -7.31],
    [55.00, -7.32],
    [55.19, -6.95]
  ],
  [
    [54.76, -6.46],
    [54.71, -6.24],
    [54.62, -6.23],
    [54.51, -6.37],
    [54.48, -6.43],
    [54.61, -6.50],
    [54.70, -6.53],
    [54.76, -6.46]
  ]
]
```

---

## 🛠️ Hardware Requirements (CYD Edition)

* **Display Module**: [ESP32 Cheap Yellow Display (CYD) - ESP32-2432S028R](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
* **Touch Controller**: Built-in resistive touch panel (pre-calibrated for VSPI pins in firmware).
* **Power Supply**: Standard Micro-USB or USB-C cable (5V, 1A).

---

## 📝 Notes & Credits

> Customized and upgraded from the [original round ESP32 micro-radar repository by AnthonySturdy](https://github.com/AnthonySturdy/micro-radar).
> Inspired by [therealhacksaw](https://www.instagram.com/therealhacksaw/)'s desk radar.
