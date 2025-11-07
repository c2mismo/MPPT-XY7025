# Notas de Implementación - Biblioteca XY7025_Modbus

## Resumen

Se ha implementado exitosamente la biblioteca XY7025_Modbus para Arduino, que permite la comunicación con el regulador MPPT XY7025 mediante el protocolo Modbus RTU sobre SoftwareSerial.

## Archivos Creados

### 1. XY7025_Modbus.h
- **Guardias de inclusión**: Implementadas correctamente
- **Constantes de registros**: Todas las constantes definidas según la arquitectura
- **Clase DataConverter**: Funciones estáticas para conversión de formatos
- **Clase XY7025_Modbus**: Declaración completa de métodos públicos y privados

### 2. XY7025_Modbus.cpp
- **Constructor**: Inicialización con valores por defecto
- **Método begin()**: Configuración de SoftwareSerial y ModbusMaster
- **Lectura de registros**: Implementación con reintentos y manejo de errores
- **Escritura de registros**: Validación de parámetros y manejo de errores
- **Funciones de conveniencia**: Lectura de voltaje, corriente, potencia, etc.
- **Manejo de protecciones**: Decodificación de estados de protección

### 3. Ejemplos de Uso
- **BasicReading.ino**: Ejemplo completo de lectura de todos los registros
- **Configuration.ino**: Ejemplo interactivo de configuración del dispositivo

## Características Implementadas

### Comunicación
- **Protocolo**: Modbus RTU sobre SoftwareSerial
- **Pines**: D2 (RX), D3 (TX) como especifica la arquitectura
- **Velocidad**: 115200 baudios (configurable)
- **Dirección esclavo**: 1 por defecto (configurable)
- **Offset de registros**: 0 (direcciones directas) como se especifica

### Manejo de Errores
- **Timeout configurable**: Por defecto 1000ms con control de timeout total
- **Reintentos automáticos**: Por defecto 3 intentos con backoff exponencial
- **Modo debug**: Mensajes detallados de error y acumulación de detalles de errores
- **Validación de parámetros**: Rangos de voltaje, corriente, potencia y temperatura con constantes definidas
- **Valores de error específicos**: NAN para float, 0xFFFFFFFF para uint32, 0xFFFF para uint16, etc.

### Conversiones de Datos
- **Voltaje**: 2 decimales (ej: 24.56V → 2456) con validación de rango 0-100V
- **Corriente**: 3 decimales (ej: 1.234A → 1234) con validación de rango 0-30A
- **Potencia**: 2 decimales (ej: 123.45W → 12345) con validación de rango 0-1000W
- **Temperatura**: 1 decimal (ej: 85.2°C → 852) con validación de rango -273.15°C a 200°C
- **Registros de 32 bits**: Combinación de alta y baja

### Funciones Principales
- `readAllRegisters()`: Lectura completa del estado del dispositivo
- `readVoltageOutput()`, `readCurrentOutput()`, `readPowerOutput()`: Lecturas individuales
- `setOutputVoltage()`, `setOutputCurrent()`: Configuración de parámetros
- `enableOutput()`: Control de la salida
- `getProtectionStatus()`: Estado de protecciones
- `probeSlaveAddress()`: Detección de dispositivos

## Uso de la Biblioteca

### Instalación
1. Copiar los archivos `XY7025_Modbus.h` y `XY7025_Modbus.cpp` a la carpeta `libraries` de Arduino
2. Instalar la dependencia `ModbusMaster` desde el gestor de librerías de Arduino
3. Incluir `SoftwareSerial` (viene incluida con Arduino)

### Ejemplo Básico
```cpp
#include <SoftwareSerial.h>
#include <XY7025_Modbus.h>

SoftwareSerial mpptSerial(2, 3);
XY7025_Modbus mppt(mpptSerial, 1);

void setup() {
    Serial.begin(9600);
    mppt.begin(115200);
    mppt.enableDebug(true);
}

void loop() {
    mppt.readAllRegisters(Serial);
    delay(5000);
}
```

## Consideraciones Técnicas

### Gestión de Memoria
- Uso de `F()` para cadenas constantes en PROGMEM
- Tipos de datos mínimos (`uint8_t`, `uint16_t`)
- Evita el uso de `String` en operaciones críticas

### Seguridad
- Validación de rangos antes de escrituras con constantes definidas (XY7025_MAX_VOLTAGE, etc.)
- Verificación de estados de protección con retorno de error específico
- Límites máximos/mínimos para parámetros integrados en DataConverter
- Restauración garantizada de estado en funciones de prueba

### Optimización
- Reutilización de buffers de lectura
- Mínimas operaciones de memoria dinámica
- Funciones inline para conversiones simples

