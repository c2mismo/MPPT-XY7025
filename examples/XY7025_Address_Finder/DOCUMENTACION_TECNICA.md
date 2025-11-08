# Documentación Técnica - XY7025_Address_Finder

## 📋 Descripción General

XY7025_Address_Finder es un configurador interactivo para dispositivos MPPT XY7025 que permite detectar automáticamente la dirección slave Modbus y el baudrate de comunicación. El programa está diseñado para Arduino UNO y utiliza comunicación serial por software.

## 🎯 Características Principales

### Funcionalidades del Sistema
- **Detección Automática**: Busca direcciones slave entre 1-247
- **Búsqueda de Baudrate**: Prueba 9 velocidades de comunicación diferentes
- **Configuración Interactiva**: Menú completo con opciones de configuración
- **Verificación de Conexión**: Prueba de funcionamiento en tiempo real
- **Escritura de Parámetros**: Guarda configuración en el dispositivo XY7025
- **Modo Debug**: Información detallada de las operaciones

### Opciones del Menú Principal
```
[a] Verificar conexión actual
[s] Buscar dirección Slave (1-247)
[b] Buscar Baudrate completo (proceso largo)
[w] Escribir Slave en XY7025
[r] Escribir Baudrate en XY7025
[m] Cambiar Slave local (Arduino)
[n] Cambiar Baudrate local (Arduino)
[i] Información y opciones
[h] Ayuda
[q] Salir
[d] Toggle Debug Mode
```

## 🔧 Configuración Hardware

### Conexiones Arduino UNO → XY7025
```
Arduino UNO    |    XY7025
-------------|-------------
Pin 2 (RX)   →    Pin RX
Pin 3 (TX)   →    Pin TX
GND          →    GND
```

**Nota**: No conecte 5V si el XY7025 ya tiene alimentación propia.

## 📊 Parámetros Técnicos

### Constantes de Configuración
- **Dirección Slave Mínima**: 1
- **Dirección Slave Máxima**: 247
- **Timeout Serial**: 10,000 ms (10 segundos)
- **Timeout de Conexión**: 5,000 ms (5 segundos)
- **Máximo de Reintentos**: 3
- **Delay entre Reintentos**: 500 ms

### Tabla de Baudrates Soportados
| Índice | Velocidad | Estado |
|--------|-----------|---------|
| 0      | 9600 bps  | Estándar |
| 1      | 14400 bps | Opcional |
| 2      | 19200 bps | Estándar |
| 3      | 38400 bps | Rápido |
| 4      | 56000 bps | Alto |
| 5      | 57600 bps | Rápido |
| 6      | 115200 bps| **Por Defecto** |
| 7      | 2400 bps  | Lento |
| 8      | 4800 bps  | Lento |

## 🔍 Estados del Sistema

El sistema opera en cinco estados principales:
- **STATE_INIT**: Inicialización del sistema
- **STATE_MENU**: Esperando comandos del usuario
- **STATE_SEARCHING**: Búsqueda activa de dispositivos
- **STATE_CONNECTED**: Conexión establecida con XY7025
- **STATE_ERROR**: Error en la comunicación

## ⚡ Procesos de Búsqueda

### Búsqueda de Dirección Slave
1. Itera direcciones 1-247 con baudrate actual
2. Usa `probeSlaveAddress()` para detectar respuesta
3. Verifica con lectura de registro VOUT
4. Tiempo estimado: 1-30 segundos

### Búsqueda Exhaustiva de Baudrate
1. Prueba los 9 baudrates disponibles
2. Para cada baudrate, busca en direcciones 1-247
3. Tiempo estimado: 1-3 horas completas
4. Proceso cancelable presionando 'q'

## 💾 Gestión de Memoria

### Optimizaciones de RAM
- **Mensajes en PROGMEM**: Todos los textos constantes en memoria flash
- **Buffer Serial Software**: 64 bytes (optimizado para UNO)
- **Variables estáticas**: Uso eficiente de memoria RAM

### Uso de Recursos
- **Memoria Flash**: ~21KB (65% del total)
- **Memoria RAM**: ~680 bytes (33% del total)
- **Eficiencia**: Código optimizado para Arduino UNO

## 🔧 Funciones Principales

### testConnection()
Verifica la conexión actual leyendo el registro VOUT del dispositivo.

### searchSlaveAddress()
Busca automáticamente la dirección slave correcta en el rango 1-247.

### searchBaudrateComplete()
Realiza búsqueda exhaustiva de baudrate y dirección slave.

### writeSlaveToXY7025()
Guarda la dirección slave actual en el registro 0x0018 del XY7025.

### writeBaudrateToXY7025()
Guarda el baudrate actual en el registro 0x0019 del XY7025.

### verifyConnection()
Lee y muestra información detallada del dispositivo conectado.

## ⚠️ Consideraciones Importantes

### Reinicio Manual Requerido
Los cambios guardados en el XY7025 requieren:
1. Apagar y encender el dispositivo con el botón físico
2. NO es posible reiniciar por software
3. La nueva configuración se activa después del reinicio

### Proceso de Verificación
Después de cualquier cambio:
1. Use opción [a] para verificar conexión
2. Si falla, use [s] para buscar nueva configuración
3. Repita el proceso hasta encontrar la configuración correcta

### Cancelación de Procesos
- Presione 'q' durante búsquedas para cancelar
- El sistema mantiene la configuración anterior
- No se realizan cambios si se cancela

## 📈 Flujo de Trabajo Recomendado

### Primera Conexión
1. Conectar hardware según esquema
2. Cargar programa en Arduino UNO
3. Abrir monitor serial a 115200 bps
4. Presionar cualquier tecla para continuar
5. Usar [s] para buscar dirección slave
6. Si no encuentra, usar [b] para búsqueda exhaustiva

### Cambio de Configuración
1. Usar [m] o [n] para cambiar localmente
2. Verificar con [a] que funciona
3. Guardar con [w] o [r] en XY7025
4. Reiniciar manualmente el dispositivo
5. Verificar nuevamente con [a]

## 🛡️ Manejo de Errores

### Códigos de Error
- **ERROR_NONE (0)**: Operación exitosa
- **ERROR_NO_CONNECTION (1)**: Sin conexión establecida
- **ERROR_INVALID_ADDRESS (2)**: Dirección fuera de rango
- **ERROR_INVALID_BAUDRATE (3)**: Baudrate no soportado
- **ERROR_WRITE_FAILED (4)**: Error al escribir en XY7025
- **ERROR_TIMEOUT (5)**: Tiempo de espera agotado
- **ERROR_CANCELLED (6)**: Operación cancelada por usuario

### Mensajes de Error Comunes
- "✗ No hay conexión establecida": Verificar conexiones físicas
- "✗ Sin respuesta del dispositivo": XY7025 apagado o mal conectado
- "✗ Tiempo de espera agotado": Problema de comunicación
- "Operación cancelada": Usuario canceló el proceso

## 📚 Referencias Técnicas

### Registros Modbus Utilizados
- **0x0002 (XY7025_VOUT)**: Voltaje de salida actual
- **0x0018 (XY7025_SLAVE_ADD)**: Dirección slave del dispositivo
- **0x0019 (XY7025_BAUDRATE_L)**: Baudrate de comunicación

### Librerías Requeridas
- **SoftwareSerial**: Puerto serial por software
- **XY7025_Modbus**: Comunicación Modbus RTU personalizada
- **Arduino.h**: Funciones básicas de Arduino

---

**Documentación generada para el proyecto XY7025_Address_Finder**  
*Versión actualizada con todas las características implementadas*