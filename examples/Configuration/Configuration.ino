/*
 * Ejemplo de configuración del MPPT XY7025 usando Modbus
 * 
 * Este ejemplo muestra cómo configurar los parámetros del dispositivo:
 * - Establecer voltaje y corriente de salida
 * - Activar/desactivar la salida
 * - Cambiar entre perfiles
 * - Configurar dirección Modbus
 * 
 * Conexiones:
 * - Pin 2 (RX) del Arduino al pin TX del XY7025
 * - Pin 3 (TX) del Arduino al pin RX del XY7025
 * - GND común entre Arduino y XY7025
 */

#include <SoftwareSerial.h>
#include <XY7025_Modbus.h>

SoftwareSerial mpptSerial(2, 3); // RX, TX
XY7025_Modbus mppt(mpptSerial, 1);

// Variables para almacenar la configuración actual
float targetVoltage = 24.0;  // Voltaje objetivo
float targetCurrent = 5.0;   // Corriente objetivo
bool outputEnabled = false;  // Estado de la salida
uint8_t currentProfile = 0;  // Perfil actual

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ;
    }
    
    Serial.println(F("=== Configurador XY7025 MPPT ==="));
    
    // Inicializar comunicación
    if (mppt.begin(115200)) {
        Serial.println(F("Comunicación iniciada"));
    } else {
        Serial.println(F("Error al iniciar comunicación"));
        while (1);
    }
    
    mppt.enableDebug(true);
    
    Serial.println(F("Comandos disponibles:"));
    Serial.println(F("1 - Leer estado actual"));
    Serial.println(F("2 - Establecer voltaje (ej: 2 24.5)"));
    Serial.println(F("3 - Establecer corriente (ej: 3 2.5)"));
    Serial.println(F("4 - Activar salida"));
    Serial.println(F("5 - Desactivar salida"));
    Serial.println(F("6 - Cambiar perfil (ej: 6 1)"));
    Serial.println(F("7 - Probar dirección esclavo"));
    Serial.println(F("8 - Mostrar ayuda"));
    Serial.println(F("====================================="));
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.length() == 0) {
            return;
        }
        
        int spaceIndex = command.indexOf(' ');
        String cmd = (spaceIndex > 0) ? command.substring(0, spaceIndex) : command;
        String param = (spaceIndex > 0) ? command.substring(spaceIndex + 1) : "";
        
        int cmdNum = cmd.toInt();
        
        switch (cmdNum) {
            case 1:
                readCurrentStatus();
                break;
                
            case 2:
                if (param.length() > 0) {
                    float voltage = param.toFloat();
                    setOutputVoltage(voltage);
                } else {
                    Serial.println(F("Error: Debe especificar voltaje"));
                }
                break;
                
            case 3:
                if (param.length() > 0) {
                    float current = param.toFloat();
                    setOutputCurrent(current);
                } else {
                    Serial.println(F("Error: Debe especificar corriente"));
                }
                break;
                
            case 4:
                enableOutput(true);
                break;
                
            case 5:
                enableOutput(false);
                break;
                
            case 6:
                if (param.length() > 0) {
                    int profile = param.toInt();
                    setProfile(profile);
                } else {
                    Serial.println(F("Error: Debe especificar perfil (0-9)"));
                }
                break;
                
            case 7:
                probeSlaveAddresses();
                break;
                
            case 8:
                showHelp();
                break;
                
            default:
                Serial.println(F("Comando no reconocido. Use 8 para ayuda."));
                break;
        }
    }
}

