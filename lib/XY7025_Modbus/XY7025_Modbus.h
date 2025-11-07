#ifndef XY7025_MODBUS_H
#define XY7025_MODBUS_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>

// Constante de dirección esclavo por defecto
#define XY7025_DEFAULT_SLAVE_ADDRESS 1

// Registros de Monitoreo (0x0000-0x0023)
static const uint16_t XY7025_V_SET        = 0x0000;  // Voltaje configurado (2 decimales)
static const uint16_t XY7025_I_SET        = 0x0001;  // Corriente configurada (3 decimales)

// Lecturas en tiempo real
static const uint16_t XY7025_VOUT         = 0x0002;  // Voltaje de salida (2 decimales)
static const uint16_t XY7025_IOUT         = 0x0003;  // Corriente de salida (3 decimales)
static const uint16_t XY7025_POWER        = 0x0004;  // Potencia de salida (2 decimales)
static const uint16_t XY7025_UIN          = 0x0005;  // Voltaje de entrada (2 decimales)

// Acumuladores (32 bits, divididos en alta/baja)
static const uint16_t XY7025_AH_LOW       = 0x0006;  // Amperios-hora (parte baja)
static const uint16_t XY7025_AH_HIGH      = 0x0007;  // Amperios-hora (parte alta)
static const uint16_t XY7025_WH_LOW       = 0x0008;  // Vatios-hora (parte baja)
static const uint16_t XY7025_WH_HIGH      = 0x0009;  // Vatios-hora (parte alta)

// Tiempo de operación
static const uint16_t XY7025_OUT_H        = 0x000A;  // Horas activo
static const uint16_t XY7025_OUT_M        = 0x000B;  // Minutos activo
static const uint16_t XY7025_OUT_S        = 0x000C;  // Segundos activo

// Temperaturas
static const uint16_t XY7025_T_IN         = 0x000D;  // Temperatura interna (1 decimal)
static const uint16_t XY7025_T_EX         = 0x000E;  // Temperatura externa (1 decimal)

// Estado y control
static const uint16_t XY7025_LOCK        = 0x000F;  // Bloqueo de teclado
static const uint16_t XY7025_PROTECT     = 0x0010;  // Estado de protecciones
static const uint16_t XY7025_CVCC         = 0x0011;  // Modo de carga (0=CV, 1=CC)
static const uint16_t XY7025_ONOFF        = 0x0012;  // Estado de salida
static const uint16_t XY7025_F_C          = 0x0013;  // Escala de temperatura
static const uint16_t XY7025_B_LED        = 0x0014;  // Brillo de pantalla
static const uint16_t XY7025_SLEEP        = 0x0015;  // Tiempo de apagado de pantalla

// Información del dispositivo
static const uint16_t XY7025_MODEL        = 0x0016;  // Modelo del producto
static const uint16_t XY7025_VERSION      = 0x0017;  // Versión de firmware
static const uint16_t XY7025_SLAVE_ADD    = 0x0018;  // Dirección del esclavo
static const uint16_t XY7025_BAUDRATE_L    = 0x0019;  // Tasa de baudios

// Ajustes de temperatura
static const uint16_t XY7025_T_IN_OFFSET  = 0x001A;  // Offset temperatura interna
static const uint16_t XY7025_T_EX_OFFSET  = 0x001B;  // Offset temperatura externa

// Control adicional
static const uint16_t XY7025_BUZZER       = 0x001C;  // Estado del buzzer
static const uint16_t XY7025_EXTRACT_M    = 0x001D;  // Perfil predefinido (0-9)
static const uint16_t XY7025_DEVICE       = 0x001E;  // Estado del dispositivo
static const uint16_t XY7025_MPPT_SW      = 0x001F;  // Habilitar MPPT
static const uint16_t XY7025_MPPT_K      = 0x0020;  // Factor de calibración MPPT
static const uint16_t XY7025_BATFUL      = 0x0021;  // Corriente fin de carga
static const uint16_t XY7025_CW_SW       = 0x0022;  // Habilitar potencia constante
static const uint16_t XY7025_CW          = 0x0023;  // Potencia constante objetivo

