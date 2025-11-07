# Configuración para PlatformIO

Para compilar y probar este sketch con PlatformIO, crea un archivo `platformio.ini` en la raíz del proyecto con el siguiente contenido:

```ini
[env:uno]
platform = atmelavr
board = uno
framework = arduino
lib_deps = 
    arduino-libraries/SoftwareSerial@^1.0
    4-20ma/ModbusMaster@^2.0.1
    XY7025_Modbus
lib_extra_dirs = 
    lib
monitor_speed = 9600
monitor_filters = 
    default
    time
```

## Dependencias

El proyecto requiere las siguientes librerías:

1. **SoftwareSerial** (incluida en Arduino IDE)
2. **ModbusMaster** de 4-20ma (versión 2.0.1 o superior)
3. **XY7025_Modbus** (librería propia en `lib/XY7025_Modbus/`)

## Compilación

```bash
# Instalar PlatformIO Core
pip install platformio

# Compilar el proyecto
pio run

# Subir a Arduino UNO
pio run --target upload

# Monitorear puerto serial
pio device monitor
```

## Configuración del Monitor Serial

El monitor serial debe configurarse a 9600 baudios para los mensajes de debug del Arduino. La comunicación Modbus con el XY7025 se realiza a 115200 baudios.

## Notas de Compilación

- El sketch está optimizado para Arduino UNO
- Usa SoftwareSerial en pines D2 (RX) y D3 (TX)
- La memoria RAM debe ser monitoreada debido al uso de strings
- Se recomienda usar Arduino IDE 1.8.x o superior