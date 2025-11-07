/*
  XY7025_Address_Finder - Configurador interactivo para dirección slave y baudrate
  
  Funcionalidades:
  - Verificar conexión con XY7025
  - Buscar dirección slave automáticamente
  - Buscar baudrate automáticamente  
  - Escribir configuración en XY7025
  - Cambiar configuración local (Arduino)
  - Menú interactivo con opciones completas

  Autor: Código generado para proyecto XY7025
  Fecha: 2025-11-06
*/

#include <Arduino.h>
#include <XY7025_Modbus.h>
#include <SoftwareSerial.h>

// Pines de comunicación
const uint8_t MODBUS_RX = 2;   // RX Arduino -> RX XY7025
const uint8_t MODBUS_TX = 3;   // TX Arduino -> TX XY7025

// Configuración por defecto
uint8_t currentSlaveAddress = 1;      // Dirección slave inicial
uint8_t currentBaudrate = 6;          // 115200 bps por defecto (índice 6)

// Tabla de baudrates disponibles
const uint32_t BAUDRATES[] = {
    9600,    // 0
    14400,   // 1
    19200,   // 2
    38400,   // 3
    56000,   // 4
    57600,   // 5
    115200,  // 6 - Por defecto
    2400,    // 7
    4800     // 8
};

const uint8_t BAUDRATE_COUNT = sizeof(BAUDRATES) / sizeof(BAUDRATES[0]);

// Objetos de comunicación
SoftwareSerial mpptSerial(MODBUS_RX, MODBUS_TX);
XY7025_Modbus mppt(mpptSerial, currentSlaveAddress);

// Variables de estado
bool systemConnected = false;
bool debugMode = true;
bool searchCancelled = false;

// Prototipos de funciones
void setup();
void loop();
void displayMenu();
void verifyConnection();
void searchSlaveAddress();
void searchBaudrateComplete();
void writeSlaveToXY7025();
void writeBaudrateToXY7025();
void changeLocalSlave();
void changeLocalBaudrate();
void showHelp();
bool testConnection();
String getBaudrateName(uint8_t index);
void printProgress(uint8_t current, uint8_t total, String prefix);

//====================================================================
// CONFIGURACIÓN INICIAL
//====================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println(F("=== INICIANDO CONFIGURADOR XY7025 ==="));
    Serial.println(F("Búsqueda automática de dispositivo..."));
    
    // Inicializar comunicación con baudrate por defecto
    mpptSerial.begin(BAUDRATES[currentBaudrate]);
    delay(500);
    
    // Inicializar objeto MPPT
    if (mppt.begin(BAUDRATES[currentBaudrate])) {
        Serial.println(F("✓ Comunicación Modbus inicializada"));
    } else {
        Serial.println(F("✗ Error inicializando Modbus"));
    }
    
    // Verificar conexión inicial
    Serial.println(F("\nVerificando conexión inicial..."));
    systemConnected = testConnection();
    
    if (systemConnected) {
        Serial.println(F("✓ Conexión establecida con slave ") + String(currentSlaveAddress) + 
                      F(" a ") + String(BAUDRATES[currentBaudrate]) + F(" bps"));
    } else {
        Serial.println(F("✗ Sin respuesta del dispositivo"));
        Serial.println(F("ℹ️ Use menú de búsqueda para encontrar el dispositivo"));
    }
    
    Serial.println(F("\nConfigurador listo. Presione una tecla para continuar..."));
    while (!Serial.available()) {
        delay(100);
    }
    Serial.read(); // Limpiar buffer
    
    displayMenu();
}

//====================================================================
// BUCLE PRINCIPAL
//====================================================================

