# XY7025 Modbus Test Suite - Guía de Uso

## Descripción
Sketch completo para probar la librería XY7025_Modbus con el dispositivo MPPT XY7025. Permite verificar la dirección del esclavo, leer todos los registros y escribir valores para confirmar la funcionalidad del dispositivo.

## Características Principales

### 1. Verificación de Dirección del Esclavo
- Lee el registro `SLAVE-ADD` (0x0018) para confirmar la dirección configurada
- Permite probar diferentes direcciones (1-247) si la documentada no responde
- Función `probeSlaveAddress()` para detectar dispositivos en el bus

### 2. Lectura Completa de Registros
- Lee todos los registros del XY7025 usando `readAllRegisters()`
- Muestra voltaje, corriente, potencia, temperaturas, estado de protecciones
- Volcado completo de datos para análisis con `screen -L`

### 3. Lectura de Registro Específico
- Lee cualquier registro individual por dirección hexadecimal
- Interpretación automática para registros conocidos (voltajes, corrientes, temperaturas)
- Visualización multi-formato: decimal, hexadecimal y binario
- Exploración interactiva de registros desconocidos

### 4. Escritura de Valores
- Permite escribir valores específicos en registros
- Test interactivo para cambiar voltaje y verificar en pantalla física
- Función de restauración automática de valores originales

## Conexiones de Hardware

```
Arduino UNO    |    XY7025 MPPT
--------------|------------------
D2 (RX)       |    TX
D3 (TX)       |    RX
GND           |    GND
5V (opcional) |    VCC (si se requiere alimentación)
```

**Nota**: No conectar directamente 5V de Arduino al XY7025 sin verificar los niveles lógicos del dispositivo.

## Comandos Disponibles

### Comandos Básicos
- `h` - Mostrar ayuda completa
- `a` - Verificar dirección esclavo actual
- `s` - Buscar dispositivos Modbus (escanea direcciones 1-20)

### Lectura de Datos
- `r` - Leer y mostrar TODOS los registros del dispositivo
- `v` - Leer valores básicos (Voltaje, Corriente, Potencia, Temperaturas)
- `l` - Leer registro específico por dirección hexadecimal

### Escritura de Datos
- `w` - Mostrar menú de escritura de registros
- `t` - Test de escritura (cambia voltaje y espera confirmación visual)

## Uso con Screen para Volcado de Datos

Para capturar todos los datos en un archivo:

```bash
# Conectar el Arduino y detectar el puerto
ls /dev/ttyUSB* /dev/ttyACM*

# Iniciar screen con logging
screen -L /dev/ttyUSB0 9600

# En el screen de Arduino, ejecutar:
# 'r' para leer todos los registros
# 's' para buscar dispositivos
# 'v' para valores básicos

# Salir de screen: Ctrl+A, K, Y
# El archivo se guardará como screenlog.0
```

## Ejemplos de Uso

### 1. Verificar Dirección del Dispositivo
```
=== Verificación de Dirección Esclavo ===
Dirección esclavo en registro: 1
✓ La dirección coincide con la configurada
Probando comunicación con dirección 1... ✓ COMUNICACIÓN OK
```

### 2. Buscar Dispositivos en el Bus
```
=== Búsqueda de Dispositivos Modbus ===
Escaneando direcciones 1-20
Dirección | Estado
---------|-------
    1    | ENCONTRADO ✓
    2    | ---
    3    | ---
```

### 4. Lectura de Registro Específico
```
=== LECTURA DE REGISTRO ESPECÍFICO ===
Ingresa la dirección del registro en hexadecimal (0x0000-0x00ED)
Ingresa 4 dígitos hexadecimales (ej: 0002) o 'q' para cancelar:
0002
Leyendo registro 0x0002...
✓ Valor leído: 2415
 → Voltaje de salida: 24.15V
```

**Ejemplos de registros comunes:**
- `0000` → Voltaje configurado
- `0002` → Voltaje de salida actual
- `0003` → Corriente de salida actual
- `0012` → Estado de salida (ON/OFF)
- `0018` → Dirección esclavo actual

### 3. Lectura Completa de Registros
```
=== LECTURA COMPLETA DE REGISTROS ===
=== Estado del MPPT XY7025 ===
Voltaje Configurado: 24.00V
Corriente Configurada: 5.000A
Voltaje de Salida: 24.15V
Corriente de Salida: 2.345A
Potencia de Salida: 56.67W
Voltaje de Entrada: 24.89V
```

### 4. Test de Escritura Interactivo
```
=== TEST DE ESCRITURA ===
Voltaje actual: 24.00V
Estableciendo voltaje a: 26.00V...
✓ Comando de escritura enviado
Verifica en la pantalla del XY7025 si el voltaje cambió
Una vez verificado, presiona cualquier tecla para continuar...
```

## Formatos de Datos

Los datos se presentan con los siguientes formatos:
- **Voltaje**: 2 decimales (ej: 24.00V)
- **Corriente**: 3 decimales (ej: 5.000A)
- **Potencia**: 2 decimales (ej: 120.50W)
- **Temperatura**: 1 decimal (ej: 25.5°C)

## Solución de Problemas

### Sin Respuesta del Dispositivo
1. Verificar conexiones físicas (D2→TX, D3→RX, GND)
2. Confirmar que el XY7025 esté encendido
3. Probar comando 's' para buscar dispositivos
4. Verificar velocidad de comunicación (115200 baudios)

### Errores de Lectura
1. Verificar que la dirección esclavo sea correcta ('a')
2. Confirmar cableado sin intercambios TX/RX
3. Verificar alimentación del XY7025
4. Alejar de fuentes de interferencia

### Errores de Escritura
1. Verificar que el dispositivo no esté bloqueado
2. Confirmar que los valores estén dentro del rango permitido
3. Verificar estado de protecciones
4. Intentar con valores más conservadores

### Errores en Lectura de Registro Específico
1. Verificar que se ingresen exactamente 4 dígitos hexadecimales
2. Confirmar que la dirección esté en el rango 0x0000-0x00ED
3. Usar letras minúsculas para 'q' si se desea cancelar
4. Algunos registros pueden no estar implementados en el hardware

## Validación de la Documentación

Este sketch permite validar la documentación del XY7025:
1. **Dirección esclavo**: Confirmar si es realmente 1 por defecto
2. **Formato de datos**: Verificar decimales y unidades
3. **Registros disponibles**: Confirmar qué registros responden
4. **Rangos de valores**: Determinar límites reales del hardware

## Notas de Seguridad

- Siempre verificar valores en la pantalla física del XY7025
- No exceder los límites eléctricos del dispositivo
- Usar fusibles apropiados en las conexiones de potencia
- Monitorear temperaturas durante pruebas prolongadas

## Archivos Relacionados

- [`lib/XY7025_Modbus/`](lib/XY7025_Modbus/) - Librería Modbus
- [`ARCHITECTURE.md`](ARCHITECTURE.md) - Documentación técnica
- [`Documentation/`](Documentation/) - Documentación del protocolo
- [`IMPLEMENTATION_NOTES.md`](IMPLEMENTATION_NOTES.md) - Notas de implementación