# Guía de Configuración y Compilación - XY7025 Address Finder

## 📋 Resumen

Esta guía proporciona instrucciones paso a paso para configurar, compilar y cargar el proyecto XY7025_Address_Finder en Arduino UNO usando PlatformIO.

## 🎯 Objetivo

El proyecto XY7025_Address_Finder es una herramienta interactiva para:
- Detectar automáticamente la dirección slave de un dispositivo XY7025
- Buscar el baudrate correcto de comunicación
- Configurar y guardar parámetros en el dispositivo
- Probar la conexión Modbus

## 🔧 Configuración del Hardware

### Conexiones Arduino UNO → XY7025

```
Arduino UNO    |    XY7025
-------------|-------------
Pin 2 (RX)   →    Pin RX
Pin 3 (TX)   →    Pin TX
GND          →    GND
5V           →    VCC (opcional, si el XY7025 lo requiere)
```

**Nota**: No conecte el pin 5V si el XY7025 ya tiene su propia fuente de alimentación.

## 📦 Requisitos Previos

### Software Necesario
- [PlatformIO Core](https://platformio.org/install) o [PlatformIO IDE](https://platformio.org/install/ide)
- [Git](https://git-scm.com/downloads) (opcional, para control de versiones)

### Hardware Compatible
- Arduino UNO (ATmega328P)
- Cable USB tipo A-B para Arduino UNO
- Dispositivo XY7025 MPPT
- Cables de conexión (jumpers o cable Dupont)

## 🚀 Instrucciones de Compilación

### 1. Preparar el Entorno

```bash
# Verificar que PlatformIO esté instalado
pio --version

# Navegar al directorio del proyecto
cd /home/c2mismo/PROYECTOS/MPPT-XY7025
```

### 2. Verificar la Configuración

La configuración de PlatformIO está optimizada para Arduino UNO con las siguientes características:

**Entorno Principal: `uno_xy7025`**
- **Plataforma**: Atmel AVR
- **Board**: Arduino UNO
- **Frecuencia**: 16 MHz
- **Memoria Flash**: 31.5 KB disponibles
- **Memoria RAM**: 2 KB
- **Velocidad de Monitor**: 115200 bps

**Librerías Incluidas:**
- `ModbusMaster@^2.0.1` - Comunicación Modbus RTU
- `SoftwareSerial@1.0` - Puerto serial por software
- `XY7025_Modbus` - Librería personalizada para XY7025

### 3. Compilar el Proyecto

```bash
# Compilar para Arduino UNO (entorno principal)
pio run --environment uno_xy7025

# Compilar con modo debug (más información)
pio run --environment uno_xy7025_debug

# Compilar para Arduino Mega 2560 (más memoria)
pio run --environment mega_xy7025

# Compilar para ESP32 (opcional)
pio run --environment esp32_xy7025
```

### 4. Verificar el Resultado

Si la compilación es exitosa, verá:
```
========================= [SUCCESS] Took 0.88 seconds =========================
Environment    Status    Duration
-------------  --------  ------------
uno_xy7025     SUCCESS   00:00:00.880
========================= 1 succeeded in 00:00:00.880 =========================
```

**Estadísticas de Memoria:**
- **RAM**: 33.3% (682 bytes de 2048 bytes)
- **Flash**: 65.6% (21166 bytes de 32256 bytes)

## 🔌 Carga del Firmware

### 1. Conectar el Arduino
1. Conecte el Arduino UNO al computador via USB
2. Verifique que el dispositivo sea reconocido:
   ```bash
   # Linux/Mac
   ls /dev/ttyUSB* /dev/ttyACM*
   
   # Windows (en PowerShell)
   Get-WMIObject Win32_SerialPort
   ```

### 2. Cargar el Código

```bash
# Cargar automáticamente (detecta puerto)
pio run --target upload --environment uno_xy7025

# Cargar con puerto específico
pio run --target upload --environment uno_xy7025 --upload-port /dev/ttyUSB0
```

### 3. Verificar la Carga
El LED TX del Arduino parpadeará durante la carga. Una vez completada, verá:
```
========================= [SUCCESS] Took 2.15 seconds =========================
```

## 📊 Uso del Programa

### 1. Iniciar el Monitor Serial

```bash
# Abrir monitor serial con la configuración correcta
pio device monitor --baud 115200 --port /dev/ttyUSB0
```

### 2. Opciones del Menú

Una vez cargado, el programa mostrará un menú interactivo:

```
=== CONFIGURADOR XY7025 ===
Estado actual:
  Slave Address: 1
  Baudrate: 115200 bps (índice 6)
  Conexión: ERROR
  Debug Mode: ON

Opciones:
1. [a] Verificar conexión actual
2. [s] Buscar dirección Slave (1-247)
3. [b] Buscar Baudrate completo (proceso largo)
4. [w] Escribir Slave en XY7025
5. [r] Escribir Baudrate en XY7025
6. [m] Cambiar Slave local (Arduino)
7. [n] Cambiar Baudrate local (Arduino)
8. [h] Ayuda
9. [q] Salir
10. [d] Toggle Debug Mode
```

### 3. Flujo Recomendado

1. **Primera vez**: Use `[s]` para buscar la dirección slave
2. **Si no encuentra nada**: Use `[b]` para búsqueda exhaustiva de baudrate
3. **Guardar configuración**: Use `[w]` o `[r]` según necesite
4. **Verificar**: Use `[a]` para confirmar la conexión

## 🔍 Solución de Problemas

### Error: "No se encontró ningún dispositivo"
- Verifique las conexiones físicas
- Asegúrese que el XY7025 esté encendido
- Pruebe diferentes baudrates con `[b]`
- Revise que los pines RX/TX no estén invertidos

### Error de Compilación
- Verifique que todas las librerías estén instaladas
- Actualice PlatformIO: `pio upgrade`
- Limpie el proyecto: `pio run --target clean`

### Error de Carga
- Verifique que el Arduino esté correctamente conectado
- Cierre cualquier otro programa que use el puerto serial
- Instale los drivers CH340/CP2102 si es necesario
- Pruebe con un cable USB diferente

### Comunicación Inestable
- Use cables cortos para las conexiones
- Agregue resistencias de pull-up (10kΩ) en las líneas RX/TX
- Verifique la alimentación del XY7025
- Aleje el cableado de fuentes de interferencia

## 📁 Estructura del Proyecto

```
MPPT-XY7025/
├── platformio.ini          # Configuración de PlatformIO
├── lib/
│   └── XY7025_Modbus/      # Librería personalizada
│       ├── XY7025_Modbus.h
│       └── XY7025_Modbus.cpp
├── examples/
│   └── XY7025_Address_Finder/
│       └── XY7025_Address_Finder.cpp  # Código principal
└── Documentation/          # Documentación adicional
```

## 🛠️ Entornos de Compilación Disponibles

| Entorno | Placa | Optimización | Uso |
|---------|--------|-------------|-----|
| `uno_xy7025` | Arduino UNO | Release | Producción |
| `uno_xy7025_debug` | Arduino UNO | Debug | Desarrollo |
| `mega_xy7025` | Arduino Mega 2560 | Release | Más memoria |
| `esp32_xy7025` | ESP32 DevKit | Release | WiFi/Bluetooth |

## 📞 Soporte

Para problemas técnicos:
1. Revise esta guía de configuración
2. Consulte los archivos de documentación en `/Documentation`
3. Verifique los mensajes de error en el monitor serial
4. Controle las conexiones físicas del hardware

## 📄 Licencia

Este proyecto es parte del sistema MPPT XY7025 y está sujeto a los términos de licencia correspondientes.