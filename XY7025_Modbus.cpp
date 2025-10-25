#include "XY7025_Modbus.h"

// Constructor
XY7025_Modbus::XY7025_Modbus(SoftwareSerial& serial, uint8_t address) {
    this->serial = &serial;
    this->slaveAddress = address;
    this->timeout = 1000;     // 1 segundo por defecto
    this->retries = 3;        // 3 reintentos por defecto
    this->debugMode = false;
}

// Configuración inicial
bool XY7025_Modbus::begin(uint32_t baudRate) {
    serial->begin(baudRate);
    node.begin(slaveAddress, *serial);
    node.setTimeout(timeout);
    
    if (debugMode) {
        Serial.println(F("XY7025_Modbus: Inicializado con éxito"));
    }
    return true;
}

// Configurar timeout
void XY7025_Modbus::setTimeout(uint16_t timeout) {
    this->timeout = timeout;
    node.setTimeout(timeout);
}

// Configurar reintentos
void XY7025_Modbus::setRetries(uint8_t retries) {
    this->retries = retries;
}

// Habilitar/deshabilitar modo debug
void XY7025_Modbus::enableDebug(bool enable) {
    this->debugMode = enable;
}

// Método privado para leer registros con manejo de errores
bool XY7025_Modbus::readHoldingRegisters(uint16_t address, uint8_t count) {
    uint8_t result;
    uint8_t attempt = 0;
    
    do {
        result = node.readHoldingRegisters(address, count);
        attempt++;
        
        if (result == node.ku8MBSuccess) {
            return true;
        }
        
        if (debugMode) {
            Serial.print(F("XY7025_Modbus: Error leyendo registros 0x"));
            Serial.print(address, HEX);
            Serial.print(F(" (intento "));
            Serial.print(attempt);
            Serial.print(F("): 0x"));
            Serial.println(result, HEX);
        }
        
        delay(100); // Pequeño delay entre reintentos
        
    } while (attempt < retries);
    
    return false;
}

// Obtener valor del buffer de respuesta
uint16_t XY7025_Modbus::getResponseBuffer(uint8_t index) {
    return node.getResponseBuffer(index);
}

// Leer todos los registros principales
bool XY7025_Modbus::readAllRegisters(Print& output) {
    // Leer registros de estado básico (0x0000-0x0005)
    if (readHoldingRegisters(0x0000, 6)) {
        float voltajeConfig = DataConverter::toVoltage(getResponseBuffer(0));
        float corrienteConfig = DataConverter::toCurrent(getResponseBuffer(1));
        float voltajeSalida = DataConverter::toVoltage(getResponseBuffer(2));
        float corrienteSalida = DataConverter::toCurrent(getResponseBuffer(3));
        float potenciaSalida = DataConverter::toPower(getResponseBuffer(4));
        float voltajeEntrada = DataConverter::toVoltage(getResponseBuffer(5));
        
        output.println(F("=== Estado del MPPT XY7025 ==="));
        output.print(F("Voltaje Configurado: ")); output.print(voltajeConfig); output.println(F("V"));
        output.print(F("Corriente Configurada: ")); output.print(corrienteConfig); output.println(F("A"));
        output.print(F("Voltaje de Salida: ")); output.print(voltajeSalida); output.println(F("V"));
        output.print(F("Corriente de Salida: ")); output.print(corrienteSalida); output.println(F("A"));
        output.print(F("Potencia de Salida: ")); output.print(potenciaSalida); output.println(F("W"));
        output.print(F("Voltaje de Entrada: ")); output.print(voltajeEntrada); output.println(F("V"));
        
        // Leer acumuladores (32 bits)
        if (readHoldingRegisters(XY7025_AH_LOW, 2)) {
            uint32_t ampHours = DataConverter::combine32(getResponseBuffer(1), getResponseBuffer(0));
            output.print(F("Amperios-hora: ")); output.print(ampHours / 1000.0); output.println(F("Ah"));
        }
        
        if (readHoldingRegisters(XY7025_WH_LOW, 2)) {
            uint32_t wattHours = DataConverter::combine32(getResponseBuffer(1), getResponseBuffer(0));
            output.print(F("Vatios-hora: ")); output.print(wattHours / 100.0); output.println(F("Wh"));
        }
        
        // Leer temperaturas
        if (readHoldingRegisters(XY7025_T_IN, 2)) {
            float tempInt = DataConverter::toTemperature(getResponseBuffer(0));
            float tempExt = DataConverter::toTemperature(getResponseBuffer(1));
            output.print(F("Temp. Interna: ")); output.print(tempInt); output.println(F("°C"));
            output.print(F("Temp. Externa: ")); output.print(tempExt); output.println(F("°C"));
        }
        
        // Leer estado de protección
        uint8_t protectStatus = getProtectionStatus();
        output.print(F("Estado de Protección: ")); 
        output.println(getProtectionDescription(protectStatus));
        
        return true;
    }
    
    output.println(F("Error: No se pudieron leer los registros"));
    return false;
}

