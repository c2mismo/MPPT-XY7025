# Ejemplo ReadWrite - Lectura y Escritura Completa XY7025

## Descripción
Este ejemplo demuestra el uso completo de la librería XY7025_Modbus con todas las funciones de lectura y escritura disponibles, incluyendo:
- Lectura de registros individuales
- Lectura de registros de perfiles M0-M9
- Escritura de registros individuales
- Escritura de registros de perfiles
- Lectura de múltiples registros consecutivos
- Lectura con visualización en formato hexadecimal
- Escritura con valores en formato hexadecimal
- Menú interactivo por consola serial

## Características
- **Baudrate**: 115200 para ambos puertos seriales (USB y SoftwareSerial)
- **Dirección esclavo**: 1 (configurable)
- **Menú interactivo**: Acceso a todas las funciones mediante comandos
- **Visualización hexadecimal**: Lectura y escritura de valores en formato hex
- **Gestión de errores**: Manejo completo de errores de comunicación Modbus

## Hardware Requerido
- Arduino UNO/Nano o ESP32/ESP8266
- MPPT XY7025 con comunicación Modbus RS485
- Convertidor RS485 a TTL
- Pines de conexión:
  - RX: Pin 10
  - TX: Pin 11
  - RE/DE: Pin 12 (control de dirección)

## Estructura del Menú
```
=== MENÚ PRINCIPAL XY7025 ===
1. Leer registro individual
2. Leer registro de perfil
3. Escribir registro individual
4. Escribir registro de perfil
5. Leer múltiples registros
6. Leer registro con valor hex
7. Escribir registro con valor hex
8. Leer todos los registros
9. Ayuda (help)
0. Salir
```

## Funciones Implementadas

### 1. Lectura de Registro Individual
```cpp
uint16_t readSingleRegister(uint16_t address)
```
- Lee un registro específico por su dirección
- Retorna el valor crudo o 0xFFFF en caso de error

### 2. Lectura de Registro de Perfil
```cpp
uint16_t readProfileRegister(uint8_t profile, ProfileOffset offset)
```
- Lee un registro dentro de un perfil M0-M9
- Utiliza la fórmula de cálculo de direcciones de perfiles

### 3. Escritura de Registro Individual
```cpp
bool writeSingleRegister(uint16_t address, uint16_t value)
```
- Escribe un valor en un registro específico
- Retorna true/false según el resultado

### 4. Escritura de Registro de Perfil
```cpp
bool writeProfileRegister(uint8_t profile, ProfileOffset offset, uint16_t value)
```
- Escribe un valor en un registro de perfil M0-M9
- Valida el rango del perfil (0-9)

### 5. Lectura de Múltiples Registros
```cpp
bool readMultipleRegisters(uint16_t startAddress, uint8_t count)
```
- Lee varios registros consecutivos
- Muestra los valores en formato decimal y hexadecimal

### 6. Lectura con Visualización Hexadecimal
```cpp
void readRegisterWithHex(uint16_t address)
```
- Lee un registro y muestra su valor en ambos formatos
- Útil para depuración y análisis de datos

### 7. Escritura con Valor Hexadecimal
```cpp
bool writeRegisterWithHex(uint16_t address, String hexValue)
```
- Escribe un valor hexadecimal en un registro
- Convierte el string hex a uint16_t antes de escribir

## Comandos del Menú

### Comando 1: Leer registro
```
Ingrese dirección del registro (HEX): 0002
Leyendo registro 0x0002...
Valor: 1234 (0x04D2)
```

### Comando 2: Leer registro de perfil
```
Ingrese número de perfil (0-9): 0
Ingrese offset del registro (0-13): 0
Leyendo perfil M0, offset 0x00...
Valor: 5000 (0x1388)
```

### Comando 3: Escribir registro
```
Ingrese dirección del registro (HEX): 0000
Ingrese valor a escribir: 1234
Escribiendo 1234 en registro 0x0000...
Resultado: ÉXITO
```

### Comando 4: Escribir registro de perfil
```
Ingrese número de perfil (0-9): 1
Ingrese offset del registro (0-13): 1
Ingrese valor a escribir: 2500
Escribiendo 2500 en perfil M1, offset 0x01...
Resultado: ÉXITO
```

### Comando 5: Leer múltiples registros
```
Ingrese dirección inicial (HEX): 0000
Ingrese cantidad de registros (1-10): 5
Registros desde 0x0000:
[0x0000]: 1234 (0x04D2)
[0x0001]: 0567 (0x0237)
[0x0002]: 0890 (0x037A)
[0x0003]: 0123 (0x007B)
[0x0004]: 0456 (0x01C8)
```

### Comando 6: Leer con formato hex
```
Ingrese dirección del registro (HEX): 0010
Leyendo registro 0x0010...
Valor: 0 (0x0000)
```

### Comando 7: Escribir con formato hex
```
Ingrese dirección del registro (HEX): 0000
Ingrese valor hexadecimal (4 dígitos): 04D2
Escribiendo 0x04D2 en registro 0x0000...
Resultado: ÉXITO
```

## Constantes de Registros Importantes

### Registros de Monitoreo (0x0000-0x0023)
- `XY7025_V_SET` (0x0000): Voltaje configurado
- `XY7025_I_SET` (0x0001): Corriente configurada
- `XY7025_VOUT` (0x0002): Voltaje de salida
- `XY7025_IOUT` (0x0003): Corriente de salida
- `XY7025_POWER` (0x0004): Potencia de salida

### Registros de Perfiles (0x0050-0x00ED)
- Perfiles M0-M9 disponibles
- Cada perfil tiene 14 registros (0x00-0x0D)
- Offset: V_SET, I_SET, S_LVP, S_OVP, S_OCP, S_OPP, etc.

## Gestión de Errores
El ejemplo incluye manejo completo de errores:
- Timeouts de comunicación
- CRC inválidos
- Direcciones de registro inválidas
- Rangos de perfiles incorrectos
- Valores fuera de rango

## Ejemplo de Uso Básico
```cpp
#include <XY7025_Modbus.h>
#include <SoftwareSerial.h>

SoftwareSerial modbusSerial(10, 11); // RX, TX
XY7025_Modbus xy7025(modbusSerial, 1); // Slave address 1

void setup() {
  Serial.begin(115200);
  modbusSerial.begin(115200);
  xy7025.begin(115200);
  
  // Leer voltaje de salida
  float voltage = xy7025.readVoltageOutput();
  Serial.print("Voltaje: "); Serial.println(voltage);
}
```

## Notas de Implementación
- Utiliza SoftwareSerial para la comunicación Modbus
- Implementa reintentos automáticos en caso de error
- Incluye modo debug para depuración
- Valida todos los rangos de entrada
- Muestra resultados en formatos decimal y hexadecimal