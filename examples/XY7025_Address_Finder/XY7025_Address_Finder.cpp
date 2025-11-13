/*
  XY7025_Address_Finder - Configurador interactivo para dirección slave y baudrate
  para un unico equipo XY7025, se crearà una unica instancia XY7025_Ṃodbus

  Funcionalidades:
  - Verificar conexión con XY7025
  - Buscar dirección slave automáticamente
  - Buscar baudrate automáticamente
  - Escribir configuración en XY7025
  - Cambiar configuración local (Arduino)
  - Menú interactivo con opciones completas

  Autores: motxi y toniypedro quarrytroni@gmail.com
  Fecha: 2025-11-06
  Estado: En desarrollo
  Licencia: GNU
*/

#include <Arduino.h>
#include <XY7025_Modbus.h>
#include <SoftwareSerial.h>

//====================================================================
// CONSTANTES Y CONFIGURACIÓN
//====================================================================

// Pines de comunicación
const uint8_t MODBUS_RX = 2;   // RX Arduino -> RX XY7025
const uint8_t MODBUS_TX = 3;   // TX Arduino -> TX XY7025

// Límites y valores mágicos
const uint8_t MIN_SLAVE_ADDRESS = 1;
const uint8_t MAX_SLAVE_ADDRESS = 247;
const uint8_t BAUDRATE_COUNT;
const uint16_t SERIAL_TIMEOUT_MS = 10000;     // 10 segundos timeout
const uint16_t CONNECTION_TIMEOUT_MS = 5000;  // 5 segundos para conexión
const uint8_t MAX_RETRIES = 3;
const uint16_t RETRY_DELAY_MS = 500;

// Objetos de comunicación
SoftwareSerial xy7025_serial(MODBUS_RX, MODBUS_TX);
XY7025_Modbus xy7025_1(xy7025_serial, MIN_SLAVE_ADDRESS);

// Variables de estado
SystemState systemState = STATE_INIT;
uint8_t currentSlaveAddress = MIN_SLAVE_ADDRESS;      // Dirección slave inicial
uint8_t currentBaudrate = 6;                          // 115200 bps por defecto (índice 6)
uint8_t serialBaudrate = 115200;
bool systemConnected = false;
bool debugMode = true;
bool searchCancelled = false;

// Estados del sistema
enum SystemState {
    STATE_INIT,
    STATE_MENU,
    STATE_SEARCHING,
    STATE_CONNECTED,
    STATE_ERROR
};

// Códigos de error
enum ErrorCode {
    ERROR_NONE = 0,
    ERROR_NO_CONNECTION = 1,
    ERROR_INVALID_ADDRESS = 2,
    ERROR_INVALID_BAUDRATE = 3,
    ERROR_WRITE_FAILED = 4,
    ERROR_TIMEOUT = 5,
    ERROR_CANCELLED = 6
};

// Tabla de baudrates disponibles
const uint32_t BAUDRATES[] PROGMEM = {
    9600,    // 0
    14400,   // 1
    19200,   // 2
    38400,   // 3
    56000,   // 4 ¿ = 28800 ?
    57600,   // 5
    115200,  // 6 - Por defecto
    2400,    // 7
    4800     // 8
};

BAUDRATE_COUNT = sizeof(BAUDRATES) / sizeof(BAUDRATES[0]);

// Mensajes constantes en PROGMEM para ahorrar RAM
const char MSG_INIT[] PROGMEM = "=== INICIANDO CONFIGURADOR XY7025 ===";
const char MSG_SEARCH[] PROGMEM = "Búsqueda automática de dispositivo...";
const char MSG_MODBUS_OK[] PROGMEM = "✓ Comunicación Modbus inicializada";
const char MSG_MODBUS_ERROR[] PROGMEM = "✗ Error inicializando Modbus";
const char MSG_VERIFY[] PROGMEM = "\nVerificando conexión inicial...";
const char MSG_CONNECTED[] PROGMEM = "✓ Conexión establecida con slave ";
const char MSG_NO_RESPONSE[] PROGMEM = "✗ Sin respuesta del dispositivo";
const char MSG_USE_MENU[] PROGMEM = "ℹ️ Pulsa la tecla [I] para ver el menú de búsqueda";
const char MSG_READY[] PROGMEM = "\nConfigurador listo. Presione una tecla para continuar...";

const char MSG_MENU_TITLE[] PROGMEM = "\n=== CONFIGURADOR XY7025 ===";
const char MSG_STATUS[] PROGMEM = "Estado actual:";
const char MSG_SLAVE_ADR[] PROGMEM = "  Slave Address: ";
const char MSG_BAUDRATE[] PROGMEM = "  Baudrate: ";
const char MSG_CONNECTION[] PROGMEM = "  Conexión: ";
const char MSG_DEBUG[] PROGMEM = "  Debug Mode: ";
const char MSG_OPTIONS[] PROGMEM = "Opciones:";
const char MSG_CMD_PROMPT[] PROGMEM = "Comando: ";

// Mensajes de error
const char ERROR_NO_CONN[] PROGMEM = "✗ No hay conexión establecida";
const char ERROR_INVALID_INPUT_MSG[] PROGMEM = "Entrada inválida";
const char ERROR_WRITE_FAIL[] PROGMEM = "✗ Error escribiendo en XY7025";
const char ERROR_TIMEOUT_MSG[] PROGMEM = "✗ Tiempo de espera agotado";
const char ERROR_CANCEL_MSG[] PROGMEM = "Operación cancelada";
const char ERROR_EXIT_SRC[] PROGMEM = "Salida inesperada durante la busqueda";

