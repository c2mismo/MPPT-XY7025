# MPPT-XY7025 Controller
# Controlador MPPT-XY7025

**English / Español**

## Project Description / Descripción General
**English:**  
This is an experimental project for interfacing and controlling XY7025 solar charge controllers using Arduino or ESP32 microcontrollers. It focuses on Maximum Power Point Tracking (MPPT) applications via the Modbus RTU protocol. The system supports data logging, precise time monitoring with RTC, and high-resolution analog acquisition for real-time performance analysis.

**Español:**  
Este es un proyecto experimental para la interfaz y control de controladores solares XY7025 utilizando microcontroladores Arduino o ESP32. Se centra en aplicaciones de Maximum Power Point Tracking (MPPT) mediante el protocolo Modbus RTU. El sistema soporta logging de datos, monitoreo preciso de tiempo con RTC y adquisición analógica de alta resolución para análisis de rendimiento en tiempo real.

## Main Hardware Components / Componentes Hardware Principales
**English:**  
- **Arduino or ESP32**: Main microcontroller for processing and communication.  
- **DS3231 RTC Module**: High-precision real-time clock for synchronization and timestamps in data logs.  
- **MicroSD Card Adapter Module**: For storing performance logs, events, and historical data.  
- **ADS1115 ADC Module**: I2C interface with integrated comparator for 16-bit precision voltage/current measurements.

**Español:**  
- **Arduino o ESP32**: Microcontrolador principal para procesamiento y comunicación.  
- **Módulo RTC DS3231**: Reloj en tiempo real de alta precisión para sincronización y timestamps en logs de datos.  
- **Módulo Adaptador de Tarjeta MicroSD**: Para almacenamiento de registros de rendimiento, eventos y datos históricos.  
- **Módulo ADC ADS1115**: Interfaz I2C con comparador integrado para mediciones de voltaje/corriente con precisión de 16 bits.

## Features and Functionalities / Características y Funcionalidades
**English:**  
- Modbus RTU protocol implementation for XY7025 communication (read/write registers for PV voltage, current, power, etc.).  
- Data logging to MicroSD with DS3231 timestamps.  
- Real-time analog signal acquisition and processing via ADS1115.  
- Initial support for protection testing (overload, short-circuit) and Modbus command validation.

**Español:**  
- Implementación del protocolo Modbus RTU para comunicación con XY7025 (lectura/escritura de registros para voltaje PV, corriente, potencia, etc.).  
- Logging de datos en MicroSD con timestamps del DS3231.  
- Adquisición y procesamiento de señales analógicas en tiempo real vía ADS1115.  
- Soporte inicial para pruebas de protección (sobrecarga, cortocircuito) y validación de comandos Modbus.

## Repository Structure / Estructura del Repositorio
**English:**  
- `/src/`: Main source code (Arduino/ESP32 sketches).  
- `/Documentación/`: Support files, including "PROTOCOLO MODBUS MPPT XY7025.pdf" (Technical document detailing the Modbus protocol for XY7025 MPPT controllers, with register maps, configuration commands, and implementation examples).  
- `/tests/`: Initial test scripts and data.  
- `/hardware/`: Schematic diagrams (if available) or connection notes.  
- `.gitignore`: Ignores temporary files, builds, and sensitive logs.

**Español:**  
- `/src/`: Código fuente principal (sketches de Arduino/ESP32).  
- `/Documentación/`: Archivos de soporte, incluyendo "PROTOCOLO MODBUS MPPT XY7025.pdf" (Documento técnico que detalla el protocolo Modbus para controladores MPPT XY7025, con mapas de registros, comandos de configuración y ejemplos de implementación).  
- `/tests/`: Scripts y datos de pruebas iniciales.  
- `/hardware/`: Diagramas esquemáticos (si disponibles) o notas de conexiones.  
- `.gitignore`: Ignora archivos temporales, builds y logs sensibles.

## Project Status / Estado del Proyecto
**English:**  
The project is in its initial experimental phase. Attached report covers:  
- Initial hardware verification.  
- Basic electrical tests (rest voltages, currents).  
- Dynamic tests (response to variable loads).  
- Modbus protocol validation (successful read/write of key registers).  
- Protection tests (fault detection and handling).  
Future phases will include full integration and optimization.

