# ESP32 Automatic Generator Start Device

This project is an Automatic Generator Start/Stop Controller built with an ESP32.  
It monitors mains power availability and automatically starts or stops a generator based on supply status.  
It also supports manual mode operation, WiFi control, and static IP configuration.

---

## ✨ Features
- ✅ Automatic generator starts when mains power fails  
- ✅ Automatic generator stops when mains power returns  
- ✅ Manual mode (start/stop via button or web interface)  
- ✅ ESP32 Web Server interface for control & monitoring  
- ✅ Static IP configuration supported  
- ✅ Status indication via GPIO pins and Serial Monitor  

---

## 🛠️ Hardware Requirements
- ESP32 Development Board  
- Relay module (for generator start/stop control)  
- Power supply (5V for ESP32, relay driver circuit)  
- Push buttons (for manual mode)  
- Generator with start/stop input capability  
- WiFi network (for remote access)  

---

## 📂 Repository Structure
```
GEN-START/
│── src/
│   └── gen-start.ino   # Main Arduino code
│── schematic/
│   └── schematic.png  # Circuit diagram
│── README.md
|__ FINAL-OUTPUT.PNG
```



## 🚀 Getting Started



###  Open the Arduino IDE
- Install ESP32 board support from **Boards Manager**  
- Select your ESP32 board and correct COM port  

###  Upload the Code
- Open `src/GEN-START.ino` in Arduino IDE  
- Adjust WiFi SSID & Password inside the code  
- Click **Upload**  

### Connect the Hardware
- Relay output → Generator start/stop pins  
- Input pins → Mains power sense, manual buttons  

###  Access Web Dashboard
- Open browser → enter ESP32 IP (static or DHCP assigned)  
- Control & monitor generator status  

---

## 📸 Demo & Circuit
Schematic: `SCHEMATIC.png`  








