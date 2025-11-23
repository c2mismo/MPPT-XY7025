# Nueva Estructura de Perfiles XY7025

## Descripción

La biblioteca XY7025_Modbus ha sido reestructurada para proporcionar un acceso más claro y mantenible a los perfiles de memoria M0-M9 del dispositivo XY7025.

## Cambios Principales

### 1. Estructura de Offsets Unificada

Se ha creado un enum `ProfileOffset` que define los offsets dentro de cada perfil:

```cpp
enum ProfileOffset {
    PROFILE_V_SET    = 0x00,  // Voltaje objetivo
    PROFILE_I_SET    = 0x01,  // Corriente objetivo
    PROFILE_S_LVP    = 0x02,  // Protección subtensión
    PROFILE_S_OVP    = 0x03,  // Protección sobretensión
    PROFILE_S_OCP    = 0x04,  // Protección sobrecorriente
    PROFILE_S_OPP    = 0x05,  // Protección sobrepotencia
    PROFILE_S_OHP_H  = 0x06,  // Protección tiempo horas
    PROFILE_S_OHP_M  = 0x07,  // Protección tiempo minutos
    PROFILE_S_OAH_L  = 0x08,  // Contador AH bajo
    PROFILE_S_OAH_H  = 0x09,  // Contador AH alto
    PROFILE_S_OWH_L  = 0x0A,  // Contador WH bajo
    PROFILE_S_OWH_H  = 0x0B,  // Contador WH alto
    PROFILE_S_OTP    = 0x0C,  // Protección temperatura interna
    PROFILE_S_ETP    = 0x0D   // Protección temperatura externa (Batería)
};
```

### 2. Métodos Genéricos

Se han añadido dos métodos genéricos para acceder a cualquier perfil:

```cpp
// Leer un registro de perfil
uint16_t readProfileReg(uint8_t profile, ProfileOffset offset);

// Escribir un registro de perfil
bool writeProfileReg(uint8_t profile, ProfileOffset offset, uint16_t value);
```

### 3. Fórmula de Direccionamiento

La fórmula para calcular la dirección de cualquier registro de perfil es:

```cpp
XY7025_PROFILE_REGISTER(perfil, offset) = 0x0050 + (perfil * 0x0010) + offset
```

Donde:
- `perfil`: número de perfil (0-9)
- `offset`: offset dentro del perfil (0x00-0x0D)

## Ejemplos de Uso

### Leer configuración de un perfil

```cpp
// Leer voltaje objetivo del perfil M3
uint16_t voltaje = mppt.readProfileReg(3, PROFILE_V_SET);
float voltajeV = voltaje / 100.0; // Convertir a voltios
```

### Configurar un perfil

```cpp
// Configurar perfil M5 con 24V y 5A
mppt.writeProfileReg(5, PROFILE_V_SET, 2400);  // 24.00V
mppt.writeProfileReg(5, PROFILE_I_SET, 5000);  // 5.000A
```

### Configurar protecciones

```cpp
// Configurar protecciones para perfil M2
mppt.writeProfileReg(2, PROFILE_S_LVP, 2200);  // LVP 22.00V
mppt.writeProfileReg(2, PROFILE_S_OVP, 2800);  // OVP 28.00V
mppt.writeProfileReg(2, PROFILE_S_OCP, 6000);  // OCP 6.000A
```

## Ventajas de la Nueva Estructura

1. **Escalabilidad**: Fácil añadir nuevos offsets si se descubren más registros
2. **Mantenibilidad**: Código más limpio y menos repetitivo
3. **Flexibilidad**: Un solo método para acceder a cualquier perfil
4. **Validación**: Los métodos incluyen validación de rangos automática
5. **Debug**: Modo debug muestra información detallada de las operaciones

## Compatibilidad

La nueva estructura es completamente compatible con el código existente. Los métodos antiguos siguen funcionando, pero ahora se recomienda usar los nuevos métodos genéricos para un código más limpio y mantenible.

## Archivos Modificados

- `lib/XY7025_Modbus/XY7025_Modbus.h`: Nueva estructura de perfiles
- `lib/XY7025_Modbus/XY7025_Modbus.cpp`: Implementación de métodos genéricos
- `examples/Profile_Example/Profile_Example.ino`: Ejemplo completo de uso

## Próximos Pasos

- Crear más ejemplos específicos para diferentes casos de uso
- Añadir métodos de conveniencia para operaciones comunes
- Documentar mejoras de rendimiento y uso de memoria