**Español:**  
El proyecto está en su fase experimental inicial. El informe adjunto cubre:  
- Verificación inicial del hardware.  
- Pruebas eléctricas básicas (voltajes, corrientes en reposo).  
- Pruebas dinámicas (respuesta a cargas variables).  
- Validación del protocolo Modbus (lectura/escritura exitosa de registros clave).  
- Pruebas de protección (detección y manejo de fallos).  
Fases futuras incluirán integración completa y optimización.

## Installation and Usage Instructions / Instrucciones de Instalación y Uso
**English:**  
### Requirements / Requisitos  
- Arduino IDE or PlatformIO.  
- Libraries: ModbusMaster, RTClib (for DS3231), SD, Adafruit_ADS1X15.  

### Setup Steps / Pasos de Configuración  
1. Clone the repo: `git clone https://github.com/yourusername/MPPT-XY7025.git` (replace with actual URL).  
2. Install dependencies via Library Manager in Arduino IDE.  
3. Connect hardware: See pinout table below.  
4. Load the sketch from `/src/main.ino`.  
5. Monitor serial output for debugging; check MicroSD for logs.

**Example Pin Connections / Ejemplo de Conexiones de Pines**  

| Component | Pin | Microcontroller Pin | Notes |
|-----------|-----|---------------------|-------|
| DS3231 | SDA | A4 (Uno) / GPIO21 (ESP32) | I2C Data |
| DS3231 | SCL | A5 (Uno) / GPIO22 (ESP32) | I2C Clock |
| MicroSD | MISO | Pin 12 | SPI |
| MicroSD | MOSI | Pin 11 | SPI |
| ADS1115 | SDA | A4 / GPIO21 | I2C |
| XY7025 (TTL) | TX | Pin 2 | Modbus TX |
| XY7025 (TTL) | RX | Pin 3 | Modbus RX |

**Usage Example / Ejemplo de Uso**  
Upload the sketch, open Serial Monitor (115200 baud). Logs will be saved to `/logs/performance.csv` on MicroSD with format: `Timestamp,Voltage,Current,Power`.

**Español:**  
### Requisitos  
- Arduino IDE o PlatformIO.  
- Bibliotecas: ModbusMaster, RTClib (para DS3231), SD, Adafruit_ADS1X15.  

### Pasos de Configuración  
1. Clonar el repo: `git clone https://github.com/yourusername/MPPT-XY7025.git` (reemplaza con URL real).  
2. Instalar dependencias vía Library Manager en Arduino IDE.  
3. Conectar hardware: Ver tabla de pines abajo.  
4. Cargar el sketch de `/src/main.ino`.  
5. Monitorear salida serial para depuración; revisar logs en MicroSD.

**Ejemplo de Conexiones de Pines**  
(Same table as above / Misma tabla arriba)

**Ejemplo de Uso**  
Cargar el sketch, abrir Monitor Serial (115200 baud). Los logs se guardarán en `/logs/performance.csv` en MicroSD con formato: `Timestamp,Voltage,Current,Power`.

## Contributions and License / Contribuciones y Licencia
**English:**  
Contributions welcome via issues or pull requests. Please follow coding standards and test changes.  

Licensed under the GNU Lesser General Public License (LGPL). See [LICENSE](LICENSE) for details. The .gitignore excludes temporary files, builds, and sensitive logs.

**Español:**  
Se aceptan contribuciones vía issues o pull requests. Por favor, sigue estándares de codificación y prueba cambios.  

Licenciado bajo la GNU Lesser General Public License (LGPL). Ver [LICENSE](LICENSE) para detalles. El .gitignore excluye archivos temporales, builds y logs sensibles.

## Additional Notes / Notas Adicionales
**English:**  
For technical details, refer to [Documentación/PROTOCOLO MODBUS MPPT XY7025.pdf](Documentación/PROTOCOLO%20MODBUS%20MPPT%20XY7025.pdf).  
**Status:** Experimental – not for production use.  

![License](https://img.shields.io/badge/license-LGPL-blue.svg)  
![Build Status](https://img.shields.io/badge/build-experimental-yellow.svg)

**Español:**  
Para detalles técnicos, consulta [Documentación/PROTOCOLO MODBUS MPPT XY7025.pdf](Documentación/PROTOCOLO%20MODBUS%20MPPT%20XY7025.pdf).  
**Estado:** Experimental – no para uso en producción.  

(Same badges / Mismos badges)