void loop() {
    if (Serial.available()) {
        char command = Serial.read();
        command = toupper(command);
        
        // Limpiar caracteres de nueva línea
        while (Serial.available()) {
            char extra = Serial.read();
            if (extra == '\n' || extra == '\r') break;
        }
        
        Serial.println(F("\n--- Ejecutando opción: [") + String(command) + F("] ---"));
        
        switch (command) {
            case 'A':
                verifyConnection();
                break;
            case 'S':
                searchSlaveAddress();
                break;
            case 'B':
                searchBaudrateComplete();
                break;
            case 'W':
                writeSlaveToXY7025();
                break;
            case 'R':
                writeBaudrateToXY7025();
                break;
            case 'M':
                changeLocalSlave();
                break;
            case 'N':
                changeLocalBaudrate();
                break;
            case 'H':
                showHelp();
                break;
            case 'Q':
                Serial.println(F("Saliendo del configurador..."));
                while(true) delay(1000); // Detener programa
                break;
            case 'D':
                debugMode = !debugMode;
                mppt.enableDebug(debugMode);
                Serial.println(F("Debug mode: ") + String(debugMode ? F("ON") : F("OFF")));
                break;
            default:
                Serial.println(F("Comando no reconocido. Use 'h' para ayuda."));
                break;
        }
        
        Serial.println(F("\n--- Comando completado ---"));
        delay(500);
        displayMenu();
    }
}

//====================================================================
// MENÚ PRINCIPAL
//====================================================================

void displayMenu() {
    Serial.println(F("\n=== CONFIGURADOR XY7025 ==="));
    Serial.println(F("Estado actual:"));
    Serial.println(F("  Slave Address: ") + String(currentSlaveAddress));
    Serial.println(F("  Baudrate: ") + String(BAUDRATES[currentBaudrate]) + F(" bps (índice ") + String(currentBaudrate) + F(")"));
    Serial.println(F("  Conexión: ") + String(systemConnected ? F("OK") : F("ERROR")));
    Serial.println(F("  Debug Mode: ") + String(debugMode ? F("ON") : F("OFF")));
    Serial.println();
    Serial.println(F("Opciones:"));
    Serial.println(F("1. [a] Verificar conexión actual"));
    Serial.println(F("2. [s] Buscar dirección Slave (1-247)"));
    Serial.println(F("3. [b] Buscar Baudrate completo (proceso largo)"));
    Serial.println(F("4. [w] Escribir Slave en XY7025"));
    Serial.println(F("5. [r] Escribir Baudrate en XY7025"));
    Serial.println(F("6. [m] Cambiar Slave local (Arduino)"));
    Serial.println(F("7. [n] Cambiar Baudrate local (Arduino)"));
    Serial.println(F("8. [h] Ayuda"));
    Serial.println(F("9. [q] Salir"));
    Serial.println(F("10. [d] Toggle Debug Mode"));
    Serial.println();
    Serial.print(F("Comando: "));
}

//====================================================================
// FUNCIONES DE VERIFICACIÓN Y PRUEBA
//====================================================================

bool testConnection() {
    if (debugMode) {
        Serial.println(F("Probando conexión con slave ") + String(currentSlaveAddress) + 
                      F(" a ") + String(BAUDRATES[currentBaudrate]) + F(" bps..."));
    }
    
    bool success = mppt.readHoldingRegisters(XY7025_VOUT, 1);
    
    if (success) {
        uint16_t vout = mppt.getResponseBuffer(0);
        if (debugMode) {
            Serial.println(F("✓ Respuesta recibida. Vout = ") + String(vout));
        }
        return true;
    } else {
        if (debugMode) {
            Serial.println(F("✗ Sin respuesta del dispositivo"));
        }
        return false;
    }
}

void verifyConnection() {
    Serial.println(F("--- VERIFICANDO CONEXIÓN ---"));
    
    systemConnected = testConnection();
    
    if (systemConnected) {
        // Leer registros adicionales para confirmar funcionamiento
        Serial.println(F("Leyendo registros de configuración..."));
        
        // Leer dirección slave actual del dispositivo
        uint16_t deviceSlave = mppt.readRegister(XY7025_SLAVE_ADD);
        if (deviceSlave != XY7025_ERROR_UINT16) {
            Serial.println(F("  Dirección Slave en dispositivo: ") + String(deviceSlave));
        }
        
        // Leer baudrate actual del dispositivo  
        uint16_t deviceBaudrate = mppt.readBaudrate();
        if (deviceBaudrate != XY7025_ERROR_UINT16) {
            String baudName = F("Desconocido");
            for (uint8_t i = 0; i < BAUDRATE_COUNT; i++) {
                if (BAUDRATES[i] == deviceBaudrate) {
                    baudName = String(BAUDRATES[i]) + F(" bps");
                    break;
                }
            }
            Serial.println(F("  Baudrate en dispositivo: ") + baudName);
        }
        
        // Leer voltaje de salida actual
        uint16_t vout = mppt.readRegister(XY7025_VOUT);
        if (vout != XY7025_ERROR_UINT16) {
            float voutReal = vout / 100.0;
            Serial.println(F("  Voltaje salida actual: ") + String(voutReal, 2) + F(" V"));
        }
        
        Serial.println(F("✓ Conexión VERIFICADA exitosamente"));
    } else {
        Serial.println(F("✗ Conexión FALLIDA"));
        Serial.println(F("Sugerencias:"));
        Serial.println(F("  - Verificar conexiones físicas"));
        Serial.println(F("  - Comprobar baudrate correcto"));
        Serial.println(F("  - Intentar búsqueda de dirección slave"));
    }
}

