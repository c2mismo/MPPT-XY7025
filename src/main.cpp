/*
 * Sketch de prueba completo para XY7025_Modbus
 * 
 * Este sketch permite:
 * 1. Verificar la dirección del esclavo Modbus (SLAVE-ADD)
 * 2. Leer todos los registros del XY7025 y volcarlos a archivo
 * 3. Escribir valores específicos en registros del XY7025
 * 
 * Comunicación: Modbus RTU sobre SoftwareSerial
 * Pinos: D2 (RX), D3 (TX)
 * Velocidad: 115200 baudios
 * 
 * Comandos disponibles por puerto serial:
 * - 'a': Verificar dirección esclavo actual
 * - 's': Buscar dispositivos en rango de direcciones 1-247
 * - 'r': Leer todos los registros
 * - 'v': Leer voltaje de salida
 * - 'c': Leer corriente de salida
 * - 'p': Leer potencia de salida
 * - 't': Leer temperaturas
 * - 'l': Leer registro concreto
 * - 'w': Escribir valor en registro
 * - 'h': Mostrar ayuda
 * 
 * Para volcar a archivo con screen:
 * screen -L /dev/ttyUSB0 9600
 * (Luego ejecutar comandos y salir con Ctrl+A, K)
 */

#include <SoftwareSerial.h>
#include <XY7025_Modbus.h>

// Configuración de pines y comunicación
SoftwareSerial mpptSerial(2, 3); // RX, TX
XY7025_Modbus mppt(mpptSerial, 5); // Dirección por defecto: 1

// Variables globales
bool debugMode = true;
uint8_t currentSlaveAddress = 5;

// Prototipos de funciones
void printHelp();
void verifySlaveAddress();
void scanSlaveAddresses();
void readAllRegisters();
void readBasicValues();
void writeRegisterMenu();
void testWriteOperation();
void readSpecificRegister();

void setup() {
    // Inicializar puerto serial para debug/monitor
    Serial.begin(9600);
    while (!Serial) {
        ; // Esperar a que el puerto serial esté listo
    }
    
    Serial.println(F("\n=== XY7025 Modbus Test Suite ==="));
    Serial.println(F("Herramienta de prueba para MPPT XY7025"));
    Serial.println(F("Versión 1.0 - Desarrollado para Arduino UNO"));
    Serial.println(F("====================================="));
    
    // Inicializar comunicación Modbus
    Serial.print(F("Inicializando comunicación Modbus a 115200 baudios... "));
    if (mppt.begin(115200)) {
        Serial.println(F("OK"));
    } else {
        Serial.println(F("ERROR"));
        Serial.println(F("Verifica las conexiones y reinicia"));
        while (1);
    }
    
    // Configurar parámetros de comunicación
    mppt.enableDebug(debugMode);
    mppt.setTimeout(2000);  // 2 segundos de timeout
    mppt.setRetries(3);     // 3 reintentos
    
    Serial.println(F("Configuración completada"));
    Serial.println(F("Pines: D2(RX) -> XY7025(TX), D3(TX) -> XY7025(RX)"));
    Serial.println(F("Velocidad: 115200 baudios"));
    Serial.println(F("Dirección por defecto: 1"));
    Serial.println(F("====================================="));
    
    printHelp();
}

void loop() {
    if (Serial.available()) {
        char command = Serial.read();
        
        switch (command) {
            case 'h': // Help
            case 'H':
                printHelp();
                break;
                
            case 'a': // Verificar dirección actual
            case 'A':
                verifySlaveAddress();
                break;
                
            case 's': // Buscar dispositivos
            case 'S':
                scanSlaveAddresses();
                break;
                
            case 'r': // Leer todos los registros
            case 'R':
                readAllRegisters();
                break;
                
            case 'l': // Leer registro concreto
            case 'L':
                readSpecificRegister();
                break;
                
            case 'v': // Leer voltaje
            case 'V':
                readBasicValues();
                break;
                
            case 'w': // Escribir registro
            case 'W':
                writeRegisterMenu();
                break;
                
            case 't': // Test de escritura
            case 'T':
                testWriteOperation();
                break;
                
            case '\n': // Ignorar saltos de línea
            case '\r':
                break;
                
            default:
                Serial.print(F("Comando desconocido: '"));
                Serial.print(command);
                Serial.println(F("'. Usa 'h' para ayuda."));
                break;
        }
    }
}