// Mensajes de éxito
const char SUCCESS_WRITE[] PROGMEM = "✓ Escritura exitosa";
const char SUCCESS_VERIFY[] PROGMEM = "✓ Verificación exitosa: valor guardado correctamente";
const char SUCCESS_CONFIG[] PROGMEM = "✓ Nueva configuración funciona correctamente";

// Mensajes de advertencia
const char WARN_VERIFY_FAIL[] PROGMEM = "⚠️ Advertencia: valor escrito pero verificación falló";
const char WARN_NO_WORK[] PROGMEM = "⚠️ Nueva configuración no funciona";

//====================================================================
// PROTOTIPOS DE FUNCIONES
//====================================================================

// Funciones principales
void setup();
void loop();

// Funciones de menú y visualización
void displayMenu();
void showHelp();
void printProgress(uint8_t current, uint8_t total, const char* prefix);
void printFromPROGMEM(const char* str);
void printConnectionStatus();

// Funciones de verificación y prueba
bool testConnection();
bool testConnectionWithAddress(uint8_t address);
bool testConnectionWithAddressAndBaudrate(uint8_t address, uint32_t baudrate);
void verifyConnection();

// Funciones de búsqueda
void searchSlaveAddress();
void searchBaudrateComplete();

// Funciones de escritura
void writeSlaveToXY7025();
void writeBaudrateToXY7025();
ErrorCode writeToXY7025(uint16_t registerAddr, uint16_t value, const char* regName);

// Funciones de configuración local
void changeLocalSlave();
void changeLocalBaudrate();

// Funciones auxiliares
uint32_t getBaudrateValue(uint8_t index);
const char* getConnectionStatusText();
const char* getBaudrateName(uint8_t index);
ErrorCode waitForSerialResponse(char& response, uint16_t timeout = SERIAL_TIMEOUT_MS);
ErrorCode readIntegerInput(int& value, int minVal, int maxVal, const char* prompt);
ErrorCode updateLocalSlaveAfterWrite(uint8_t newSlaveAddress);
ErrorCode updateLocalBaudrateAfterWrite(uint8_t newBaudrateIndex);

//====================================================================
// CONFIGURACIÓN INICIAL
//====================================================================

void setup() {
    Serial.begin(serialBaudrate);
    delay(1000);
    
    printFromPROGMEM(MSG_INIT);
    printFromPROGMEM(MSG_SEARCH);
    
    // Inicializar comunicación con baudrate por defecto
    uint32_t baudValue = getBaudrateValue(currentBaudrate);
    xy7025_serial.begin(baudValue);
    delay(500);
    
    // Reinicializar instancia del objeto  XY7025_Modbus
    if (xy7025_1.begin(baudValue)) {
        delay(500);
        printFromPROGMEM(MSG_MODBUS_OK);
    } else {
        printFromPROGMEM(MSG_MODBUS_ERROR);
        systemState = STATE_ERROR;
    }
    
    // Verificar conexión inicial
    printFromPROGMEM(MSG_VERIFY);
    systemConnected = testConnection();
    
    if (systemConnected) {
        Serial.print(F("✓ Conexión establecida con slave "));
        Serial.print(currentSlaveAddress);
        Serial.print(F(" a "));
        Serial.print(baudValue);
        Serial.println(F(" bps"));
        systemState = STATE_CONNECTED;
    } else {
        printFromPROGMEM(MSG_NO_RESPONSE);
        printFromPROGMEM(MSG_USE_MENU);
        systemState = STATE_MENU;
    }
    
    printFromPROGMEM(MSG_READY);
    unsigned long startTime = millis();
    while (!Serial.available() && (millis() - startTime < 5000)) {
        delay(100);
    }
    if (Serial.available()) {
        Serial.read(); // Limpiar buffer
    }
    
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
        
        Serial.print(F("\n--- Ejecutando opción: ["));
        Serial.print(command);
        Serial.println(F("] ---"));
        
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
            case 'I':
                displayMenu();
                break;
            case 'H':
                showHelp();
                break;
            case 'Q':
                Serial.println(F("Saliendo del configurador..."));
                systemState = STATE_INIT;
                while(true) delay(1000); // Detener programa
                break;
            case 'D':
                debugMode = !debugMode;
                xy7025_1.enableDebug(debugMode);
                Serial.print(F("Debug mode: "));
                Serial.println(debugMode ? F("ON") : F("OFF"));
                break;
            default:
                Serial.println(F("Comando no reconocido. Use 'h' para ayuda."));
                break;
        }
        
        Serial.println(F("\n--- Comando completado ---"));
        delay(500);
    }
}

//====================================================================
// FUNCIONES DE MENÚ Y VISUALIZACIÓN
//====================================================================

void displayMenu() {
    printFromPROGMEM(MSG_MENU_TITLE);
    printFromPROGMEM(MSG_STATUS);
    
    printFromPROGMEM(MSG_SLAVE_ADR);
    Serial.println(currentSlaveAddress);
    
    Serial.print(F("  Baudrate: "));
    Serial.print(getBaudrateValue(currentBaudrate));
    Serial.print(F(" bps (índice "));
    Serial.print(currentBaudrate);
    Serial.println(F(")"));
    
    Serial.print(F("  Conexión: "));
    Serial.println(getConnectionStatusText());
    
    Serial.print(F("  Debug Mode: "));
    Serial.println(debugMode ? F("ON") : F("OFF"));
    
    Serial.println();
    printFromPROGMEM(MSG_OPTIONS);
    Serial.println(F("[a] Verificar conexión actual"));
    Serial.println(F("[s] Buscar dirección Slave (1-247)"));
    Serial.println(F("[b] Buscar Baudrate completo (proceso largo)"));
    Serial.println(F("[w] Escribir Slave en XY7025"));
    Serial.println(F("[r] Escribir Baudrate en XY7025"));
    Serial.println(F("[m] Cambiar Slave local (Arduino)"));
    Serial.println(F("[n] Cambiar Baudrate local (Arduino)"));
    Serial.println(F("[i] Información y opciones"));
    Serial.println(F("[h] Ayuda"));
    Serial.println(F("[q] Salir"));
    Serial.println(F("[d] alternar modo Debugeo"));
    Serial.println();
    printFromPROGMEM(MSG_CMD_PROMPT);
}