//====================================================================
// BÚSQUEDA DE DIRECCIÓN SLAVE
//====================================================================

void searchSlaveAddress() {
    Serial.println(F("--- BÚSQUEDA DE DIRECCIÓN SLAVE ---"));
    Serial.println(F("⚠️ Este proceso probará direcciones 1-247"));
    Serial.println(F("Presione 'q' para cancelar en cualquier momento"));
    Serial.println();
    
    Serial.print(F("¿Continuar? (s/n): "));
    while (!Serial.available()) {
        delay(100);
    }
    char response = Serial.read();
    response = tolower(response);
    
    if (response != 's') {
        Serial.println(F("Búsqueda cancelada"));
        return;
    }
    
    Serial.println(F("\nIniciando búsqueda..."));
    Serial.println(F("Manteniendo baudrate: ") + String(BAUDRATES[currentBaudrate]) + F(" bps"));
    
    searchCancelled = false;
    uint8_t foundAddress = 0;
    
    for (uint8_t addr = 1; addr <= 247 && !searchCancelled; addr++) {
        printProgress(addr, 247, F("Probando dirección"));
        
        // Probar dirección
        if (mppt.probeSlaveAddress(addr)) {
            // Verificar con lectura adicional
            if (testConnectionWithAddress(addr)) {
                foundAddress = addr;
                Serial.println();
                Serial.println(F("================================================"));
                Serial.println(F("✓ DISPOSITIVO ENCONTRADO"));
                Serial.println(F("  Dirección: ") + String(foundAddress));
                Serial.println(F("  Baudrate: ") + String(BAUDRATES[currentBaudrate]) + F(" bps"));
                Serial.println(F("================================================"));
                break;
            }
        }
        
        // Verificar si se canceló
        if (Serial.available()) {
            char cancel = Serial.read();
            if (cancel == 'q' || cancel == 'Q') {
                searchCancelled = true;
                break;
            }
        }
        
        delay(100); // Pequeña pausa entre pruebas
    }
    
    if (searchCancelled) {
        Serial.println(F("\nBúsqueda cancelada por el usuario"));
    } else if (foundAddress == 0) {
        Serial.println(F("\n✗ No se encontró ningún dispositivo"));
        Serial.println(F("Sugerencias:"));
        Serial.println(F("  - Verificar que el XY7025 esté encendido"));
        Serial.println(F("  - Comprobar conexiones"));
        Serial.println(F("  - Intentar búsqueda de baudrate"));
    } else {
        // Actualizar configuración local
        currentSlaveAddress = foundAddress;
        mppt = XY7025_Modbus(mpptSerial, currentSlaveAddress);
        mppt.begin(BAUDRATES[currentBaudrate]);
        systemConnected = true;
        
        Serial.println(F("\n⚠️ ACCIÓN REQUERIDA:"));
        Serial.println(F("Si desea GUARDAR esta dirección en el XY7025:"));
        Serial.println(F("1. Use opción [w] del menú"));
        Serial.println(F("2. Apague y encienda manualmente el XY7025 (botón físico - NO reinicio por software)"));
        Serial.println(F("3. Verifique conexión con opción [a]"));
    }
}

bool testConnectionWithAddress(uint8_t address) {
    // Crear objeto temporal para probar dirección
    XY7025_Modbus tempMppt(mpptSerial, address);
    tempMppt.begin(BAUDRATES[currentBaudrate]);
    
    return tempMppt.readHoldingRegisters(XY7025_VOUT, 1);
}

//====================================================================
// BÚSQUEDA COMPLETA DE BAUDRATE
//====================================================================