void printHelp() {
    Serial.println(F("\n=== COMANDOS DISPONIBLES ==="));
    Serial.println(F("a - Verificar dirección esclavo actual"));
    Serial.println(F("s - Buscar dispositivos Modbus (direcciones 1-247)"));
    Serial.println(F("r - Leer y mostrar todos los registros"));
    Serial.println(F("l - Leer registro concreto por dirección"));
    Serial.println(F("v - Leer valores básicos (V, I, P, Temp)"));
    Serial.println(F("w - Escribir valor en registro"));
    Serial.println(F("t - Test de escritura (cambiar voltaje)"));
    Serial.println(F("h - Mostrar esta ayuda"));
    Serial.println(F("====================================="));
    Serial.println(F("Para volcar a archivo, usar: screen -L /dev/ttyUSB0 9600"));
}

void verifySlaveAddress() {
    Serial.println(F("\n=== Verificación de Dirección Esclavo ==="));
    
    // Leer el registro SLAVE_ADD (0x0018)
    uint16_t slaveAddrReg;
    slaveAddrReg = mppt.readRegister(XY7025_SLAVE_ADD);
    if (slaveAddrReg != 0xFFFF) {
        Serial.print(F("Dirección esclavo en registro: "));
        Serial.println(slaveAddrReg);
        
        if (slaveAddrReg == currentSlaveAddress) {
            Serial.println(F("✓ La dirección coincide con la configurada"));
        } else {
            Serial.print(F("⚠ La dirección NO coincide. Configurada: "));
            Serial.print(currentSlaveAddress);
            Serial.print(F(", Registro: "));
            Serial.println(slaveAddrReg);
        }
    } else {
        Serial.println(F("✗ Error leyendo registro de dirección"));
    }
    
    // Probar comunicación con la dirección actual
    Serial.print(F("Probando comunicación con dirección "));
    Serial.print(currentSlaveAddress);
    Serial.print(F("... "));
    
    if (mppt.probeSlaveAddress(currentSlaveAddress)) {
        Serial.println(F("✓ COMUNICACIÓN OK"));
    } else {
        Serial.println(F("✗ SIN RESPUESTA"));
        Serial.println(F("Sugerencias:"));
        Serial.println(F("- Verifica conexiones (D2->TX, D3->RX, GND)"));
        Serial.println(F("- Prueba con comando 's' para buscar dispositivos"));
        Serial.println(F("- Verifica que el XY7025 esté encendido"));
    }
}

void scanSlaveAddresses() {
    Serial.println(F("\n=== Búsqueda de Dispositivos Modbus ==="));
    Serial.println(F("Escaneando direcciones 1-20 (primeras 20 por velocidad)"));
    Serial.println(F("Dirección | Estado"));
    Serial.println(F("---------|-------"));
    
    bool foundAny = false;
    for (int addr = 1; addr <= 20; addr++) {
        Serial.print(F("    "));
        if (addr < 10) Serial.print(' ');
        Serial.print(addr);
        Serial.print(F("    | "));
        
        if (mppt.probeSlaveAddress(addr)) {
            Serial.println(F("ENCONTRADO ✓"));
            foundAny = true;
            
            // Si encontramos el dispositivo, actualizar la dirección actual
            if (addr != currentSlaveAddress) {
                Serial.print(F("          > Actualizando dirección actual a "));
                Serial.println(addr);
                currentSlaveAddress = addr;
            }
        } else {
            Serial.println(F("---"));
        }
        
        delay(100); // Pausa entre pruebas
    }
    
    if (!foundAny) {
        Serial.println(F("\n⚠ No se encontraron dispositivos"));
        Serial.println(F("Sugerencias:"));
        Serial.println(F("- Verifica las conexiones físicas"));
        Serial.println(F("- Asegúrate de que el XY7025 esté encendido"));
        Serial.println(F("- Prueba con un rango mayor (modificar código)"));
    }
}