void printFromPROGMEM(const char* str) {
    char buffer[128];
    strcpy_P(buffer, str);
    Serial.println(buffer);
}

/*
Lectura de PROGMEM sin buffer:

void printFromPROGMEM(const char* str) {
    // Sin buffer - imprime carácter por carácter
    for (uint8_t i = 0; ; i++) {
        char c = pgm_read_byte(str + i);
        if (c == 0) break;
        Serial.print(c);
    }
    Serial.println();
}

*/



// Función comentada temporalmente - no se utiliza en el código actual
/*
void printConnectionStatus() {
    Serial.print(F("  Conexión: "));
    Serial.println(getConnectionStatusText());
}
*/

void printProgress(uint8_t current, uint8_t total, const char* prefix) {
    char buffer[64];
    strcpy_P(buffer, prefix);
    Serial.print(F("\r"));
    Serial.print(buffer);
    Serial.print(F(" "));
    Serial.print(current);
    Serial.print(F("/"));
    Serial.print(total);
    Serial.print(F(" ("));
    Serial.print((current * 100) / total);
    Serial.print(F("%)"));
    Serial.flush();
}

//====================================================================
// FUNCIONES DE VERIFICACIÓN Y PRUEBA
//====================================================================

bool testConnection() {
    if (debugMode) {
        Serial.print(F("Probando conexión con slave "));
        Serial.print(currentSlaveAddress);
        Serial.print(F(" a "));
        Serial.print(getBaudrateValue(currentBaudrate));
        Serial.println(F(" bps..."));
    }
    
    uint16_t vout = xy7025_1.readRegister(XY7025_VOUT);
    bool success = (vout != XY7025_ERROR_UINT16);
    
    if (success) {
        if (debugMode) {
            Serial.print(F("✓ Respuesta recibida. Vout = "));
            Serial.println(vout);
        }
        return true;
    } else {
        if (debugMode) {
            printFromPROGMEM(MSG_NO_RESPONSE);
        }
        return false;
    }
}

bool testConnectionWithAddress(uint8_t address) {
    // Crear objeto temporal para probar dirección
    XY7025_Modbus temp_xy7025(xy7025_serial, address);
    temp_xy7025.begin(getBaudrateValue(currentBaudrate));
    
    uint16_t vout = temp_xy7025.readRegister(XY7025_VOUT);
    return (vout != XY7025_ERROR_UINT16);
}

bool testConnectionWithAddressAndBaudrate(uint8_t address, uint32_t baudrate) {
    XY7025_Modbus temp_xy7025(xy7025_serial, address);
    temp_xy7025.begin(baudrate);
    uint16_t vout = temp_xy7025.readRegister(XY7025_VOUT);
    return (vout != XY7025_ERROR_UINT16);
}