void searchBaudrateComplete() {
    Serial.println(F("--- BÚSQUEDA EXHAUSTIVA DE BAUDRATE ---"));
    Serial.println(F("⚠️ ADVERTENCIA: Búsqueda exhaustiva"));
    Serial.println(F("Este proceso probará todas las combinaciones:"));
    Serial.println(F("  - 9 baudrates diferentes"));
    Serial.println(F("  - 247 direcciones slave por baudrate"));
    Serial.println(F("  - Tiempo estimado: 1-3 horas"));
    Serial.println();
    
    Serial.print(F("¿Desea continuar? (s/n): "));
    while (!Serial.available()) {
        delay(100);
    }
    char response = Serial.read();
    response = tolower(response);
    
    if (response != 's') {
        Serial.println(F("Búsqueda cancelada"));
        return;
    }
    
    Serial.println(F("\nIniciando búsqueda exhaustiva..."));
    searchCancelled = false;
    
    bool found = false;
    uint8_t foundBaudrate = 0;
    uint8_t foundAddress = 0;
    
    for (uint8_t baudIndex = 0; baudIndex < BAUDRATE_COUNT && !searchCancelled; baudIndex++) {
        Serial.println(F("\n--- Probando baudrate ") + String(baudIndex) + F("/") + String(BAUDRATE_COUNT-1) + 
                      F(" (") + String(BAUDRATES[baudIndex]) + F(" bps) ---"));
        
        // Cambiar baudrate
        mpptSerial.end();
        delay(100);
        mpptSerial.begin(BAUDRATES[baudIndex]);
        delay(200);
        
        // Buscar slave en este baudrate
        for (uint8_t addr = 1; addr <= 247 && !searchCancelled; addr++) {
            printProgress(addr, 247, F("Baudrate ") + String(baudIndex) + F(" - Probando slave"));
            
            if (mppt.probeSlaveAddress(addr)) {
                if (testConnectionWithAddressAndBaudrate(addr, BAUDRATES[baudIndex])) {
                    foundBaudrate = baudIndex;
                    foundAddress = addr;
                    found = true;
                    break;
                }
            }
            
            // Verificar cancelación
            if (Serial.available()) {
                char cancel = Serial.read();
                if (cancel == 'q' || cancel == 'Q') {
                    searchCancelled = true;
                    break;
                }
            }
            
            delay(50); // Pausa más corta para búsqueda más rápida
        }
        
        if (found) break;
    }
    
    if (searchCancelled) {
        Serial.println(F("\nBúsqueda cancelada por el usuario"));
    } else if (!found) {
        Serial.println(F("\n✗ No se encontró ningún dispositivo"));
        Serial.println(F("Verifique:"));
        Serial.println(F("  - XY7025 encendido y conectado"));
        Serial.println(F("  - Conexiones físicas correctas"));
        Serial.println(F("  - XY7025 funcionando correctamente"));
    } else {
        // Actualizar configuración
        currentBaudrate = foundBaudrate;
        currentSlaveAddress = foundAddress;
        
        mppt = XY7025_Modbus(mpptSerial, currentSlaveAddress);
        mppt.begin(BAUDRATES[currentBaudrate]);
        systemConnected = true;
        
        Serial.println(F("\n================================================"));
        Serial.println(F("✓ DISPOSITIVO ENCONTRADO"));
        Serial.println(F("  Slave Address: ") + String(foundAddress));
        Serial.println(F("  Baudrate: ") + String(BAUDRATES[foundBaudrate]) + F(" bps (índice ") + String(foundBaudrate) + F(")"));
        Serial.println(F("================================================"));
        
        Serial.println(F("\n⚠️ CAMBIOS DETECTADOS:"));
        Serial.println(F("Para que los cambios sean permanentes:"));
        Serial.println(F("1. Opción [w]: Guardar dirección slave"));
        Serial.println(F("2. Opción [r]: Guardar baudrate"));
        Serial.println(F("3. REINICIO MANUAL del XY7025 (botón físico)"));
        Serial.println(F("4. Verificar con opción [a]"));
    }
}

bool testConnectionWithAddressAndBaudrate(uint8_t address, uint32_t baudrate) {
    XY7025_Modbus tempMppt(mpptSerial, address);
    tempMppt.begin(baudrate);
    return tempMppt.readHoldingRegisters(XY7025_VOUT, 1);
}

//====================================================================
// ESCRITURA EN XY7025
//====================================================================