void readAllRegisters() {
    Serial.println(F("\n=== LECTURA COMPLETA DE REGISTROS ==="));
    Serial.println(F("Volcando todos los registros del XY7025..."));
    Serial.println(F("====================================="));
    
    // Leer todos los registros usando la función de la librería
    bool success = mppt.readAllRegisters(Serial);
    
    if (success) {
        Serial.println(F("====================================="));
        Serial.println(F("✓ Lectura completada exitosamente"));
    } else {
        Serial.println(F("====================================="));
        Serial.println(F("✗ Algunos registros no pudieron leerse"));
    }
    
    // Lectura adicional de registros específicos
    Serial.println(F("\n=== REGISTROS ADICIONALES ==="));
    
    // Leer modelo y versión
    uint16_t model, version;
    model = mppt.readModel();
    if (model != 0xFFFF) {
        Serial.print(F("Modelo: "));
        Serial.println(model);
    }
    
    version = mppt.readVersion();
    if (version != 0xFFFF) {
        Serial.print(F("Versión: "));
        Serial.println(version);
    }
    
    // Leer baudrate
    uint16_t baudrate;
    baudrate = mppt.readBaudrate();
    if (baudrate != 0xFFFF) {
        Serial.print(F("Baudrate registro: "));
        Serial.println(baudrate);
    }
    
    // Leer estado de salida
    uint16_t outputState;
    outputState = mppt.readOutputState();
    if (outputState != 0xFFFF) {
        Serial.print(F("Estado salida (ONOFF): "));
        Serial.println(outputState ? F("ACTIVA") : F("INACTIVA"));
    }
    
    // Leer modo de carga
    uint16_t chargeMode;
    chargeMode = mppt.readChargeMode();
    if (chargeMode != 0xFFFF) {
        Serial.print(F("Modo carga (CVCC): "));
        Serial.println(chargeMode ? F("CC (Corriente Constante)") : F("CV (Voltaje Constante)"));
    }
}

void readBasicValues() {
    Serial.println(F("\n=== LECTURA DE VALORES BÁSICOS ==="));
    
    // Voltaje de salida
    float voltage = mppt.readVoltageOutput();
    if (!isnan(voltage)) {
        Serial.print(F("Voltaje de salida: "));
        Serial.print(voltage);
        Serial.println(F("V"));
    } else {
        Serial.println(F("Error leyendo voltaje de salida"));
    }
    
    // Corriente de salida
    float current = mppt.readCurrentOutput();
    if (!isnan(current)) {
        Serial.print(F("Corriente de salida: "));
        Serial.print(current);
        Serial.println(F("A"));
    } else {
        Serial.println(F("Error leyendo corriente de salida"));
    }
    
    // Potencia de salida
    float power = mppt.readPowerOutput();
    if (!isnan(power)) {
        Serial.print(F("Potencia de salida: "));
        Serial.print(power);
        Serial.println(F("W"));
    } else {
        Serial.println(F("Error leyendo potencia de salida"));
    }
    
    // Voltaje de entrada
    float inputVoltage = mppt.readVoltageInput();
    if (!isnan(inputVoltage)) {
        Serial.print(F("Voltaje de entrada: "));
        Serial.print(inputVoltage);
        Serial.println(F("V"));
    } else {
        Serial.println(F("Error leyendo voltaje de entrada"));
    }
    
    // Temperaturas
    float tempInt = mppt.readTemperatureInternal();
    if (!isnan(tempInt)) {
        Serial.print(F("Temperatura interna: "));
        Serial.print(tempInt);
        Serial.println(F("°C"));
    }
    
    float tempExt = mppt.readTemperatureExternal();
    if (!isnan(tempExt)) {
        Serial.print(F("Temperatura externa: "));
        Serial.print(tempExt);
        Serial.println(F("°C"));
    }
    
    // Estado de protección
    uint8_t protectStatus = mppt.getProtectionStatus();
    Serial.print(F("Estado de protección: "));
    Serial.println(mppt.getProtectionDescription(protectStatus));
}

void writeRegisterMenu() {
    Serial.println(F("\n=== ESCRITURA EN REGISTRO ==="));
    Serial.println(F("Formato: w <direccion_hex> <valor_decimal>"));
    Serial.println(F("Ejemplos:"));
    Serial.println(F("  w 0 2400    -> Establecer voltaje a 24.00V"));
    Serial.println(F("  w 1 5000    -> Establecer corriente a 5.000A"));
    Serial.println(F("  w 12 1      -> Activar salida"));
    Serial.println(F("  w 12 0      -> Desactivar salida"));
    Serial.println(F("Registros comunes:"));
    Serial.println(F("  0x0000: V-SET (voltaje, 2 decimales)"));
    Serial.println(F("  0x0001: I-SET (corriente, 3 decimales)"));
    Serial.println(F("  0x0012: ONOFF (1=activar, 0=desactivar)"));
    Serial.println(F("  0x0018: SLAVE-ADD (dirección esclavo)"));
    Serial.println(F("Ingresa comando (w para mostrar este menú):"));
}