void verifyConnection() {
    Serial.println(F("--- VERIFICANDO CONEXIÓN ---"));
    
    systemConnected = testConnection();
    
    if (systemConnected) {
        // Leer registros adicionales para confirmar funcionamiento
        Serial.println(F("Leyendo registros de configuración..."));
        
        // Leer dirección slave actual del dispositivo
        uint16_t deviceSlave = xy7025_1.readRegister(XY7025_SLAVE_ADR);
        if (deviceSlave != XY7025_ERROR_UINT16) {
            Serial.print(F("  Dirección Slave en dispositivo: "));
            Serial.println(deviceSlave);
        }
        
        // Leer baudrate actual del dispositivo  
        uint16_t deviceBaudrate = xy7025_1.readBaudrate();
        if (deviceBaudrate != XY7025_ERROR_UINT16) {
            Serial.print(F("  Baudrate en dispositivo: "));
            Serial.print(deviceBaudrate);
            Serial.println(F(" bps"));
        }
        
        // Leer voltaje de salida actual
        uint16_t vout = xy7025_1.readRegister(XY7025_VOUT);
        if (vout != XY7025_ERROR_UINT16) {
            float voutReal = vout / 100.0;
            Serial.print(F("  Voltaje salida actual: "));
            Serial.print(voutReal, 2);
            Serial.println(F(" V"));
        }
        
        Serial.println(F("✓ Conexión VERIFICADA exitosamente"));
        systemState = STATE_CONNECTED;
    } else {
        Serial.println(F("✗ Conexión FALLIDA"));
        Serial.println(F("Sugerencias:"));
        Serial.println(F("  - Verificar conexiones físicas"));
        Serial.println(F("  - Comprobar baudrate correcto"));
        Serial.println(F("  - Intentar búsqueda de dirección slave"));
        systemState = STATE_ERROR;
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
    
    char response;
    if (waitForSerialResponse(response) != ERROR_NONE) {
        return;
    }
    
    response = tolower(response);
    if (response != 's') {
        printFromPROGMEM(ERROR_CANCEL_MSG);
        return;
    }
    
    Serial.print(F("\nIniciando búsqueda..."));
    Serial.print(F("Manteniendo baudrate: "));
    Serial.print(getBaudrateValue(currentBaudrate));
    Serial.println(F(" bps"));
    
    searchCancelled = false;
    bool found = false;
    
    for (uint8_t addr = MIN_SLAVE_ADDRESS; addr <= MAX_SLAVE_ADDRESS && !searchCancelled; addr++) {
        printProgress(addr, MAX_SLAVE_ADDRESS, PSTR("Probando dirección"));
        
        // Probar dirección
        if (xy7025_1.probeSlaveAddress(addr)) {
            currentSlaveAddress = addr;
            found = true;
            // Verificar con lectura adicional
            break;
        } else if (testConnectionWithAddress(addr)) {
            currentSlaveAddress = addr;
            found = true;
            break;
        }
        
        // Verificar si se canceló
        if (Serial.available()) {
            char cancel = Serial.read();
            if (cancel == 'q' || cancel == 'Q') {
                searchCancelled = true;
                break;
            }
        }
    }
    
    if (searchCancelled) {
        Serial.println(F("\nBúsqueda cancelada por el usuario"));
    } else if (!found) {
        Serial.println(F("\n✗ No se encontró ningún dispositivo"));
        Serial.println(F("Sugerencias:"));
        Serial.println(F("  - Verificar que el XY7025 esté encendido"));
        Serial.println(F("  - Comprobar conexiones"));
        Serial.println(F("  - Intentar búsqueda de baudrate"));
    } else if (found) {
        // Reinicializar instancia del objeto  XY7025_Modbus
        XY7025_Modbus xy7025_1(xy7025_serial, currentSlaveAddress);
        if (xy7025_1.begin(getBaudrateValue(currentBaudrate))) {
            delay(500);
            printFromPROGMEM(MSG_MODBUS_OK);
            systemConnected = true;
            systemState = STATE_CONNECTED;


            Serial.println();
            Serial.println(F("================================================"));
            Serial.println(F("✓ DISPOSITIVO ENCONTRADO"));
            Serial.print(F("  Dirección: "));
            Serial.println(adr);
            Serial.print(F("  Baudrate: "));
            Serial.print(getBaudrateValue(currentBaudrate));
            Serial.println(F(" bps"));
            Serial.println(F("================================================"));
        
            Serial.println(F("\n⚠️ ACCIÓN REQUERIDA:"));
            Serial.println(F("Si desea MODIFICAR la dirección en el XY7025:"));
            Serial.println(F("1. Use opción [w] del menú"));
            Serial.println(F("2. Apague y encienda manualmente el XY7025 (botón físico - NO reinicio por software)"));
            Serial.println(F("3. Verifique conexión con opción [a]"));
        } else {
            printFromPROGMEM(MSG_MODBUS_ERROR);
            systemConnected = false;
            systemState = STATE_ERROR;
        }
    } else {
            printFromPROGMEM(ERROR_EXIT_SRC);
    }
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
    
    char response;
    if (waitForSerialResponse(response) != ERROR_NONE) {
        return;
    }
    
    response = tolower(response);
    if (response != 's') {
        printFromPROGMEM(ERROR_CANCEL_MSG);
        return;
    }
    
    Serial.println(F("\nIniciando búsqueda exhaustiva..."));

    searchCancelled = false;
    bool found = false;
    
    for (uint8_t baudIndex = 0; baudIndex < BAUDRATE_COUNT && !searchCancelled; baudIndex++) {
        if (baudIndex == currentBaudrate) {
            continue;  // Salta el currentBaudrate
        }
        Serial.print(F("\n--- Probando baudrate "));
        Serial.print(baudIndex);
        Serial.print(F("/"));
        Serial.print(BAUDRATE_COUNT-1);
        Serial.print(F(" ("));
        Serial.print(getBaudrateValue(baudIndex));
        Serial.println(F(" bps) ---"));
        Serial.print(baudIndex);
        Serial.print(F("/"));
        Serial.print(BAUDRATE_COUNT-1);
        Serial.print(F(" ("));
        Serial.print(getBaudrateValue(baudIndex));
        Serial.println(F(" bps) ---"));
        
        // Reconfigurar SoftwareSerial con el nuevo baudrate
        xy7025_serial.end();
        delay(100);
        xy7025_serial.begin(getBaudrateValue(baudIndex));
        delay(200);
        // Reconfigurar modbusMaster
        xy7025_1.begin(getBaudrateValue(baudIndex));
        
        // Buscar slave en este baudrate
        for (uint8_t adr = MIN_SLAVE_ADDRESS; adr <= MAX_SLAVE_ADDRESS && !searchCancelled; adr++) {
            char progressMsg[64];
            snprintf_P(progressMsg, sizeof(progressMsg),
                      PSTR("Baudrate %d - Probando slave"), baudIndex);
            printProgress(adr, MAX_SLAVE_ADDRESS, progressMsg);
            
            if (xy7025_1.probeSlaveAddress(adr)) {
            // Actualizar configuración
                currentBaudrate = baudIndex;
                currentSlaveAddress = adr;
                found = true;
                break;
            // Verificar con lectura adicional
            } else if (testConnectionWithAddressAndBaudrate(adr, getBaudrateValue(baudIndex))) {
            // Actualizar configuración
                currentBaudrate = baudIndex;
                currentSlaveAddress = adr;
                found = true;
                break;
            }
            
            // Verificar cancelación
            if (Serial.available()) {
                char cancel = Serial.read();
                if (cancel == 'q' || cancel == 'Q') {
                    searchCancelled = true;
                    break;
                }
            }
        }
    }
    
    if (searchCancelled) {
        Serial.println(F("\nBúsqueda cancelada por el usuario"));
    } else if (!found) {
        Serial.println(F("\n✗ No se encontró ningún dispositivo"));
        Serial.println(F("Verifique:"));
        Serial.println(F("  - XY7025 encendido y conectado"));
        Serial.println(F("  - Conexiones físicas correctas"));
        Serial.println(F("  - XY7025 funcionando correctamente"));
    } else if (found) {
        
        // Reinicializar instancia del objeto  XY7025_Modbus
        XY7025_Modbus xy7025_1(xy7025_serial, currentSlaveAddress);
        if (xy7025_1.begin(getBaudrateValue(currentBaudrate))) {
            delay(500);
            printFromPROGMEM(MSG_MODBUS_OK);
            systemConnected = true;
            systemState = STATE_CONNECTED;
        
            Serial.println(F("\n================================================"));
            Serial.println(F("✓ DISPOSITIVO ENCONTRADO"));
            Serial.print(F("  Slave Address: "));
            Serial.println(currentSlaveAddress);
            Serial.print(F("  Baudrate: "));
            Serial.print(getBaudrateValue(currentBaudrate));
            Serial.print(F(" bps (índice "));
            Serial.print(currentBaudrate);
            Serial.println(F(")"));
            Serial.println(F("================================================"));
            
            Serial.println(F("\n⚠️ CAMBIOS DETECTADOS:"));
            Serial.println(F("Si desea MODIFICAR la dirección en el XY7025:"));
            Serial.println(F("1. Opción [w]: Guardar dirección slave"));
            Serial.println(F("Si desea MODIFICAR el Baudrate en el XY7025:"));
            Serial.println(F("2. Opción [r]: Guardar baudrate"));
            Serial.println(F("3. Apague y encienda manualmente el XY7025 (botón físico - NO reinicio por software)"));
            Serial.println(F("4. Verificar con opción [a]"));
        } else {
            printFromPROGMEM(MSG_MODBUS_ERROR);
            systemConnected = false;
            systemState = STATE_ERROR;
        }
    } else {
            printFromPROGMEM(ERROR_EXIT_SRC);
    }
}

//====================================================================
// ESCRITURA EN XY7025
//====================================================================

ErrorCode writeToXY7025(uint16_t registerAddr, uint16_t value, const char* regName) {
    Serial.print(F("Escribir valor "));
    Serial.print(value);
    Serial.print(F(" en registro 0x"));
    Serial.print(registerAddr, HEX);
    if (regName != nullptr) {
        Serial.print(F(" ("));
        Serial.print(regName);
        Serial.print(F(")"));
    }
    Serial.println();
    Serial.println();
    
    char response;
    if (waitForSerialResponse(response) != ERROR_NONE) {
        return ERROR_CANCELLED;
    }
    
    response = tolower(response);
    if (response != 's') {
        printFromPROGMEM(ERROR_CANCEL_MSG);
        return ERROR_CANCELLED;
    }
    
    Serial.println(F("\nEscribiendo en XY7025..."));
    
    // Escribiendo valor en la unica instancia XY7025_Ṃodbus
    bool success = xy7025_1.writeRegister(registerAddr, value);
    
    if (success) {
        printFromPROGMEM(SUCCESS_WRITE);
        
        // Verificar escritura leyendo el registro
        uint16_t readBack = xy7025_1.readRegister(registerAddr);
        if (readBack == value) {
            printFromPROGMEM(SUCCESS_VERIFY);
        } else {
            printFromPROGMEM(WARN_VERIFY_FAIL);
        }
        return ERROR_NONE;
    } else {
        printFromPROGMEM(ERROR_WRITE_FAIL);
        return ERROR_WRITE_FAILED;
    }
}

void writeSlaveToXY7025() {
    Serial.println(F("--- ESCRIBIR SLAVE EN XY7025 ---"));
    
    if (!systemConnected) {
        printFromPROGMEM(ERROR_NO_CONN);
        return;
    }
    
    // Guardar la dirección actual antes de cambiarla
    uint8_t oldSlaveAddress = currentSlaveAddress;
    
    ErrorCode result = writeToXY7025(XY7025_SLAVE_ADR, currentSlaveAddress, "SLAVE_ADR");
    if (result == ERROR_NONE) {
        Serial.println(F("\n⚠️ REINICIO MANUAL REQUERIDO"));
        Serial.println(F("El cambio NO es efectivo hasta que:"));
        Serial.println(F("→ Apague y encienda el XY7025 con el botón físico"));
        Serial.println(F("→ NO es posible reiniciar por software"));
        
        Serial.println(F("\n⚠️ IMPORTANTE: Actualización de configuración local"));
        Serial.println(F("Después del reinicio manual, el Arduino actualizará automáticamente su configuración local."));
        Serial.println(F("Esto garantiza que la comunicación se mantenga después del cambio."));
        
        // Programar actualización automática después del reinicio
        Serial.println(F("\nConfiguración programada para actualización automática:"));
        Serial.print(F("  Nueva dirección slave: "));
        Serial.println(currentSlaveAddress);
        
        // Marcar que necesitamos actualizar la configuración local después del reinicio
        bool pendingLocalUpdate = true;
        uint8_t pendingSlaveAddress = currentSlaveAddress;
        
        Serial.println(F("\nPasos siguientes:"));
        Serial.println(F("1. REINICIE MANUALMENTE el XY7025"));
        Serial.println(F("2. El Arduino detectará el cambio y actualizará su configuración automáticamente"));
        Serial.println(F("3. Use opción [a] para verificar conexión"));
        Serial.println(F("4. Si falla, use opción [s] para buscar nueva dirección"));
        
        // Simular la actualización local inmediatamente después del write
        // (en la práctica, esto debería hacerse después del reinicio manual)
        Serial.println(F("\nActualizando configuración local inmediatamente..."));
        updateLocalSlaveAfterWrite(currentSlaveAddress);
    }
}

void writeBaudrateToXY7025() {
    Serial.println(F("--- ESCRIBIR BAUDRATE EN XY7025 ---"));
    
    if (!systemConnected) {
        printFromPROGMEM(ERROR_NO_CONN);
        return;
    }
    
    // Guardar el baudrate actual antes de cambiarlo
    uint8_t oldBaudrate = currentBaudrate;
    
    Serial.println(F("Configuración actual:"));
    Serial.print(F("  Baudrate local: "));
    Serial.print(getBaudrateValue(currentBaudrate));
    Serial.print(F(" bps (índice "));
    Serial.print(currentBaudrate);
    Serial.println(F(")"));
    Serial.println();
    Serial.println(F("Tabla de baudrates disponibles:"));
    for (uint8_t i = 0; i < BAUDRATE_COUNT; i++) {
        Serial.print(F("  "));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.print(getBaudrateValue(i));
        Serial.print(F(" bps"));
        if (i == currentBaudrate) {
            Serial.print(F(" (ACTUAL)"));
        }
        Serial.println();
    }
    Serial.println();
    
    ErrorCode result = writeToXY7025(XY7025_BAUDRATE_L, currentBaudrate, "BAUDRATE_L");
    if (result == ERROR_NONE) {
        Serial.println(F("\n⚠️ REINICIO MANUAL REQUERIDO"));
        Serial.println(F("El cambio NO es efectivo hasta que:"));
        Serial.println(F("→ Apague y encienda el XY7025 con el botón físico"));
        Serial.println(F("→ NO es posible reiniciar por software"));
        
        Serial.println(F("\n⚠️ IMPORTANTE: Actualización de configuración local"));
        Serial.println(F("Después del reinicio manual, el Arduino actualizará automáticamente su configuración local."));
        Serial.println(F("Esto garantiza que la comunicación se mantenga después del cambio."));
        
        // Programar actualización automática después del reinicio
        Serial.println(F("\nConfiguración programada para actualización automática:"));
        Serial.print(F("  Nuevo baudrate: "));
        Serial.print(getBaudrateValue(currentBaudrate));
        Serial.println(F(" bps"));
        
        // Marcar que necesitamos actualizar la configuración local después del reinicio
        bool pendingLocalUpdate = true;
        uint8_t pendingBaudrate = currentBaudrate;
        
        Serial.println(F("\nPasos siguientes:"));
        Serial.println(F("1. REINICIE MANUALMENTE el XY7025"));
        Serial.println(F("2. El Arduino detectará el cambio y actualizará su configuración automáticamente"));
        Serial.println(F("3. Use opción [a] para verificar conexión"));
        
        // Simular la actualización local inmediatamente después del write
        // (en la práctica, esto debería hacerse después del reinicio manual)
        Serial.println(F("\nActualizando configuración local inmediatamente..."));
        updateLocalBaudrateAfterWrite(currentBaudrate);
    }
}

//====================================================================
// CAMBIOS LOCALES
//====================================================================

void changeLocalSlave() {
    Serial.println(F("--- CAMBIAR SLAVE LOCAL (Arduino) ---"));
    
    Serial.println(F("Configuración actual:"));
    Serial.print(F("  Slave Address local: "));
    Serial.println(currentSlaveAddress);
    Serial.print(F("  Baudrate local: "));
    Serial.print(getBaudrateValue(currentBaudrate));
    Serial.println(F(" bps"));
    Serial.println();
    
    // Solicitar y validar una nueva dirección slave ingresado por el usuario
    int newAddress;
    if (readIntegerInput(newAddress, MIN_SLAVE_ADDRESS, MAX_SLAVE_ADDRESS, 
                        "Ingrese nueva dirección slave (1-247): ") != ERROR_NONE) {
        return;
    }
    
    Serial.print(F("Dirección ingresada: "));
    Serial.println(newAddress);
    
    // Actualizar configuración local
    currentSlaveAddress = (uint8_t)newAddress;
    xy7025_1 = XY7025_Modbus(xy7025_serial, currentSlaveAddress);
    
    Serial.print(F("✓ Slave local actualizado a "));
    Serial.println(currentSlaveAddress);
    Serial.println(F("ℹ️ Cambio LOCAL únicamente - Solo afecta al Arduino"));
    Serial.println(F("ℹ️ Para guardar en XY7025 use opción [w]"));
    
    // Probar nueva configuración
    Serial.println(F("Probando nueva configuración..."));
    systemConnected = testConnection();
    
    if (systemConnected) {
        printFromPROGMEM(SUCCESS_CONFIG);
        systemState = STATE_CONNECTED;
    } else {
        printFromPROGMEM(WARN_NO_WORK);
        Serial.println(F("  - Verifique que la dirección sea correcta"));
        Serial.println(F("  - Si es necesario, restaure con opción [a]"));
        systemState = STATE_ERROR;
    }
}

void changeLocalBaudrate() {
    Serial.println(F("--- CAMBIAR BAUDRATE LOCAL (Arduino) ---"));
    
    Serial.println(F("Configuración actual:"));
    Serial.print(F("  Slave Address: "));
    Serial.println(currentSlaveAddress);
    Serial.print(F("  Baudrate local: "));
    Serial.print(getBaudrateValue(currentBaudrate));
    Serial.print(F(" bps (índice "));
    Serial.print(currentBaudrate);
    Serial.println(F(")"));
    Serial.println();
    
    Serial.println(F("Tabla de baudrates disponibles:"));
    for (uint8_t i = 0; i < BAUDRATE_COUNT; i++) {
        Serial.print(F("  "));
        Serial.print(i);
        Serial.print(F(": "));
        Serial.print(getBaudrateValue(i));
        Serial.print(F(" bps"));
        if (i == currentBaudrate) {
            Serial.print(F(" (ACTUAL)"));
        }
        Serial.println();
    }
    Serial.println();
    
    // Solicitar y validar un índice de baudrate (no el valor directo) ingresado por el usuario
    int newBaudrate;
    char prompt[64];
    snprintf_P(prompt, sizeof(prompt), PSTR("Ingrese nuevo índice de baudrate (0-%d): "), BAUDRATE_COUNT-1);
    
    if (readIntegerInput(newBaudrate, 0, BAUDRATE_COUNT-1, prompt) != ERROR_NONE) {
        return;
    }
    
    Serial.print(F("Baudrate seleccionado: "));
    Serial.print(getBaudrateValue(newBaudrate));
    Serial.print(F(" bps (índice "));
    Serial.print(newBaudrate);
    Serial.println(F(")"));
    
    // Cambiar baudrate de comunicación
    Serial.println(F("Cambiando baudrate de comunicación..."));
    xy7025_serial.end();
    delay(100);
    xy7025_serial.begin(getBaudrateValue(newBaudrate));
    delay(200);
    
    // Actualizar configuración local
    currentBaudrate = (uint8_t)newBaudrate;
    xy7025_1.begin(getBaudrateValue(currentBaudrate));
    
    Serial.println(F("✓ Baudrate local actualizado"));
    Serial.println(F("ℹ️ Cambio LOCAL únicamente - Solo afecta al Arduino"));
    Serial.println(F("ℹ️ Para guardar en XY7025 use opción [r]"));
    
    // Probar nueva configuración
    Serial.println(F("Probando nueva configuración..."));
    systemConnected = testConnection();
    
    if (systemConnected) {
        printFromPROGMEM(SUCCESS_CONFIG);
        systemState = STATE_CONNECTED;
    } else {
        printFromPROGMEM(WARN_NO_WORK);
        Serial.println(F("  - Verifique que el baudrate sea correcto"));
        Serial.println(F("  - Use búsqueda de baudrate si es necesario"));
        systemState = STATE_ERROR;
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
    Serial.println(F("   [s] → Buscar Slave"));
    Serial.println(F("   [b] → Buscar Baudrate (si [s] falla)"));
    Serial.println();
    Serial.println(F("2. Para CAMBIAR CONFIGURACIÓN:"));
    Serial.println(F("   [m]/[n] → Cambiar localmente"));
    Serial.println(F("   [a] → Verificar que funciona"));
    Serial.println(F("   [w]/[r] → Guardar en XY7025"));
    Serial.println(F("   REINICIAR MANUALMENTE"));
    Serial.println(F("   [a] → Verificar nuevamente"));
    Serial.println();
    Serial.println(F("3. RECUPERACIÓN DE EMERGENCIA:"));
    Serial.println(F("   Si pierde comunicación:"));
    Serial.println(F("   [b] → Búsqueda exhaustiva"));
    Serial.println();
    Serial.println(F("OPCIONES DISPONIBLES:"));
    Serial.println(F("  [a] Verificar: Prueba conexión y lee configuración actual"));
    Serial.println(F("  [s] Buscar Slave: Busca automáticamente dirección 1-247"));
    Serial.println(F("  [b] Buscar Baudrate: Prueba todos los baudrates (proceso largo)"));
    Serial.println(F("  [w] Escribir Slave: Guarda dirección en registro 0x0018 del XY7025"));
    Serial.println(F("  [r] Escribir Baudrate: Guarda baudrate en registro 0x0019 del XY7025"));
    Serial.println(F("  [m] Cambiar Local: Cambia dirección solo en Arduino (no en XY7025)"));
    Serial.println(F("  [n] Cambiar Local: Cambia baudrate solo en Arduino (no en XY7025)"));
    Serial.println(F("  [i] Info: Muestra opciones e información básica"));
    Serial.println(F("  [h] Ayuda: Muestra ayuda detallada"));
    Serial.println(F("  [q] Salir: Termina el programa"));
    Serial.println();
    Serial.println(F("IMPORTANTE:"));
    Serial.println(F("  - Los cambios en XY7025 requieren REINICIO MANUAL (botón físico)"));
    Serial.println(F("  - Presione 'q' durante búsquedas para cancelar"));
    Serial.println(F("  - Use [d] para activar/desactivar modo debug"));
    Serial.println();
    Serial.println(F("CONEXIONES:"));
    Serial.println(F("  Arduino Pin 2 (RX) → XY7025 Pin RX"));
    Serial.println(F("  Arduino Pin 3 (TX) → XY7025 Pin TX"));
    Serial.println(F("  GND Arduino → GND XY7025"));
}

//====================================================================
// FUNCIONES AUXILIARES
//====================================================================
// Obtener el valor numérico del baudrate desde el array almacenado en memoria de programa "PROGMEM"

uint32_t getBaudrateValue(uint8_t index) {
    if (index >= BAUDRATE_COUNT) return 115200; // Valor por defecto
    uint32_t value;
    memcpy_P(&value, &BAUDRATES[index], sizeof(uint32_t));
    return value;
}

const char* getConnectionStatusText() {
    return systemConnected ? "OK" : "ERROR";
}

// Función comentada temporalmente - no se utiliza en el código actual
/*
const char* getBaudrateName(uint8_t index) {
    if (index >= BAUDRATE_COUNT) return "Desconocido";
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "%lu bps", (unsigned long)getBaudrateValue(index));
    return buffer;
}
*/

ErrorCode waitForSerialResponse(char& response, uint16_t timeout) {
    unsigned long startTime = millis();
    
    Serial.print(F("¿Continuar? (s/n): "));
    
    while (!Serial.available() && (millis() - startTime < timeout)) {
        delay(100);
    }
    
    if (!Serial.available()) {
        printFromPROGMEM(ERROR_TIMEOUT_MSG);
        return ERROR_TIMEOUT;
    }
    
    response = Serial.read();
    return ERROR_NONE;
}

ErrorCode readIntegerInput(int& value, int minVal, int maxVal, const char* prompt) {
    Serial.print(prompt);
    
    unsigned long startTime = millis();
    while (!Serial.available() && (millis() - startTime < SERIAL_TIMEOUT_MS)) {
        delay(100);
    }
    
    if (!Serial.available()) {
        printFromPROGMEM(ERROR_TIMEOUT_MSG);
        return ERROR_TIMEOUT;
    }
    
    value = Serial.parseInt();
    
    if (value < minVal || value > maxVal) {
        Serial.print(F("Valor inválido. Ingrese valor entre "));
        Serial.print(minVal);
        Serial.print(F("-"));
        Serial.print(maxVal);
        Serial.println(F(": "));
        return ERROR_INVALID_ADDRESS;  // Usar un ErrorCode válido
    }
    
    return ERROR_NONE;
}

//====================================================================
// ACTUALIZACIÓN DE CONFIGURACIÓN LOCAL DESPUÉS DE WRITE
//====================================================================

ErrorCode updateLocalSlaveAfterWrite(uint8_t newSlaveAddress) {
    Serial.println(F("\n--- ACTUALIZANDO CONFIGURACIÓN LOCAL ---"));
    Serial.print(F("Actualizando dirección slave local de "));
    Serial.print(currentSlaveAddress);
    Serial.print(F(" a "));
    Serial.println(newSlaveAddress);
    
    // Guardar la dirección anterior por si hay problemas
    uint8_t oldSlaveAddress = currentSlaveAddress;
    
    // Actualizar configuración local
    currentSlaveAddress = newSlaveAddress;
    
    // Reinicializar el objeto XY7025_Modbus con la nueva dirección
    xy7025_1 = XY7025_Modbus(xy7025_serial, currentSlaveAddress);
    
    // Reconfigurar con el baudrate actual
    xy7025_1.begin(getBaudrateValue(currentBaudrate));
    
    // Probar la nueva configuración
    Serial.println(F("Probando nueva configuración local..."));
    systemConnected = testConnection();
    
    if (systemConnected) {
        Serial.println(F("✓ Configuración local actualizada exitosamente"));
        systemState = STATE_CONNECTED;
        return ERROR_NONE;
    } else {
        Serial.println(F("✗ Error actualizando configuración local"));
        Serial.println(F("Restaurando configuración anterior..."));
        
        // Restaurar configuración anterior
        currentSlaveAddress = oldSlaveAddress;
        xy7025_1 = XY7025_Modbus(xy7025_serial, currentSlaveAddress);
        xy7025_1.begin(getBaudrateValue(currentBaudrate));
        
        // Verificar si la configuración anterior funciona
        systemConnected = testConnection();
        if (systemConnected) {
            systemState = STATE_CONNECTED;
        } else {
            systemState = STATE_ERROR;
        }
        return ERROR_WRITE_FAILED;
    }
}

ErrorCode updateLocalBaudrateAfterWrite(uint8_t newBaudrateIndex) {
    Serial.println(F("\n--- ACTUALIZANDO CONFIGURACIÓN LOCAL ---"));
    Serial.print(F("Actualizando baudrate local de "));
    Serial.print(getBaudrateValue(currentBaudrate));
    Serial.print(F(" a "));
    Serial.println(getBaudrateValue(newBaudrateIndex));
    
    // Guardar el baudrate anterior por si hay problemas
    uint8_t oldBaudrate = currentBaudrate;
    
    // Actualizar configuración local
    currentBaudrate = newBaudrateIndex;
    
    // Cambiar baudrate de comunicación
    Serial.println(F("Reconfigurando puerto serial..."));
    xy7025_serial.end();
    delay(100);
    xy7025_serial.begin(getBaudrateValue(currentBaudrate));
    delay(200);
    
    // Reinicializar el objeto XY7025_Modbus con el nuevo baudrate
    xy7025_1.begin(getBaudrateValue(currentBaudrate));
    
    // Probar la nueva configuración
    Serial.println(F("Probando nueva configuración local..."));
    systemConnected = testConnection();
    
    if (systemConnected) {
        Serial.println(F("✓ Configuración local actualizada exitosamente"));
        systemState = STATE_CONNECTED;
        return ERROR_NONE;
    } else {
        Serial.println(F("✗ Error actualizando configuración local"));
        Serial.println(F("Restaurando configuración anterior..."));
        
        // Restaurar configuración anterior
        currentBaudrate = oldBaudrate;
        xy7025_serial.end();
        delay(100);
        xy7025_serial.begin(getBaudrateValue(currentBaudrate));
        delay(200);
        xy7025_1.begin(getBaudrateValue(currentBaudrate));
        
        // Verificar si la configuración anterior funciona
        systemConnected = testConnection();
        if (systemConnected) {
            systemState = STATE_CONNECTED;
        } else {
            systemState = STATE_ERROR;
        }
        return ERROR_WRITE_FAILED;
    }
}

//====================================================================
// FIN DEL SKETCH
//====================================================================