void writeSlaveToXY7025() {
    Serial.println(F("--- ESCRIBIR SLAVE EN XY7025 ---"));
    
    if (!systemConnected) {
        Serial.println(F("✗ No hay conexión establecida"));
        return;
    }
    
    Serial.println(F("Escribir dirección ") + String(currentSlaveAddress) + 
                  F(" en registro 0x") + String(XY7025_SLAVE_ADD, HEX));
    Serial.println();
    
    Serial.print(F("¿Confirmar? (s/n): "));
    while (!Serial.available()) {
        delay(100);
    }
    char response = Serial.read();
    response = tolower(response);
    
    if (response != 's') {
        Serial.println(F("Operación cancelada"));
        return;
    }
    
    Serial.println(F("\nEscribiendo en XY7025..."));
    
    bool success = mppt.writeRegister(XY7025_SLAVE_ADD, currentSlaveAddress);
    
    if (success) {
        Serial.println(F("✓ Escritura exitosa"));
        
        // Verificar escritura leyendo el registro
        uint16_t readBack = mppt.readRegister(XY7025_SLAVE_ADD);
        if (readBack == currentSlaveAddress) {
            Serial.println(F("✓ Verificación exitosa: valor guardado correctamente"));
        } else {
            Serial.println(F("⚠️ Advertencia: valor escrito pero verificación falló"));
        }
    } else {
        Serial.println(F("✗ Error escribiendo en XY7025"));
        return;
    }
    
    Serial.println(F("\n⚠️ REINICIO MANUAL REQUERIDO"));
    Serial.println(F("El cambio NO es efectivo hasta que:"));
    Serial.println(F"→ Apague y encienda el XY7025 con el botón físico");
    Serial.println(F"→ NO es posible reiniciar por software");
    
    Serial.println(F("\nPasos siguientes:"));
    Serial.println(F("1. REINICIE MANUALMENTE el XY7025"));
    Serial.println(F"2. Use opción [a] para verificar conexión"));
    Serial.println(F"3. Si falla, use opción [s] para buscar nueva dirección"));
}

void writeBaudrateToXY7025() {
    Serial.println(F("--- ESCRIBIR BAUDRATE EN XY7025 ---"));
    
    if (!systemConnected) {
        Serial.println(F("✗ No hay conexión establecida"));
        return;
    }
    
    Serial.println(F("Configuración actual:"));
    Serial.println(F("  Baudrate local: ") + String(BAUDRATES[currentBaudrate]) + F(" bps (índice ") + String(currentBaudrate) + F(")"));
    Serial.println();
    Serial.println(F("Tabla de baudrates disponibles:"));
    for (uint8_t i = 0; i < BAUDRATE_COUNT; i++) {
        Serial.println(F("  ") + String(i) + F(": ") + String(BAUDRATES[i]) + F(" bps") + 
                      (i == currentBaudrate ? F(" (ACTUAL)") : F("")));
    }
    Serial.println();
    
    Serial.print(F("¿Escribir baudrate ") + String(BAUDRATES[currentBaudrate]) + 
                F(" (índice ") + String(currentBaudrate) + F(") en registro 0x") + 
                String(XY7025_BAUDRATE_L, HEX) + F("? (s/n): "));
    
    while (!Serial.available()) {
        delay(100);
    }
    char response = Serial.read();
    response = tolower(response);
    
    if (response != 's') {
        Serial.println(F("Operación cancelada"));
        return;
    }
    
    Serial.println(F("\nEscribiendo en XY7025..."));
    
    bool success = mppt.writeRegister(XY7025_BAUDRATE_L, currentBaudrate);
    
    if (success) {
        Serial.println(F("✓ Escritura exitosa"));
        
        // Verificar escritura
        uint16_t readBack = mppt.readRegister(XY7025_BAUDRATE_L);
        if (readBack == currentBaudrate) {
            Serial.println(F("✓ Verificación exitosa: valor guardado correctamente"));
        } else {
            Serial.println(F("⚠️ Advertencia: valor escrito pero verificación falló"));
        }
    } else {
        Serial.println(F("✗ Error escribiendo en XY7025"));
        return;
    }
    
    Serial.println(F("\n⚠️ REINICIO MANUAL REQUERIDO"));
    Serial.println(F("El cambio NO es efectivo hasta que:"));
    Serial.println(F"→ Apague y encienda el XY7025 con el botón físico");
    Serial.println(F"→ NO es posible reiniciar por software"));
    
    Serial.println(F("\nPasos siguientes:"));
    Serial.println(F("1. REINICIE MANUALMENTE el XY7025"));
    Serial.println(F"2. El baudrate cambiará automáticamente"));
    Serial.println(F"3. Use opción [a] para verificar conexión"));
}