// Registros de Perfiles (0x0050-0x00ED)
// Perfil M0 (registros 0x0050-0x005D)
static const uint16_t XY7025_M0_V_SET     = 0x0050;  // Voltaje objetivo M0
static const uint16_t XY7025_M0_I_SET     = 0x0051;  // Corriente objetivo M0
static const uint16_t XY7025_M0_S_LVP     = 0x0052;  // Protección subtensión M0
static const uint16_t XY7025_M0_S_OVP     = 0x0053;  // Protección sobretensión M0
static const uint16_t XY7025_M0_S_OCP     = 0x0054;  // Protección sobrecorriente M0
static const uint16_t XY7025_M0_S_OPP     = 0x0055;  // Protección sobrepotencia M0
static const uint16_t XY7025_M0_S_OHP_H   = 0x0056;  // Límite tiempo horas M0
static const uint16_t XY7025_M0_S_OHP_M   = 0x0057;  // Límite tiempo minutos M0
static const uint16_t XY7025_M0_S_OAH_L   = 0x0058;  // Contador AH bajo M0
static const uint16_t XY7025_M0_S_OAH_H   = 0x0059;  // Contador AH alto M0
static const uint16_t XY7025_M0_S_OWH_L   = 0x005A;  // Contador WH bajo M0
static const uint16_t XY7025_M0_S_OWH_H   = 0x005B;  // Contador WH alto M0
static const uint16_t XY7025_M0_S_OTP     = 0x005C;  // Temperatura máxima M0
static const uint16_t XY7025_M0_S_INI     = 0x005D;  // Estado inicial M0

// Fórmula para perfiles M1-M9: 0x0050 + (perfil * 0x0010) + offset
#define XY7025_PROFILE_REGISTER(perfil, offset) (0x0050 + (perfil * 0x0010) + offset)

// Códigos de error estándar de ModbusMaster
const uint8_t XY7025_SUCCESS           = 0x00;  // Éxito
const uint8_t XY7025_INVALID_CRC       = 0xE2;  // CRC inválido
const uint8_t XY7025_RESPONSE_TIMEOUT  = 0xE1;  // Timeout
const uint8_t XY7025_ILLEGAL_FUNCTION   = 0x01;  // Función no soportada
const uint8_t XY7025_ILLEGAL_ADDRESS    = 0x02;  // Dirección inválida
const uint8_t XY7025_ILLEGAL_VALUE      = 0x03;  // Valor inválido

// Constantes para indicar errores en valores de retorno
const float XY7025_ERROR_FLOAT          = NAN;      // Error para valores float (Not a Number)
const uint32_t XY7025_ERROR_UINT32      = 0xFFFFFFFF; // Error para valores uint32 (máximo valor)
const int16_t XY7025_ERROR_INT16        = -32768;   // Error para valores int16 (mínimo valor)
const uint16_t XY7025_ERROR_UINT16      = 0xFFFF;   // Error para valores uint16 (máximo valor)
const uint8_t XY7025_ERROR_UINT8        = 0xFF;     // Error para valores uint8 (máximo valor)
const bool XY7025_ERROR_BOOL            = false;    // Error para valores booleanos

// Límites de validación para conversiones
const float XY7025_MAX_VOLTAGE          = 100.0;    // Voltaje máximo permitido (V)
const float XY7025_MAX_CURRENT          = 30.0;     // Corriente máxima permitida (A)
const float XY7025_MAX_POWER            = 1000.0;   // Potencia máxima permitida (W)
const float XY7025_MIN_TEMPERATURE      = -273.15;  // Temperatura mínima permitida (°C)
const float XY7025_MAX_TEMPERATURE      = 200.0;    // Temperatura máxima permitida (°C)

// Estados de Protección
const uint8_t PROTECT_NORMAL            = 0;   // Funcionamiento normal
const uint8_t PROTECT_OVP                = 1;   // Sobretensión
const uint8_t PROTECT_OCP                = 2;   // Sobrecorriente
const uint8_t PROTECT_OPP                = 3;   // Sobrepotencia
const uint8_t PROTECT_LVP                = 4;   // Subtensión
const uint8_t PROTECT_OAH                = 5;   // Límite capacidad AH
const uint8_t PROTECT_OHP                = 6;   // Límite tiempo operación
const uint8_t PROTECT_OTP                = 7;   // Sobretemperatura
const uint8_t PROTECT_OEP                = 8;   // Sin salida
const uint8_t PROTECT_OWH                = 9;   // Límite energía WH
const uint8_t PROTECT_ICP                = 10;  // Límite corriente entrada
const uint8_t PROTECT_ETP                = 11;  // Temperatura externa

