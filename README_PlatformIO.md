# Proyecto PlatformIO - Arduino UNO Blink

Este proyecto contiene la configuración básica de PlatformIO para Arduino UNO con un programa de parpadeo de LED.

## Estructura del Proyecto

```
MPPT-XY7025/
├── platformio.ini          # Configuración de PlatformIO
├── src/
│   └── main.cpp            # Código de parpadeo LED
└── README_PlatformIO.md    # Este archivo
```

## Configuración

### platformio.ini
- **Plataforma**: atmelavr (Arduino AVR)
- **Board**: uno (Arduino UNO)
- **Framework**: arduino
- **Velocidad de monitor serie**: 9600 baudios

### Código Principal (src/main.cpp)
- Parpadeo del LED integrado (pin 13) cada 500ms
- Incluye salida serial para monitoreo

## Instrucciones de Uso

### Prerrequisitos
1. **Extensión PlatformIO para VSCode**: Asegúrate de tener instalada la extensión "PlatformIO IDE" en VSCode.
2. **Arduino UNO**: Conecta tu placa Arduino UNO al puerto USB.

### Pasos para Compilar y Subir

1. **Abrir el proyecto en VSCode**:
   - Abre la carpeta del proyecto en VSCode
   - La extensión PlatformIO debería detectar automáticamente el proyecto

2. **Compilar el proyecto**:
   - Abre la paleta de comandos (Ctrl+Shift+P)
   - Ejecuta "PlatformIO: Build"
   - O haz clic en el icono de "check" en la barra inferior

3. **Subir el código al Arduino**:
   - Conecta el Arduino UNO por USB
   - Abre la paleta de comandos (Ctrl+Shift+P)
   - Ejecuta "PlatformIO: Upload"
   - O haz clic en el icono de "flecha hacia la derecha" en la barra inferior

4. **Monitor Serie**:
   - Para ver los mensajes seriales, ejecuta "PlatformIO: Serial Monitor"
   - O haz clic en el icono de "plug" en la barra inferior

## Configuración de Puertos Seriales

El archivo `platformio.ini` está configurado para usar `/dev/ttyUSB0`. Si tu Arduino está en un puerto diferente, edita las líneas:

```ini
upload_port = /dev/ttyACM0  # Ejemplo para Linux
monitor_port = /dev/ttyACM0 # Ejemplo para Linux
```

**Puertos comunes**:
- Linux: `/dev/ttyUSB0`, `/dev/ttyACM0`
- Windows: `COM3`, `COM4`, etc.
- macOS: `/dev/cu.usbmodemXXXX`

## Comandos Alternativos por Terminal

Si tienes PlatformIO instalado globalmente:

```bash
# Compilar el proyecto
pio run

# Subir al Arduino
pio run --target upload

# Monitor serie
pio device monitor
```

## Estado del Proyecto

✅ **Completado**:
- Configuración de PlatformIO para Arduino UNO
- Código básico de blink LED
- Commit en rama DEV del repositorio git

⚠️ **Pendiente de verificación**:
- Funcionamiento de la extensión PlatformIO en VSCode
- Compilación y upload exitosos

## Notas Técnicas

- **Microcontrolador**: ATmega328P
- **Frecuencia**: 16 MHz
- **Voltaje**: 5V
- **LED integrado**: Pin digital 13

El código hace parpadear el LED integrado cada segundo (500ms encendido, 500ms apagado) e imprime mensajes por el puerto serial.