//====================================================================
// CAMBIOS LOCALES
//====================================================================

void changeLocalSlave() {
    Serial.println(F("--- CAMBIAR SLAVE LOCAL (Arduino) ---"));
    
    Serial.println(F("Configuración actual:"));
    Serial.println(F"  Slave Address local: ") + String(currentSlaveAddress);
    Serial.println(F"  Baudrate local: ") + String(BAUDRATES[currentBaudrate]) + F(" bps");
    Serial.println();
    
    Serial.print(F("Ingrese nueva dirección slave (1-247): "));
    
    int newAddress = -1;
    while (newAddress < 0) {
        while (!Serial.available()) {
            delay(100);
        }
        newAddress = Serial.parseInt();
        
        if (newAddress < 1 || newAddress > 247) {
            Serial.println(F("Dirección inválida. Ingrese valor entre 1-247: "));
            newAddress = -1;
            // Limpiar buffer
            while (Serial.available()) {
                Serial.read();
            }
        }
    }
    
    Serial.println(F("Dirección ingresada: ") + String(newAddress));
    
    // Actualizar configuración local
    currentSlaveAddress = (uint8_t)newAddress;
    mppt = XY7025_Modbus(mpptSerial, currentSlaveAddress);
    
    Serial.println(F("✓ Slave local actualizado a ") + String(currentSlaveAddress));
    Serial.println(F("ℹ️ Cambio LOCAL únicamente - Solo afecta al Arduino"));
    Serial.println(F"ℹ️ Para guardar en XY7025 use opción [w]"));
    
    // Probar nueva configuración
    Serial.println(F("Probando nueva configuración..."));
    systemConnected = testConnection();
    
    if (systemConnected) {
        Serial.println(F("✓ Nueva configuración funciona correctamente"));
    } else {
        Serial.println(F"⚠️ Nueva configuración no funciona");
        Serial.println(F"  - Verifique que la dirección sea correcta"));
        Serial.println(F"  - Si es necesario, restaure con opción [a]"));
    }
}

void changeLocalBaudrate() {
    Serial.println(F("--- CAMBIAR BAUDRATE LOCAL (Arduino) ---"));
    
    Serial.println(F("Configuración actual:"));
    Serial.println(F"  Slave Address: ") + String(currentSlaveAddress);
    Serial.println(F"  Baudrate local: ") + String(BAUDRATES[currentBaudrate]) + F(" bps (índice ") + String(currentBaudrate) + F(")");
    Serial.println();
    
    Serial.println(F("Tabla de baudrates disponibles:"));
    for (uint8_t i = 0; i < BAUDRATE_COUNT; i++) {
        Serial.println(F("  ") + String(i) + F(": ") + String(BAUDRATES[i]) + F(" bps") + 
                      (i == currentBaudrate ? F(" (ACTUAL)") : F("")));
    }
    Serial.println();
    
    Serial.print(F("Ingrese nuevo índice de baudrate (0-") + String(BAUDRATE_COUNT-1) + F("): "));
    
    int newBaudrate = -1;
    while (newBaudrate < 0) {
        while (!Serial.available()) {
            delay(100);
        }
        newBaudrate = Serial.parseInt();
        
        if (newBaudrate < 0 || newBaudrate >= BAUDRATE_COUNT) {
            Serial.println(F("Índice inválido. Ingrese valor entre 0-") + String(BAUDRATE_COUNT-1) + F(": "));
            newBaudrate = -1;
            while (Serial.available()) {
                Serial.read();
            }
        }
    }
    
    Serial.println(F("Baudrate seleccionado: ") + String(BAUDRATES[newBaudrate]) + F(" bps (índice ") + String(newBaudrate) + F(")"));
    
    // Cambiar baudrate de comunicación
    Serial.println(F("Cambiando baudrate de comunicación..."));
    mpptSerial.end();
    delay(100);
    mpptSerial.begin(BAUDRATES[newBaudrate]);
    delay(200);
    
    // Actualizar configuración local
    currentBaudrate = (uint8_t)newBaudrate;
    mppt.begin(BAUDRATES[currentBaudrate]);
    
    Serial.println(F("✓ Baudrate local actualizado"));
    Serial.println(F("ℹ️ Cambio LOCAL únicamente - Solo afecta al Arduino"));
    Serial.println(F"ℹ️ Para guardar en XY7025 use opción [r]"));
    
    // Probar nueva configuración
    Serial.println(F("Probando nueva configuración..."));
    systemConnected = testConnection();
    
    if (systemConnected) {
        Serial.println(F("✓ Nueva configuración funciona correctamente"));
    } else {
        Serial.println(F"⚠️ Nueva configuración no funciona"));
        Serial.println(F"  - Verifique que el baudrate sea correcto"));
        Serial.println(F"  - Use búsqueda de baudrate si es necesario"));
    }
}