void readCurrentStatus() {
    Serial.println(F("\n=== Estado Actual ==="));
    
    // Leer configuración actual
    float voltajeConfig = 0;
    if (mppt.readHoldingRegisters(XY7025_V_SET, 1)) {
        voltajeConfig = DataConverter::toVoltage(mppt.getResponseBuffer(0));
    }
    
    float corrienteConfig = 0;
    if (mppt.readHoldingRegisters(XY7025_I_SET, 1)) {
        corrienteConfig = DataConverter::toCurrent(mppt.getResponseBuffer(0));
    }
    
    Serial.print(F("Voltaje configurado: "));
    Serial.print(voltajeConfig);
    Serial.println(F("V"));
    
    Serial.print(F("Corriente configurada: "));
    Serial.print(corrienteConfig);
    Serial.println(F("A"));
    
    // Leer estado de salida
    bool salidaActiva = false;
    if (mppt.readHoldingRegisters(XY7025_ONOFF, 1)) {
        salidaActiva = mppt.getResponseBuffer(0) > 0;
    }
    
    Serial.print(F("Salida: "));
    Serial.println(salidaActiva ? F("ACTIVA") : F("INACTIVA"));
    
    // Leer perfil actual
    int perfil = 0;
    if (mppt.readHoldingRegisters(XY7025_EXTRACT_M, 1)) {
        perfil = mppt.getResponseBuffer(0);
    }
    
    Serial.print(F("Perfil actual: M"));
    Serial.println(perfil);
    
    // Leer valores en tiempo real
    float voltajeSalida = mppt.readVoltageOutput();
    float corrienteSalida = mppt.readCurrentOutput();
    float potenciaSalida = mppt.readPowerOutput();
    
    Serial.print(F("Voltaje de salida: "));
    Serial.print(voltajeSalida);
    Serial.println(F("V"));
    
    Serial.print(F("Corriente de salida: "));
    Serial.print(corrienteSalida);
    Serial.println(F("A"));
    
    Serial.print(F("Potencia de salida: "));
    Serial.print(potenciaSalida);
    Serial.println(F("W"));
    
    uint8_t protectStatus = mppt.getProtectionStatus();
    Serial.print(F("Protección: "));
    Serial.println(mppt.getProtectionDescription(protectStatus));
}

void setOutputVoltage(float voltage) {
    Serial.print(F("Estableciendo voltaje a "));
    Serial.print(voltage);
    Serial.println(F("V..."));
    
    if (mppt.setOutputVoltage(voltage)) {
        Serial.println(F("✓ Voltaje configurado exitosamente"));
        targetVoltage = voltage;
    } else {
        Serial.println(F("✗ Error configurando voltaje"));
    }
}

void setOutputCurrent(float current) {
    Serial.print(F("Estableciendo corriente a "));
    Serial.print(current);
    Serial.println(F("A..."));
    
    if (mppt.setOutputCurrent(current)) {
        Serial.println(F("✓ Corriente configurada exitosamente"));
        targetCurrent = current;
    } else {
        Serial.println(F("✗ Error configurando corriente"));
    }
}

void enableOutput(bool enable) {
    if (enable) {
        Serial.println(F("Activando salida..."));
    } else {
        Serial.println(F("Desactivando salida..."));
    }
    
    if (mppt.enableOutput(enable)) {
        Serial.println(enable ? F("✓ Salida activada") : F("✓ Salida desactivada"));
        outputEnabled = enable;
    } else {
        Serial.println(F("✗ Error cambiando estado de salida"));
    }
}

void setProfile(int profile) {
    if (profile < 0 || profile > 9) {
        Serial.println(F("Error: El perfil debe estar entre 0 y 9"));
        return;
    }
    
    Serial.print(F("Cambiando a perfil M"));
    Serial.println(profile);
    
    if (mppt.setProfile(profile)) {
        Serial.println(F("✓ Perfil cambiado exitosamente"));
        currentProfile = profile;
    } else {
        Serial.println(F("✗ Error cambiando perfil"));
    }
}

void probeSlaveAddresses() {
    Serial.println(F("Buscando dispositivos Modbus..."));
    Serial.println(F("Dirección | Estado"));
    Serial.println(F("---------|-------"));
    
    for (int addr = 1; addr <= 10; addr++) {
        Serial.print(F("    "));
        Serial.print(addr);
        Serial.print(F("    | "));
        
        if (mppt.probeSlaveAddress(addr)) {
            Serial.println(F("ENCONTRADO"));
        } else {
            Serial.println(F("---"));
        }
        
        delay(100); // Pequeña pausa entre pruebas
    }
}

void showHelp() {
    Serial.println(F("\n=== AYUDA ==="));
    Serial.println(F("1 - Leer estado actual del dispositivo"));
    Serial.println(F("2 - Establecer voltaje de salida (ej: 2 24.5)"));
    Serial.println(F("3 - Establecer corriente de salida (ej: 3 2.5)"));
    Serial.println(F("4 - Activar la salida"));
    Serial.println(F("5 - Desactivar la salida"));
    Serial.println(F("6 - Cambiar perfil (ej: 6 1 para M1)"));
    Serial.println(F("7 - Buscar dispositivos Modbus en direcciones 1-10"));
    Serial.println(F("8 - Mostrar esta ayuda"));
    Serial.println(F(""));
    Serial.println(F("Rangos típicos:"));
    Serial.println(F("- Voltaje: 0-100V"));
    Serial.println(F("- Corriente: 0-30A"));
    Serial.println(F("- Perfiles: M0-M9"));
}