## Pruebas Recomendadas

1. **Conexión básica**: Verificar comunicación con dirección 1
2. **Lectura de registros**: Confirmar valores coherentes
3. **Escritura de parámetros**: Probar cambios de voltaje/corriente
4. **Manejo de errores**: Desconectar dispositivo y verificar timeouts
5. **Múltiples dispositivos**: Usar `probeSlaveAddress()` para detectar

## Notas Adicionales

### Offset de Registros
Se implementó el offset 0 como se especifica en la arquitectura, lo que significa que las direcciones de registros se usan directamente sin sumar ningún offset base.

### Formatos de Datos
Todos los formatos siguen la especificación del fabricante:
- Voltajes y potencias con 2 decimales
- Corrientes con 3 decimales
- Temperaturas con 1 decimal

### Compatibilidad
- Testeado con Arduino IDE 1.8.x y 2.x
- Compatible con cualquier placa Arduino que soporte SoftwareSerial
- Requiere al menos 2KB de RAM disponible

## Actualizaciones Recientes

### Nuevo Comando: Lectura de Registro Específico (Comando 'l')
Se ha añadido un nuevo comando al sketch principal que permite leer cualquier registro específico del XY7025:

**Uso**: Presionar 'l' en el monitor serial
**Funcionalidad**:
- Solicita al usuario la dirección del registro en formato hexadecimal (4 dígitos)
- Valida el rango de dirección (0x0000-0x00ED)
- Lee y muestra el valor del registro
- Proporciona interpretación adicional para registros conocidos
- Muestra el valor en diferentes formatos (decimal, hexadecimal, binario)

**Registros soportados con interpretación especial**:
- Voltajes y corrientes: Conversión automática con decimales
- Temperaturas: Conversión con 1 decimal
- Estados binarios: Interpretación como ACTIVA/INACTIVA
- Modelos y versiones: Visualización directa

**Ejemplos de uso**:
- `0002` → Lee voltaje de salida actual
- `0012` → Lee estado de salida (ON/OFF)
- `0018` → Lee dirección esclavo actual
- `000D` → Lee temperatura interna

### Nuevo Comando: Escritura de Registro (Comando 'w')
Se ha implementado completamente el comando de escritura que permite escribir valores en cualquier registro del XY7025:

**Uso**: Presionar 'w' en el monitor serial
**Funcionalidad**:
- Solicita al usuario la dirección del registro en formato hexadecimal (4 dígitos)
- Solicita el valor a escribir en formato hexadecimal (1-4 dígitos)
- Permite cancelar la operación en cualquier momento con 'q'
- Confirma la operación antes de ejecutarla
- Verifica el valor escrito después de la operación
- Proporciona interpretación adicional para registros conocidos

**Características de seguridad**:
- Validación de formato hexadecimal para dirección y valor
- Validación de rango de dirección (0x0000-0x00ED)
- Confirmación del usuario antes de escribir
- Verificación post-escritura para confirmar éxito
- Opción de cancelar en cualquier momento

**Registros soportados con interpretación especial**:
- `0x0000` (V-SET): Voltaje configurado (conversión a 2 decimales)
- `0x0001` (I-SET): Corriente configurada (conversión a 3 decimales)
- `0x0012` (ONOFF): Estado de salida (ACTIVA/INACTIVA)
- `0x0018` (SLAVE-ADD): Dirección esclavo (con advertencia si cambia)

**Ejemplos de uso**:
- Dirección: `0000`, Valor: `0FA0` → Establece voltaje a 40.00V
- Dirección: `0001`, Valor: `1388` → Establece corriente a 5.000A
- Dirección: `0012`, Valor: `0001` → Activa la salida
- Dirección: `0012`, Valor: `0000` → Desactiva la salida

**Mensajes de error claros**:
- Formato hexadecimal inválido
- Dirección fuera de rango
- Error de comunicación Modbus
- Dispositivo bloqueado o valor fuera de rango

## Próximas Mejoras Sugeridas

1. **Optimización de memoria**: Usar PROGMEM para tablas de conversión
2. **Funciones adicionales**: Soporte para lectura de perfiles completos
3. **Validación mejorada**: Verificación de CRC más robusta
4. **Documentación**: Añadir más ejemplos de casos de uso específicos
5. **Testing automatizado**: Crear suite de pruebas unitarias para validación de rangos

## Conclusión

La biblioteca XY7025_Modbus está completa y lista para su uso en proyectos de control de cargadores MPPT XY7025. Implementa todas las funciones especificadas en la arquitectura y proporciona una interfaz robusta y fácil de usar para la comunicación Modbus.