void testWriteOperation() {
    Serial.println(F("\n=== TEST DE ESCRITURA ==="));
    Serial.println(F("Este test cambiará el voltaje de salida para verificar"));
    Serial.println(F("que los cambios se reflejan en la pantalla del XY7025"));
    Serial.println(F("====================================="));
    
    // Leer voltaje actual
    float currentVoltage = mppt.readVoltageOutput();
    if (isnan(currentVoltage)) {
        Serial.println(F("✗ Error leyendo voltaje actual"));
        return;
    }
    
    Serial.print(F("Voltaje actual: "));
    Serial.print(currentVoltage);
    Serial.println(F("V"));
    
    // Calcular nuevo voltaje (±2V del actual, dentro de límites)
    float newVoltage = currentVoltage + 2.0;
    if (newVoltage > 28.0) newVoltage = 20.0;
    if (newVoltage < 5.0) newVoltage = 19.5;
    
    Serial.print(F("Estableciendo voltaje a: "));
    Serial.print(newVoltage);
    Serial.println(F("V..."));
    
    // Intentar establecer el nuevo voltaje
    if (mppt.setOutputVoltage(newVoltage)) {
        Serial.println(F("✓ Comando de escritura enviado"));
        Serial.println(F("Verifica en la pantalla del XY7025 si el voltaje cambió"));
        Serial.println(F("Una vez verificado, presiona cualquier tecla para continuar..."));
        
        // Esperar confirmación del usuario
        while (!Serial.available()) {
            // Esperar activamente
        }
        // Limpiar el buffer del serial
        while (Serial.available()) {
            Serial.read();
        }
        
        float verifiedVoltage = mppt.readVoltageOutput();
        if (!isnan(verifiedVoltage)) {
            Serial.print(F("Voltaje después de cambio: "));
            Serial.print(verifiedVoltage);
            Serial.println(F("V"));
            
            if (abs(verifiedVoltage - newVoltage) < 0.1) {
                Serial.println(F("✓ Cambio verificado exitosamente"));
            } else {
                Serial.println(F("⚠ El voltaje no coincide exactamente"));
                Serial.println(F("Esto puede ser normal debido a regulación interna"));
            }
        }
        
        // Restaurar voltaje original
        Serial.print(F("Restaurando voltaje original... "));
        if (mppt.setOutputVoltage(currentVoltage)) {
            Serial.println(F("✓ OK"));
        } else {
            Serial.println(F("✗ ERROR"));
        }
        
    } else {
        Serial.println(F("✗ Error al escribir el registro"));
        Serial.println(F("Posibles causas:"));
        Serial.println(F("- El dispositivo está bloqueado"));
        Serial.println(F("- El valor está fuera de rango"));
        Serial.println(F("- Error de comunicación"));
    }
}