// Escribir un solo registro
bool XY7025_Modbus::writeRegister(uint16_t address, uint16_t value) {
    uint8_t result = node.writeSingleRegister(address, value);
    
    if (result == node.ku8MBSuccess) {
        if (debugMode) {
            Serial.print(F("XY7025_Modbus: Registro 0x"));
            Serial.print(address, HEX);
            Serial.print(F(" escrito con valor: "));
            Serial.println(value);
        }
        return true;
    } else {
        if (debugMode) {
            Serial.print(F("XY7025_Modbus: Error escribiendo registro 0x"));
            Serial.print(address, HEX);
            Serial.print(F(": 0x"));
            Serial.println(result, HEX);
        }
        return false;
    }
}

// Escribir múltiples registros
bool XY7025_Modbus::writeMultipleRegisters(uint16_t address, uint16_t* values, uint8_t count) {
    uint8_t result = node.writeMultipleRegisters(address, values, count);
    
    if (result == node.ku8MBSuccess) {
        if (debugMode) {
            Serial.print(F("XY7025_Modbus: "));
            Serial.print((int)count);
            Serial.print(F(" registros escritos desde 0x"));
            Serial.println(address, HEX);
        }
        return true;
    } else {
        if (debugMode) {
            Serial.print(F("XY7025_Modbus: Error escribiendo múltiples registros desde 0x"));
            Serial.print(address, HEX);
            Serial.print(F(": 0x"));
            Serial.println(result, HEX);
        }
        return false;
    }
}

// Probar dirección de esclavo
bool XY7025_Modbus::probeSlaveAddress(uint8_t address) {
    uint8_t originalAddress = slaveAddress;
    slaveAddress = address;
    node.begin(slaveAddress, *serial);
    
    // Intentar leer un registro conocido (voltaje de salida)
    bool success = readHoldingRegisters(XY7025_VOUT, 1);
    
    // Restaurar dirección original
    slaveAddress = originalAddress;
    node.begin(slaveAddress, *serial);
    
    if (debugMode) {
        Serial.print(F("XY7025_Modbus: Prueba de dirección "));
        Serial.print(address);
        Serial.print(F(": "));
        Serial.println(success ? F("ÉXITO") : F("FALLO"));
    }
    
    return success;
}

// Funciones de lectura específicas
float XY7025_Modbus::readVoltageOutput() {
    if (readHoldingRegisters(XY7025_VOUT, 1)) {
        return DataConverter::toVoltage(getResponseBuffer(0));
    }
    return -1.0; // Valor de error
}

float XY7025_Modbus::readCurrentOutput() {
    if (readHoldingRegisters(XY7025_IOUT, 1)) {
        return DataConverter::toCurrent(getResponseBuffer(0));
    }
    return -1.0; // Valor de error
}

float XY7025_Modbus::readPowerOutput() {
    if (readHoldingRegisters(XY7025_POWER, 1)) {
        return DataConverter::toPower(getResponseBuffer(0));
    }
    return -1.0; // Valor de error
}

float XY7025_Modbus::readVoltageInput() {
    if (readHoldingRegisters(XY7025_UIN, 1)) {
        return DataConverter::toVoltage(getResponseBuffer(0));
    }
    return -1.0; // Valor de error
}

