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
    modbus.begin(slaveAddress, *serial);
    // Nota: ModbusMaster no tiene método setTimeout, usaremos delay entre operaciones
    
    if (debugMode) {
        Serial.println(F("XY7025_Modbus: Inicializado con éxito"));
    }
    return true;
}

// Configurar timeout
void XY7025_Modbus::setTimeout(uint16_t timeout) {
    this->timeout = timeout;
    // Nota: ModbusMaster no tiene método setTimeout
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
    unsigned long startTime = millis(); // Tiempo inicial para control de timeout total
    unsigned long retryDelay = 100;     // Delay inicial entre reintentos
    const unsigned long maxRetryDelay = 500; // Máximo delay entre reintentos
    
    do {
        result = modbus.readHoldingRegisters(address, count);
        attempt++;
        
        if (result == modbus.ku8MBSuccess) {
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
        
        // Verificar si se ha excedido el timeout total
        if (millis() - startTime >= timeout) {
            if (debugMode) {
                Serial.println(F("XY7025_Modbus: Timeout total excedido"));
            }
            return false;
        }
        
        // Delay entre reintentos con backoff exponencial
        delay(retryDelay);
        retryDelay = min(retryDelay * 2, maxRetryDelay); // Duplicar delay hasta el máximo
        
    } while (attempt < retries);
    
    return false;
}

// Obtener valor del buffer de respuesta
uint16_t XY7025_Modbus::getResponseBuffer(uint8_t index) {
    return modbus.getResponseBuffer(index);
}

// Leer todos los registros principales
bool XY7025_Modbus::readAllRegisters(Print& output) {
    bool allSuccess = true; // Indicador de éxito global
    String errorDetails = ""; // Acumular detalles de errores
    
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
        } else {
            allSuccess = false;
            errorDetails += F(" [Amperios-hora: ERROR]");
            output.println(F("Amperios-hora: ERROR"));
        }
        
        if (readHoldingRegisters(XY7025_WH_LOW, 2)) {
            uint32_t wattHours = DataConverter::combine32(getResponseBuffer(1), getResponseBuffer(0));
            output.print(F("Vatios-hora: ")); output.print(wattHours / 100.0); output.println(F("Wh"));
        } else {
            allSuccess = false;
            errorDetails += F(" [Vatios-hora: ERROR]");
            output.println(F("Vatios-hora: ERROR"));
        }
        
        // Leer temperaturas
        if (readHoldingRegisters(XY7025_T_IN, 2)) {
            float tempInt = DataConverter::toTemperature(getResponseBuffer(0));
            float tempExt = DataConverter::toTemperature(getResponseBuffer(1));
            output.print(F("Temp. Interna: ")); output.print(tempInt); output.println(F("°C"));
            output.print(F("Temp. Externa: ")); output.print(tempExt); output.println(F("°C"));
        } else {
            allSuccess = false;
            errorDetails += F(" [Temperaturas: ERROR]");
            output.println(F("Temp. Interna: ERROR"));
            output.println(F("Temp. Externa: ERROR"));
        }
        
        // Leer estado de protección
        uint8_t protectStatus = getProtectionStatus();
        if (protectStatus == XY7025_ERROR_UINT8) {
            allSuccess = false;
            errorDetails += F(" [Protección: ERROR_COM]");
            output.println(F("Estado de Protección: ERROR COMUNICACIÓN"));
        } else {
            output.print(F("Estado de Protección: "));
            output.println(getProtectionDescription(protectStatus));
        }
        
        // Mostrar resumen de errores si los hay
        if (!allSuccess && debugMode) {
            Serial.print(F("XY7025_Modbus: Errores parciales en readAllRegisters"));
            Serial.println(errorDetails);
        }
        
        return allSuccess;
    }
    
    output.println(F("Error: No se pudieron leer los registros básicos"));
    return false;
}

