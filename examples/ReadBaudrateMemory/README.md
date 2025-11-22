# ReadBaudrateMemory - Herramienta de Diagnóstico XY7025

## Resumen

`ReadBaudrateMemory` es un ejemplo especializado de la librería MPPT-XY7025 diseñado para diagnosticar y verificar los baudrates configurados en la memoria del dispositivo XY7025. Esta herramienta utiliza la librería `XY7025_Modbus` para leer de manera correcta los baudrates almacenados, enfocándose específicamente en identificar los valores reales de las posiciones 7 y 8, donde la documentación del fabricante es inconsistente.

## Propósito

- **Objetivo Principal**: Verificar los baudrates reales almacenados en el XY7025
- **Objetivo Secundario**: Confirmar si las posiciones 7 y 8 (2400 y 4800 baud) son funcionales
- **Objetivo Terciario**: Proporcionar un método de diagnóstico robusto usando la librería correcta

## Implementación Técnica

### ✅ Uso de Librería Correcta
- **Librería Base**: Utiliza `XY7025_Modbus.h` en lugar de comunicación directa con ModbusMaster
- **Método de Lectura**: Lee el registro `XY7025_BAUDRATE_L` (0x0019) que contiene el índice del baudrate configurado
- **Verificación Práctica**: Prueba cada baudrate cambiando temporalmente la comunicación para verificar respuesta

### ✅ Método de Verificación
1. **Lectura del Registro**: Usa `xy7025.readBaudrate()` para obtener el índice configurado
2. **Prueba Práctica**: Cambia temporalmente la velocidad de comunicación y verifica conectividad
3. **Tabla de Baudrates**: Utiliza la tabla verificada de `XY7025_Address_Finder`

## Baudrates Verificados

Según la investigación previa en `XY7025_Address_Finder`, los baudrates reales son:

| Posición | Baudrate | Estado Documentado | Observación |
|----------|----------|-------------------|-------------|
| 0 | 9600 | ✅ Correcto | Estándar |
| 1 | 14400 | ✅ Correcto | Estándar |
| 2 | 19200 | ✅ Correcto | Estándar |
| 3 | 38400 | ✅ Correcto | Estándar |
| 4 | 56000 | ✅ Correcto | Estándar |
| 5 | 57600 | ✅ Correcto | Estándar |
| 6 | 115200 | ✅ Correcto | Por defecto |
| 7 | 2400 | ⚠️ **Problemático** | Documentación inconsistente |
| 8 | 4800 | ⚠️ **Problemático** | Documentación inconsistente |

## Funcionalidades

### 🔍 Lectura de Configuración Actual
- Lee modelo y versión del dispositivo XY7025
- Obtiene dirección del esclavo configurada
- Determina el baudrate actualmente configurado
- Muestra voltaje de salida como verificación de conectividad

### 🧪 Prueba Sistemática de Baudrates
- Prueba cada uno de los 9 baudrates disponibles
- Cambia temporalmente la velocidad de comunicación
- Verifica conectividad en cada velocidad
- Restaura configuración original al finalizar

### 📊 Análisis de Resultados
- Tabla completa con estado de cada baudrate (FUNCIONA/NO FUNCIONA)
- Análisis específico de posiciones 7 y 8 (problemáticas)
- Conclusiones basadas en resultados experimentales
- Recomendaciones para uso futuro

### 🎯 Conclusiones Automáticas
El script proporciona conclusiones automáticas:
- **Si ambos funcionan**: Los baudrates documentados son correctos
- **Si ninguno funciona**: Se necesita investigación adicional
- **Resultados mixtos**: Requiere análisis más profundo

## Conexiones

```
XY7025                    Arduino UNO
┌─────────────────┐       ┌─────────────────┐
│ D+ (TX)         │──────▶│ Pin 2 (RX)      │
│ D- (RX)         │◀──────│ Pin 3 (TX)      │
│ GND             │──────▶│ GND             │
└─────────────────┘       └─────────────────┘
```

## Compilación y Uso

### Compilación con PlatformIO

```bash
# Compilar para Arduino UNO
pio run

# Cargar al dispositivo
pio device upload

# Abrir monitor serial
pio device monitor
```

### Configuración Requerida

- **Target**: Arduino UNO (board: uno, framework: arduino)
- **Monitor Serial**: 115200 baud
- **Comunicación XY7025**: 115200 baud (por defecto, probará otros)
- **Librería**: XY7025_Modbus (incluye ModbusMaster internamente)

## Salida Esperada

### Ejemplo de Salida por Monitor Serial