void readSpecificRegister() {
    Serial.println(F("\n=== LECTURA DE REGISTRO ESPECÍFICO ==="));
    Serial.println(F("Ingresa la dirección del registro en hexadecimal (0x0000-0x00ED)"));
    Serial.println(F("Ejemplos: 0000 (V-SET), 0002 (V-OUT), 0012 (ONOFF), 0018 (SLAVE-ADD)"));
    Serial.println(F("Registros comunes:"));
    Serial.println(F("  0x0000: V-SET (voltaje configurado)"));
    Serial.println(F("  0x0002: V-OUT (voltaje de salida)"));
    Serial.println(F("  0x0003: I-OUT (corriente de salida)"));
    Serial.println(F("  0x0004: POWER (potencia de salida)"));
    Serial.println(F("  0x0012: ONOFF (estado de salida)"));
    Serial.println(F("  0x0018: SLAVE-ADD (dirección esclavo)"));
    Serial.println(F("Ingresa 4 dígitos hexadecimales (ej: 0002) o 'q' para cancelar:"));
    
    // Esperar entrada del usuario
    while (!Serial.available()) {
        delay(100);
    }
    
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    // Verificar si el usuario quiere cancelar
    if (input == "q" || input == "Q") {
        Serial.println(F("Operación cancelada"));
        return;
    }
    
    // Validar longitud de entrada
    if (input.length() != 4) {
        Serial.println(F("✗ Error: Debes ingresar exactamente 4 dígitos hexadecimales"));
        return;
    }
    
    // Convertir string hexadecimal a uint16_t
    char* endptr;
    uint16_t regAddress = strtol(input.c_str(), &endptr, 16);
    
    // Verificar que la conversión fue exitosa
    if (*endptr != '\0') {
        Serial.println(F("✗ Error: Formato hexadecimal inválido"));
        return;
    }
    
    // Validar rango de dirección
    if (regAddress > 0x00ED) {
        Serial.print(F("✗ Error: Dirección 0x"));
        Serial.print(regAddress, HEX);
        Serial.println(F(" fuera de rango (máximo 0x00ED)"));
        return;
    }
    
    Serial.print(F("Leyendo registro 0x"));
    if (regAddress < 0x1000) Serial.print("0");
    if (regAddress < 0x100) Serial.print("0");
    if (regAddress < 0x10) Serial.print("0");
    Serial.print(regAddress, HEX);
    Serial.println(F("..."));
    
    // Leer el registro usando la función de la librería
    uint16_t value = mppt.readRegister(regAddress);
    
    if (value != 0xFFFF) {
        Serial.print(F("✓ Valor leído: "));
        Serial.println(value);
        
        // Mostrar interpretación adicional para registros conocidos
        switch (regAddress) {
            case XY7025_V_SET:
                Serial.print(F("  → Voltaje configurado: "));
                Serial.print(value / 100.0);
                Serial.println(F("V"));
                break;
            case XY7025_VOUT:
                Serial.print(F("  → Voltaje de salida: "));
                Serial.print(value / 100.0);
                Serial.println(F("V"));
                break;
            case XY7025_IOUT:
                Serial.print(F("  → Corriente de salida: "));
                Serial.print(value / 1000.0);
                Serial.println(F("A"));
                break;
            case XY7025_POWER:
                Serial.print(F("  → Potencia de salida: "));
                Serial.print(value / 100.0);
                Serial.println(F("W"));
                break;
            case XY7025_T_IN:
                Serial.print(F("  → Temperatura interna: "));
                Serial.print(value / 10.0);
                Serial.println(F("°C"));
                break;
            case XY7025_T_EX:
                Serial.print(F("  → Temperatura externa: "));
                Serial.print(value / 10.0);
                Serial.println(F("°C"));
                break;
            case XY7025_ONOFF:
                Serial.print(F("  → Estado de salida: "));
                Serial.println(value ? F("ACTIVA") : F("INACTIVA"));
                break;
            case XY7025_SLAVE_ADD:
                Serial.print(F("  → Dirección esclavo: "));
                Serial.println(value);
                break;
            case XY7025_MODEL:
                Serial.print(F("  → Modelo del dispositivo: "));
                Serial.println(value);
                break;
            case XY7025_VERSION:
                Serial.print(F("  → Versión de firmware: "));
                Serial.println(value);
                break;
            default:
                // Para registros no reconocidos, mostrar en diferentes formatos
                Serial.print(F("  → Valor decimal: "));
                Serial.println(value);
                Serial.print(F("  → Valor hexadecimal: 0x"));
                if (value < 0x1000) Serial.print("0");
                if (value < 0x100) Serial.print("0");
                if (value < 0x10) Serial.print("0");
                Serial.println(value, HEX);
                Serial.print(F("  → Valor binario: "));
                for (int i = 15; i >= 0; i--) {
                    Serial.print((value >> i) & 1);
                    if (i == 8) Serial.print(" ");
                }
                Serial.println();
                break;
        }
    } else {
        Serial.println(F("✗ Error leyendo el registro"));
        Serial.println(F("Posibles causas:"));
        Serial.println(F("- Dirección de registro inválida"));
        Serial.println(F("- Error de comunicación Modbus"));
        Serial.println(F("- El dispositivo no responde"));
    }
}