# Flood Monitoring IoT System

IoT-based flood monitoring system with real-time water level and flow rate detection using **JSN-SR04T ultrasonic sensor** and **YF-S201 flow sensor**. Data is transmitted via **SIM800L GSM module** using **MQTT protocol**, processed by **Arduino Nano**, and visualized through a monitoring dashboard.  

This project was developed as a **Final Project / Undergraduate Thesis** at **Politeknik Negeri Jakarta**.

---

## ✨ Features
- Real-time monitoring of **water level** and **flow rate**.
- Early warning system with **LED indicators** (Safe, Alert, Danger).
- Data transmission over **GSM 2G (SIM800L)**.
- MQTT-based communication with **HiveMQ broker**.
- Web-based dashboard for visualization.
- System availability: **98.21%** and durability: **95.76%** (based on testing).

---

## 🛠️ Hardware Components
- Arduino Nano (ATmega328P)  
- Ultrasonic Sensor JSN-SR04T (water level)  
- Flow Sensor YF-S201 (water flow rate)  
- GSM Module SIM800L (data transmission)  
- LED Traffic Light Indicator (Green, Yellow, Red)  
- Power Supply 5V / Step-down Converter  
- Project Box (outdoor protection)  

---

## 💻 Software & Tools
- Arduino IDE (C++)  
- HiveMQ MQTT Broker (hosted on AWS EC2)  
- Python (for data logging & analysis)  
- Web Dashboard (for monitoring visualization)  

---

## 📂 Repository Structure