```
=== HERRAMIENTA DE DIAGNÓSTICO XY7025 - BAUDRATES ===

PROPÓSITO:
  - Verificar los baudrates almacenados en la memoria del XY7025
  - Confirmar si las posiciones 7 y 8 son realmente 2400 y 4800 baud
  - Identificar baudrates reales vs documentación del fabricante

BAUDRATES VERIFICADOS (según XY7025_Address_Finder):
  Posiciones 0-6: Valores estándar y funcionales
  Posición 7: 2400 baud (PROBLEMÁTICA según documentación)
  Posición 8: 4800 baud (PROBLEMÁTICA según documentación)

=== PRUEBA DE CONECTIVIDAD BÁSICA ===
✓ Conexión exitosa con XY7025

=== CONFIGURACIÓN ACTUAL DEL XY7025 ===
Modelo: 7025
Versión firmware: 101
Dirección esclavo: 1
Baudrate configurado: Posición 6 (115200 baud)
Voltaje salida actual: 24.56 V

=== PROBANDO TODOS LOS BAUDRATES DISPONIBLES ===
Probando posición 0 (9600 baud): ✓ FUNCIONA
Probando posición 1 (14400 baud): ✓ FUNCIONA
Probando posición 2 (19200 baud): ✓ FUNCIONA
Probando posición 3 (38400 baud): ✓ FUNCIONA
Probando posición 4 (56000 baud): ✓ FUNCIONA
Probando posición 5 (57600 baud): ✓ FUNCIONA
Probando posición 6 (115200 baud): ✓ FUNCIONA
Probando posición 7 (2400 baud): ✗ NO RESPONDE
Probando posición 8 (4800 baud): ✗ NO RESPONDE

=== RESUMEN DE BAUDRATES DISPONIBLES ===
Pos | Baudrate | Estado     | Documentado | Observación
----|----------|------------|-------------|-------------
  0 | 9600     | FUNCIONA   | Correcto    | Verificado como correcto
  1 | 14400    | FUNCIONA   | Correcto    | Verificado como correcto
  2 | 19200    | FUNCIONA   | Correcto    | Verificado como correcto
  3 | 38400    | FUNCIONA   | Correcto    | Verificado como correcto
  4 | 56000    | FUNCIONA   | Correcto    | Verificado como correcto
  5 | 57600    | FUNCIONA   | Correcto    | Verificado como correcto
  6 | 115200   | FUNCIONA   | Correcto    | Verificado como correcto
  7 | 2400     | NO FUNCIONA| Problemático| Posición 7: Debería ser 2400
  8 | 4800     | NO FUNCIONA| Problemático| Posición 8: Debería ser 4800

=== ANÁLISIS DE POSICIONES 7 y 8 (PROBLEMÁTICAS) ===
Según la documentación del fabricante, estas posiciones deberían ser:
- Posición 7: 2400 baud
- Posición 8: 4800 baud

Resultados de la prueba:
Posición 7: ✗ NO FUNCIONA - El valor 2400 baud no es reconocido
Posición 8: ✗ NO FUNCIONA - El valor 4800 baud no es reconocido

=== CONCLUSIONES ===
✗ Los baudrates documentados (2400 y 4800) NO FUNCIONAN
✗ Se necesita investigación adicional de los valores reales
```

## Interpretación de Resultados

### Resultados Típicos

1. **Posiciones 0-6**: Normalmente funcionan correctamente (baudrates estándar)
2. **Posición 7**: Frecuentemente NO funciona con 2400 baud
3. **Posición 8**: Frecuentemente NO funciona con 4800 baud

### Conclusiones Posibles

| Resultado | Implicación | Acción Recomendada |
|-----------|-------------|-------------------|
| Ambos funcionan | Documentación correcta | Usar valores documentados |
| Ninguno funciona | Documentación incorrecta | Investigar valores reales |
| Resultados mixtos | Problema específico | Análisis adicional requerido |

## Código Fuente

El código fuente está disponible en:
`examples/ReadBaudrateMemory/ReadBaudrateMemory.cpp`

### Funciones Principales

- `readCurrentBaudrateIndex()`: Lee el índice del baudrate configurado
- `testAllBaudrates()`: Prueba sistemáticamente todos los baudrates
- `readDetailedConfiguration()`: Muestra configuración actual del dispositivo
- `analyzeProblematicPositions()`: Análisis específico de posiciones 7 y 8

### Mejoras Implementadas

1. **Uso de Librería Correcta**: Implementa `XY7025_Modbus` en lugar de ModbusMaster directo
2. **Verificación Práctica**: Prueba real de comunicación en lugar de solo lectura de registros
3. **Análisis Automático**: Proporciona conclusiones basadas en resultados
4. **Manejo de Errores**: Manejo robusto de errores de comunicación

## Diferencias con la Versión Anterior

### ❌ Versión Original
- Usaba ModbusMaster directamente
- Leía registros 0x0000-0x0008 sin verificación práctica
- No probaba conectividad real
- Producía errores de comunicación frecuentes

### ✅ Versión Corregida
- Usa librería `XY7025_Modbus` verificada
- Lee registro correcto (0x0019) para índice de baudrate
- Prueba conectividad real cambiando velocidades
- Proporciona análisis automático de resultados

## Troubleshooting

### Problemas de Comunicación

1. **Error de conexión inicial**: Verificar conexiones físicas
2. **Timeout en pruebas**: Normal, algunos baudrates no funcionan
3. **Resultados inesperados**: Documentación puede ser incorrecta

### Interpretación de Resultados

1. **Pocos baudrates funcionan**: Problema de hardware o configuración
2. **Muchos baudrates funcionan**: Dispositivo muy tolerante
3. **Posiciones 7 y 8 fallan**: Confirma problema de documentación

## Próximos Pasos

Después de ejecutar este diagnóstico:

1. **Usar solo baudrates que funcionan** según los resultados
2. **Evitar posiciones 7 y 8** si no funcionan
3. **Investigar valores alternativos** para posiciones problemáticas
4. **Actualizar documentación** del proyecto con valores reales
5. **Compartir resultados** para beneficio de la comunidad

## Contribución

Si encuentras valores diferentes o comportamientos inesperados, por favor:
1. Documenta los resultados exactos obtenidos
2. Incluye información del modelo y versión del XY7025
3. Especifica las condiciones de prueba (conexiones, etc.)
4. Comparte tus hallazgos para mejorar el conocimiento común del XY7025

## Licencia

Desarrollado como parte del proyecto MPPT-XY7025.
Licencia: GNU General Public License v3.0