// Clase auxiliar para conversiones de datos
class DataConverter {
public:
    // Conversión de raw a valores reales
    static float toVoltage(uint16_t raw) {
        return raw / 100.0;      // 2 decimales
    }
    
    static float toCurrent(uint16_t raw) {
        return raw / 1000.0;     // 3 decimales
    }
    
    static float toPower(uint16_t raw) {
        return raw / 100.0;      // 2 decimales
    }
    
    static float toTemperature(uint16_t raw) {
        return raw / 10.0;       // 1 decimal
    }
    
    // Conversión de valores reales a raw con validación de rangos
    static uint16_t fromVoltage(float voltage) {
        // Validar rango para prevenir desbordamiento de uint16_t
        if (voltage < 0.0 || voltage > XY7025_MAX_VOLTAGE) {
            return XY7025_ERROR_UINT16; // Retornar valor de error si fuera de rango
        }
        return (uint16_t)(voltage * 100);
    }
    
    static uint16_t fromCurrent(float current) {
        // Validar rango para prevenir desbordamiento de uint16_t
        // Máximo valor: 65535 / 1000 = 65.535 A
        if (current < 0.0 || current > XY7025_MAX_CURRENT) {
            return XY7025_ERROR_UINT16; // Retornar valor de error si fuera de rango
        }
        return (uint16_t)(current * 1000);
    }
    
    static uint16_t fromPower(float power) {
        // Validar rango para prevenir desbordamiento de uint16_t
        // Máximo valor: 65535 / 100 = 655.35 W
        if (power < 0.0 || power > XY7025_MAX_POWER) {
            return XY7025_ERROR_UINT16; // Retornar valor de error si fuera de rango
        }
        return (uint16_t)(power * 100);
    }
    
    // Combinación de registros de 32 bits
    static uint32_t combine32(uint16_t high, uint16_t low) {
        return ((uint32_t)high << 16) | low;
    }
};

// Clase principal XY7025_Modbus
class XY7025_Modbus {
private:
    SoftwareSerial* serial;
    ModbusMaster modbus;
    uint8_t slaveAddress;
    uint16_t timeout;
    uint8_t retries;
    bool debugMode;
    
    // Métodos auxiliares (públicos para permitir acceso directo desde main.cpp)
    bool readHoldingRegisters(uint16_t address, uint8_t count);
    uint16_t getResponseBuffer(uint8_t index);
    
public:
    // Constructor
    XY7025_Modbus(SoftwareSerial& serial, uint8_t address = XY7025_DEFAULT_SLAVE_ADDRESS);
    
    // Configuración
    bool begin(uint32_t baudRate = 115200);
    void setTimeout(uint16_t timeout);
    void setRetries(uint8_t retries);
    void enableDebug(bool enable);
    
    // Lectura de todos los registros
    bool readAllRegisters(Print& output);
    
    // Lectura y escritura de registros genéricos
    bool writeRegister(uint16_t address, uint16_t value);
    bool writeMultipleRegisters(uint16_t address, uint16_t* values, uint8_t count);
    
    // Función de prueba de dirección
    bool probeSlaveAddress(uint8_t address);
    
    // Funciones de lectura específicas
    float readVoltageOutput();
    float readCurrentOutput();
    float readPowerOutput();
    float readVoltageInput();
    uint32_t readAmpHours();
    uint32_t readWattHours();
    float readTemperatureInternal();
    float readTemperatureExternal();
    
    // Funciones de configuración
    bool setOutputVoltage(float voltage);
    bool setOutputCurrent(float current);
    bool setOutputPower(float power);
    bool enableOutput(bool enable);
    bool setProfile(uint8_t profile);
    
    // Funciones de lectura de registros específicos
    uint16_t readRegister(uint16_t address);
    uint16_t readModel();
    uint16_t readVersion();
    uint16_t readBaudrate();
    uint16_t readOutputState();
    uint16_t readChargeMode();
    
    // Funciones de estado y protección
    uint8_t getProtectionStatus();
    String getProtectionDescription(uint8_t status);
};

#endif // XY7025_MODBUS_H