// Escribir un solo registro
bool XY7025_Modbus::writeRegister(uint16_t address, uint16_t value) {
    uint8_t result = modbus.writeSingleRegister(address, value);
    
    if (result == modbus.ku8MBSuccess) {
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
    // ModbusMaster no soporta escritura múltiple de registros con la firma esperada
    // Vamos a escribir los valores uno por uno
    for (uint8_t i = 0; i < count; i++) {
        if (!writeRegister(address + i, values[i])) {
            if (debugMode) {
                Serial.print(F("XY7025_Modbus: Error en escritura múltiple, registro "));
                Serial.println(address + i);
            }
            return false;
        }
    }
    
    if (debugMode) {
        Serial.print(F("XY7025_Modbus: "));
        Serial.print((int)count);
        Serial.print(F(" registros escritos desde 0x"));
        Serial.println(address, HEX);
    }
    return true;
}

// Probar dirección de esclavo
bool XY7025_Modbus::probeSlaveAddress(uint8_t address) {
    uint8_t originalAddress = slaveAddress;
    bool success = false;
    
    // Cambiar temporalmente la dirección del esclavo
    slaveAddress = address;
    modbus.begin(slaveAddress, *serial);
    
    // Intentar leer un registro conocido (voltaje de salida)
    // Si hay algún error crítico, success permanecerá en false
    uint16_t vout = readRegister(XY7025_VOUT);
    success = (vout != XY7025_ERROR_UINT16);
    
    // Restaurar dirección original SIEMPRE, incluso si hay error
    // Esto previene la corrupción de estado del objeto
    slaveAddress = originalAddress;
    modbus.begin(slaveAddress, *serial);
    
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
    return XY7025_ERROR_FLOAT; // NAN indica error de comunicación
}

float XY7025_Modbus::readCurrentOutput() {
    if (readHoldingRegisters(XY7025_IOUT, 1)) {
        return DataConverter::toCurrent(getResponseBuffer(0));
    }
    return XY7025_ERROR_FLOAT; // NAN indica error de comunicación
}

float XY7025_Modbus::readPowerOutput() {
    if (readHoldingRegisters(XY7025_POWER, 1)) {
        return DataConverter::toPower(getResponseBuffer(0));
    }
    return XY7025_ERROR_FLOAT; // NAN indica error de comunicación
}

float XY7025_Modbus::readVoltageInput() {
    if (readHoldingRegisters(XY7025_UIN, 1)) {
        return DataConverter::toVoltage(getResponseBuffer(0));
    }
    return XY7025_ERROR_FLOAT; // NAN indica error de comunicación
}

uint32_t XY7025_Modbus::readAmpHours() {
    if (readHoldingRegisters(XY7025_AH_LOW, 2)) {
        return DataConverter::combine32(getResponseBuffer(1), getResponseBuffer(0));
    }
    return XY7025_ERROR_UINT32; // 0xFFFFFFFF indica error de comunicación
}

uint32_t XY7025_Modbus::readWattHours() {
    if (readHoldingRegisters(XY7025_WH_LOW, 2)) {
        return DataConverter::combine32(getResponseBuffer(1), getResponseBuffer(0));
    }
    return XY7025_ERROR_UINT32; // 0xFFFFFFFF indica error de comunicación
}

float XY7025_Modbus::readTemperatureInternal() {
    if (readHoldingRegisters(XY7025_T_IN, 1)) {
        return DataConverter::toTemperature(getResponseBuffer(0));
    }
    return XY7025_ERROR_FLOAT; // NAN indica error de comunicación
}

float XY7025_Modbus::readTemperatureExternal() {
    if (readHoldingRegisters(XY7025_T_EX, 1)) {
        return DataConverter::toTemperature(getResponseBuffer(0));
    }
    return XY7025_ERROR_FLOAT; // NAN indica error de comunicación
}

// Funciones de configuración
bool XY7025_Modbus::setOutputVoltage(float voltage) {
    // Usar la validación integrada en DataConverter
    uint16_t value = DataConverter::fromVoltage(voltage);
    if (value == XY7025_ERROR_UINT16) {
        if (debugMode) {
            Serial.print(F("XY7025_Modbus: Voltaje fuera de rango (0-"));
            Serial.print(XY7025_MAX_VOLTAGE);
            Serial.println(F("V)"));
        }
        return false;
    }
    return writeRegister(XY7025_V_SET, value);
}

bool XY7025_Modbus::setOutputCurrent(float current) {
    // Usar la validación integrada en DataConverter
    uint16_t value = DataConverter::fromCurrent(current);
    if (value == XY7025_ERROR_UINT16) {
        if (debugMode) {
            Serial.print(F("XY7025_Modbus: Corriente fuera de rango (0-"));
            Serial.print(XY7025_MAX_CURRENT);
            Serial.println(F("A)"));
        }
        return false;
    }
    return writeRegister(XY7025_I_SET, value);
}

bool XY7025_Modbus::setOutputPower(float power) {
    // Validar rango de potencia usando las constantes definidas
    if (power < 0.0 || power > XY7025_MAX_POWER) {
        if (debugMode) {
            Serial.print(F("XY7025_Modbus: Potencia fuera de rango (0-"));
            Serial.print(XY7025_MAX_POWER);
            Serial.println(F("W)"));
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
    // Si hay error de comunicación, retornar un código de error especial
    // en lugar de ocultar el error con PROTECT_NORMAL
    return XY7025_ERROR_UINT8; // 0xFF indica error de comunicación
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
    
    // Funciones de lectura de registros específicos
    uint16_t XY7025_Modbus::readRegister(uint16_t address) {
        if (readHoldingRegisters(address, 1)) {
            return getResponseBuffer(0);
        }
        return XY7025_ERROR_UINT16; // 0xFFFF indica error
    }
    
    uint16_t XY7025_Modbus::readModel() {
        return readRegister(XY7025_MODEL);
    }
    
    uint16_t XY7025_Modbus::readVersion() {
        return readRegister(XY7025_VERSION);
    }
    
    uint16_t XY7025_Modbus::readBaudrate() {
        return readRegister(XY7025_BAUDRATE_L);
    }
    
    uint16_t XY7025_Modbus::readOutputState() {
        return readRegister(XY7025_ONOFF);
    }
    
    uint16_t XY7025_Modbus::readChargeMode() {
        return readRegister(XY7025_CVCC);
    }