uint32_t XY7025_Modbus::readAmpHours() {
    if (readHoldingRegisters(XY7025_AH_LOW, 2)) {
        return DataConverter::combine32(getResponseBuffer(1), getResponseBuffer(0));
    }
    return 0; // Valor de error
}

uint32_t XY7025_Modbus::readWattHours() {
    if (readHoldingRegisters(XY7025_WH_LOW, 2)) {
        return DataConverter::combine32(getResponseBuffer(1), getResponseBuffer(0));
    }
    return 0; // Valor de error
}

float XY7025_Modbus::readTemperatureInternal() {
    if (readHoldingRegisters(XY7025_T_IN, 1)) {
        return DataConverter::toTemperature(getResponseBuffer(0));
    }
    return -999.0; // Valor de error
}

float XY7025_Modbus::readTemperatureExternal() {
    if (readHoldingRegisters(XY7025_T_EX, 1)) {
        return DataConverter::toTemperature(getResponseBuffer(0));
    }
    return -999.0; // Valor de error
}

// Funciones de configuración
bool XY7025_Modbus::setOutputVoltage(float voltage) {
    // Validar rango de voltaje (0-100V típicamente)
    if (voltage < 0.0 || voltage > 100.0) {
        if (debugMode) {
            Serial.println(F("XY7025_Modbus: Voltaje fuera de rango"));
        }
        return false;
    }
    
    uint16_t value = DataConverter::fromVoltage(voltage);
    return writeRegister(XY7025_V_SET, value);
}

bool XY7025_Modbus::setOutputCurrent(float current) {
    // Validar rango de corriente (0-30A típicamente)
    if (current < 0.0 || current > 30.0) {
        if (debugMode) {
            Serial.println(F("XY7025_Modbus: Corriente fuera de rango"));
        }
        return false;
    }
    
    uint16_t value = DataConverter::fromCurrent(current);
    return writeRegister(XY7025_I_SET, value);
}

bool XY7025_Modbus::setOutputPower(float power) {
    // Validar rango de potencia (0-1000W típicamente)
    if (power < 0.0 || power > 1000.0) {
        if (debugMode) {
            Serial.println(F("XY7025_Modbus: Potencia fuera de rango"));
        }
        return false;
    }
    
    // Nota: No hay registro directo de potencia configurada
    // La potencia se controla mediante voltaje y corriente
    if (debugMode) {
        Serial.println(F("XY7025_Modbus: Potencia controlada por V/I"));
    }
    return false;
}

bool XY7025_Modbus::enableOutput(bool enable) {
    return writeRegister(XY7025_ONOFF, enable ? 1 : 0);
}

bool XY7025_Modbus::setProfile(uint8_t profile) {
    // Validar rango de perfil (0-9)
    if (profile > 9) {
        if (debugMode) {
            Serial.println(F("XY7025_Modbus: Perfil fuera de rango (0-9)"));
        }
        return false;
    }
    
    return writeRegister(XY7025_EXTRACT_M, profile);
}

// Funciones de estado y protección
uint8_t XY7025_Modbus::getProtectionStatus() {
    if (readHoldingRegisters(XY7025_PROTECT, 1)) {
        return getResponseBuffer(0) & 0xFF; // Solo el byte bajo
    }
    return PROTECT_NORMAL; // Por defecto, asumir normal
}

String XY7025_Modbus::getProtectionDescription(uint8_t status) {
    switch (status) {
        case PROTECT_NORMAL:
            return F("Normal");
        case PROTECT_OVP:
            return F("Sobretensión");
        case PROTECT_OCP:
            return F("Sobrecorriente");
        case PROTECT_OPP:
            return F("Sobrepotencia");
        case PROTECT_LVP:
            return F("Subtensión");
        case PROTECT_OAH:
            return F("Límite capacidad AH");
        case PROTECT_OHP:
            return F("Límite tiempo operación");
        case PROTECT_OTP:
            return F("Sobretemperatura");
        case PROTECT_OEP:
            return F("Sin salida");
        case PROTECT_OWH:
            return F("Límite energía WH");
        case PROTECT_ICP:
            return F("Límite corriente entrada");
        case PROTECT_ETP:
            return F("Temperatura externa");
        default:
            return F("Desconocido");
    }
}