//====================================================================
// AYUDA Y UTILIDADES
//====================================================================

void showHelp() {
    Serial.println(F("=== AYUDA - CONFIGURADOR XY7025 ==="));
    Serial.println();
    Serial.println(F("FLUJO RECOMENDADO:"));
    Serial.println();
    Serial.println(F("1. Si NO HAY CONEXIÓN:"));
    Serial.println(F"   [s] → Buscar Slave"));
    Serial.println(F"   [b] → Buscar Baudrate (si [s] falla)"));
    Serial.println();
    Serial.println(F"2. Para CAMBIAR CONFIGURACIÓN:"));
    Serial.println(F"   [m]/[n] → Cambiar localmente"));
    Serial.println(F"   [a] → Verificar que funciona"));
    Serial.println(F"   [w]/[r] → Guardar en XY7025"));
    Serial.println(F"   REINICIAR MANUALMENTE"));
    Serial.println(F"   [a] → Verificar nuevamente"));
    Serial.println();
    Serial.println(F"3. RECUPERACIÓN DE EMERGENCIA:"));
    Serial.println(F"   Si pierde comunicación:"));
    Serial.println(F"   [b] → Búsqueda exhaustiva"));
    Serial.println();
    Serial.println(F("OPCIONES DISPONIBLES:"));
    Serial.println(F("  [a] Verificar: Prueba conexión y lee configuración actual"));
    Serial.println(F"  [s] Buscar Slave: Busca automáticamente dirección 1-247"));
    Serial.println(F"  [b] Buscar Baudrate: Prueba todos los baudrates (proceso largo)"));
    Serial.println(F"  [w] Escribir Slave: Guarda dirección en registro 0x0018 del XY7025"));
    Serial.println(F"  [r] Escribir Baudrate: Guarda baudrate en registro 0x0019 del XY7025"));
    Serial.println(F"  [m] Cambiar Local: Cambia dirección solo en Arduino (no en XY7025)"));
    Serial.println(F"  [n] Cambiar Local: Cambia baudrate solo en Arduino (no en XY7025)"));
    Serial.println(F("  [h] Ayuda: Muestra esta ayuda"));
    Serial.println(F("  [q] Salir: Termina el programa"));
    Serial.println();
    Serial.println(F("IMPORTANTE:"));
    Serial.println(F"  - Los cambios en XY7025 requieren REINICIO MANUAL (botón físico)"));
    Serial.println(F"  - Presione 'q' durante búsquedas para cancelar"));
    Serial.println(F"  - Use [d] para activar/desactivar modo debug"));
    Serial.println();
    Serial.println(F("CONEXIONES:"));
    Serial.println(F"  Arduino Pin 2 (RX) → XY7025 Pin RX"));
    Serial.println(F"  Arduino Pin 3 (TX) → XY7025 Pin TX"));
    Serial.println(F"  GND Arduino → GND XY7025"));
}

String getBaudrateName(uint8_t index) {
    if (index >= BAUDRATE_COUNT) return F("Desconocido");
    return String(BAUDRATES[index]) + F(" bps");
}

void printProgress(uint8_t current, uint8_t total, String prefix) {
    Serial.print(F("\r") + prefix + F(" ") + String(current) + F("/") + String(total) + F(" (") + 
                String((current * 100) / total) + F("%)"));
    Serial.flush();
}

//====================================================================
// FIN DEL SKETCH
//====================================================================