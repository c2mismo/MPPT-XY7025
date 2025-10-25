/*
 * Ejemplo básico de lectura del MPPT XY7025 usando Modbus
 * 
 * Este ejemplo muestra cómo leer todos los registros del dispositivo
 * y mostrarlos por el monitor serial.
 * 
 * Conexiones:
 * - Pin 2 (RX) del Arduino al pin TX del XY7025
 * - Pin 3 (TX) del Arduino al pin RX del XY7025
 * - GND común entre Arduino y XY7025
 * 
 * La dirección Modbus por defecto es 1 (configurable)
 */

#include <SoftwareSerial.h>
#include <XY7025_Modbus.h>

// Crear puerto serial software para comunicación Modbus
SoftwareSerial mpptSerial(2, 3); // RX, TX

// Crear objeto XY7025_Modbus con dirección esclavo 1
XY7025_Modbus mppt(mpptSerial, 1);

void setup() {
    // Inicializar puerto serial para debug
    Serial.begin(9600);
    while (!Serial) {
        ; // Esperar a que el puerto serial esté listo
    }
    
    Serial.println(F("=== Iniciando XY7025 Modbus Reader ==="));
    
    // Inicializar comunicación Modbus a 115200 baudios
    if (mppt.begin(115200)) {
        Serial.println(F("Comunicación Modbus iniciada correctamente"));
    } else {
        Serial.println(F("Error al iniciar comunicación Modbus"));
        while (1); // Detener si hay error
    }
    
    // Habilitar modo debug para ver mensajes detallados
    mppt.enableDebug(true);
    
    // Configurar timeout y reintentos
    mppt.setTimeout(2000);  // 2 segundos de timeout
    mppt.setRetries(3);     // 3 reintentos en caso de error
    
    Serial.println(F("Configuración completada"));
    Serial.println(F("====================================="));
}

void loop() {
    Serial.println(F("\n=== Leyendo estado del MPPT XY7025 ==="));
    
    // Leer y mostrar todos los registros
    mppt.readAllRegisters(Serial);
    
    // Leer valores individuales como ejemplo
    Serial.println(F("\n=== Lecturas individuales ==="));
    
    float voltage = mppt.readVoltageOutput();
    if (voltage >= 0) {
        Serial.print(F("Voltaje de salida: "));
        Serial.print(voltage);
        Serial.println(F("V"));
    } else {
        Serial.println(F("Error leyendo voltaje de salida"));
    }
    
    float current = mppt.readCurrentOutput();
    if (current >= 0) {
        Serial.print(F("Corriente de salida: "));
        Serial.print(current);
        Serial.println(F("A"));
    } else {
        Serial.println(F("Error leyendo corriente de salida"));
    }
    
    float power = mppt.readPowerOutput();
    if (power >= 0) {
        Serial.print(F("Potencia de salida: "));
        Serial.print(power);
        Serial.println(F("W"));
    } else {
        Serial.println(F("Error leyendo potencia de salida"));
    }
    
    float inputVoltage = mppt.readVoltageInput();
    if (inputVoltage >= 0) {
        Serial.print(F("Voltaje de entrada: "));
        Serial.print(inputVoltage);
        Serial.println(F("V"));
    } else {
        Serial.println(F("Error leyendo voltaje de entrada"));
    }
    
    uint32_t ampHours = mppt.readAmpHours();
    if (ampHours > 0) {
        Serial.print(F("Amperios-hora: "));
        Serial.print(ampHours / 1000.0);
        Serial.println(F("Ah"));
    }
    
    uint32_t wattHours = mppt.readWattHours();
    if (wattHours > 0) {
        Serial.print(F("Vatios-hora: "));
        Serial.print(wattHours / 100.0);
        Serial.println(F("Wh"));
    }
    
    float tempInt = mppt.readTemperatureInternal();
    if (tempInt > -100) {
        Serial.print(F("Temperatura interna: "));
        Serial.print(tempInt);
        Serial.println(F("°C"));
    }
    
    float tempExt = mppt.readTemperatureExternal();
    if (tempExt > -100) {
        Serial.print(F("Temperatura externa: "));
        Serial.print(tempExt);
        Serial.println(F("°C"));
    }
    
    // Mostrar estado de protección
    uint8_t protectStatus = mppt.getProtectionStatus();
    Serial.print(F("Estado de protección: "));
    Serial.println(mppt.getProtectionDescription(protectStatus));
    
    Serial.println(F("====================================="));
    Serial.println(F("Siguiente lectura en 10 segundos..."));
    
    // Esperar 10 segundos antes de la siguiente lectura
    delay(10000);
}