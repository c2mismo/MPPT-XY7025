/*
 * ReadBaudrateMemory - Herramienta de Diagnóstico para XY7025
 * 
 * Este ejemplo lee los baudrates configurados en la memoria del dispositivo XY7025
 * usando la librería XY7025_Modbus para verificar los valores reales de las
 * posiciones 7 y 8, ya que la documentación del fabricante es inconsistente.
 * 
 * CONEXIONES VERIFICADAS:
 * - XY7025 D+ (TX)  -> Arduino Pin 2 (RX SoftwareSerial)
 * - XY7025 D- (RX)  -> Arduino Pin 3 (TX SoftwareSerial)
 * - XY7025 GND      -> Arduino GND
 * 
 * BAUDRATES VERIFICADOS (según XY7025_Address_Finder):
 * Posición 0: 9600 baud
 * Posición 1: 14400 baud  
 * Posición 2: 19200 baud
 * Posición 3: 38400 baud
 * Posición 4: 56000 baud
 * Posición 5: 57600 baud
 * Posición 6: 115200 baud
 * Posición 7: 2400 baud  (PROBLEMÁTICA según documentación)
 * Posición 8: 4800 baud  (PROBLEMÁTICA según documentación)
 * 
 * Compilar con: PlatformIO para Arduino UNO
 * Monitor Serial: 115200 baud
 * 
 * Autor: Desarrollo para librería MPPT-XY7025
 * Fecha: 2025-11-22
 * Basado en: XY7025_Address_Finder (baudrates verificados)
 */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <XY7025_Modbus.h>

// ================================================================================================
// CONFIGURACIÓN DE COMUNICACIÓN
// ================================================================================================

// Pines para SoftwareSerial (según especificaciones del XY7025)
#define XY7025_RX_PIN  2    // Pin de recepción (D+ del XY7025)
#define XY7025_TX_PIN  3    // Pin de transmisión (D- del XY7025)

// Parámetros de comunicación
#define XY7025_BAUDRATE     115200  // Velocidad de comunicación con el dispositivo
#define SERIAL_BAUDRATE     115200  // Velocidad del monitor serial
#define SLAVE_ADDRESS       1       // Dirección del esclavo por defecto

// ================================================================================================
// BAUDRATES VERIFICADOS (según XY7025_Address_Finder)
// ================================================================================================

// Tabla de baudrates verificados experimentally
const uint32_t BAUDRATES_VERIFIED[] PROGMEM = {
    9600,    // Posición 0
    14400,   // Posición 1
    19200,   // Posición 2
    38400,   // Posición 3
    56000,   // Posición 4
    57600,   // Posición 5
    115200,  // Posición 6
    2400,    // Posición 7 (PROBLEMÁTICA según documentación)
    4800     // Posición 8 (PROBLEMÁTICA según documentación)
};

const uint8_t BAUDRATE_COUNT = sizeof(BAUDRATES_VERIFIED) / sizeof(BAUDRATES_VERIFIED[0]);

// ================================================================================================
// INSTANCIAS GLOBALES
// ================================================================================================

// Instancia de SoftwareSerial para comunicación con XY7025
SoftwareSerial xy7025Serial(XY7025_RX_PIN, XY7025_TX_PIN);

// Instancia de XY7025_Modbus para protocolo Modbus RTU
XY7025_Modbus xy7025(xy7025Serial, SLAVE_ADDRESS);

// ================================================================================================
// FUNCIONES DE UTILIDAD
// ================================================================================================

/**
 * Obtiene el valor de baudrate desde PROGMEM
 */
uint32_t getBaudrateFromPROGMEM(uint8_t index) {
    if (index >= BAUDRATE_COUNT) return 115200UL; // Valor por defecto
    uint32_t value;
    memcpy_P(&value, &BAUDRATES_VERIFIED[index], sizeof(uint32_t));
    return value;
}

/**
 * Prueba la conectividad básica leyendo el registro de voltaje de salida
 */
bool testBasicConnectivity() {
    uint16_t vout = xy7025.readRegister(XY7025_VOUT);
    return (vout != XY7025_ERROR_UINT16);
}

/**
 * Lee el baudrate configurado en el dispositivo XY7025
 * @return Índice del baudrate configurado (0-8) o 255 si error
 */
uint8_t readCurrentBaudrateIndex() {
    uint16_t baudrateValue = xy7025.readBaudrate();
    
    if (baudrateValue == XY7025_ERROR_UINT16) {
        return 255; // Error de lectura
    }
    
    // El registro devuelve el índice del baudrate (0-8)
    if (baudrateValue < BAUDRATE_COUNT) {
        return (uint8_t)baudrateValue;
    }
    
    return 255; // Índice fuera de rango
}

/**
 * Prueba todos los baudrates disponibles para verificar cuáles funcionan
 * @param workingBaudrates Array para almacenar los baudrates que funcionan
 * @param maxCount Tamaño máximo del array
 * @return Número de baudrates que funcionan
 */
uint8_t testAllBaudrates(uint8_t *workingBaudrates, uint8_t maxCount) {
    uint8_t workingCount = 0;
    
    Serial.println(F("\n=== PROBANDO TODOS LOS BAUDRATES DISPONIBLES ==="));
    Serial.println(F("Esto verificará cuáles baudrates son reconocidos por el dispositivo"));
    Serial.println();
    
    for (uint8_t i = 0; i < BAUDRATE_COUNT && workingCount < maxCount; i++) {
        uint32_t baudValue = getBaudrateFromPROGMEM(i);
        
        Serial.print(F("Probando posición "));
        Serial.print(i);
        Serial.print(F(" ("));
        Serial.print(baudValue);
        Serial.print(F(" baud): "));
        
        // Cambiar temporalmente a este baudrate
        xy7025Serial.end();
        delay(100);
        xy7025Serial.begin(baudValue);
        delay(200);
        
        // Probar conectividad
        if (testBasicConnectivity()) {
            Serial.println(F("✓ FUNCIONA"));
            workingBaudrates[workingCount++] = i;
        } else {
            Serial.println(F("✗ NO RESPONDE"));
        }
        
        delay(100); // Pausa entre pruebas
    }
    
    // Restaurar baudrate original
    xy7025Serial.end();
    delay(100);
    xy7025Serial.begin(XY7025_BAUDRATE);
    delay(200);
    
    return workingCount;
}

/**
 * Lee y muestra información detallada sobre la configuración actual
 */
void readDetailedConfiguration() {
    Serial.println(F("\n=== CONFIGURACIÓN ACTUAL DEL XY7025 ==="));
    
    // Modelo del dispositivo
    uint16_t model = xy7025.readModel();
    Serial.print(F("Modelo: "));
    if (model != XY7025_ERROR_UINT16) {
        Serial.println(model);
    } else {
        Serial.println(F("ERROR"));
    }
    
    // Versión del firmware
    uint16_t version = xy7025.readVersion();
    Serial.print(F("Versión firmware: "));
    if (version != XY7025_ERROR_UINT16) {
        Serial.println(version);
    } else {
        Serial.println(F("ERROR"));
    }
    
    // Dirección del esclavo
    uint16_t slaveAddress = xy7025.readRegister(XY7025_SLAVE_ADR);
    Serial.print(F("Dirección esclavo: "));
    if (slaveAddress != XY7025_ERROR_UINT16) {
        Serial.println(slaveAddress);
    } else {
        Serial.println(F("ERROR"));
    }
    
    // Baudrate actual configurado
    uint8_t currentBaudrateIndex = readCurrentBaudrateIndex();
    Serial.print(F("Baudrate configurado: "));
    if (currentBaudrateIndex < BAUDRATE_COUNT) {
        Serial.print(F("Posición "));
        Serial.print(currentBaudrateIndex);
        Serial.print(F(" ("));
        Serial.print(getBaudrateFromPROGMEM(currentBaudrateIndex));
        Serial.print(F(" baud)"));
        
        // Indicar si es una posición problemática
        if (currentBaudrateIndex == 7) {
            Serial.print(F(" [PROBLEMÁTICA según documentación]"));
        } else if (currentBaudrateIndex == 8) {
            Serial.print(F(" [PROBLEMÁTICA según documentación]"));
        }
        Serial.println();
    } else {
        Serial.println(F("ERROR o fuera de rango"));
    }
    
    // Voltaje de salida actual (para verificar conectividad)
    float vout = xy7025.readVoltageOutput();
    Serial.print(F("Voltaje salida actual: "));
    if (!isnan(vout)) {
        Serial.print(vout, 2);
        Serial.println(F(" V"));
    } else {
        Serial.println(F("ERROR"));
    }
}

/**
 * Muestra resumen de baudrates disponibles y su estado
 */
void showBaudrateSummary(uint8_t *workingBaudrates, uint8_t workingCount) {
    Serial.println(F("\n=== RESUMEN DE BAUDRATES DISPONIBLES ==="));
    Serial.println(F("Pos | Baudrate | Estado     | Documentado | Observación"));
    Serial.println(F("----|----------|------------|-------------|-------------"));
    
    for (uint8_t i = 0; i < BAUDRATE_COUNT; i++) {
        uint32_t baudValue = getBaudrateFromPROGMEM(i);
        
        // Verificar si este baudrate funciona
        bool isWorking = false;
        for (uint8_t j = 0; j < workingCount; j++) {
            if (workingBaudrates[j] == i) {
                isWorking = true;
                break;
            }
        }
        
        Serial.print(F("  "));
        Serial.print(i);
        Serial.print(F("  | "));
        Serial.print(baudValue);
        Serial.print(F(" | "));
        
        if (isWorking) {
            Serial.print(F("FUNCIONA "));
        } else {
            Serial.print(F("NO FUNCIONA"));
        }
        Serial.print(F(" | "));
        
        // Estado según documentación
        if (i < 6) {
            Serial.print(F("Correcto   "));
        } else if (i == 7) {
            Serial.print(F("Problemático"));
        } else if (i == 8) {
            Serial.print(F("Problemático"));
        } else {
            Serial.print(F("Desconocido "));
        }
        Serial.print(F(" | "));
        
        // Observaciones específicas
        if (i == 7) {
            Serial.print(F("Posición 7: Debería ser 2400"));
        } else if (i == 8) {
            Serial.print(F("Posición 8: Debería ser 4800"));
        } else if (i < 6) {
            Serial.print(F("Verificado como correcto"));
        } else {
            Serial.print(F("Sin observaciones"));
        }
        
        Serial.println();
    }
}

/**
 * Muestra análisis específico de las posiciones problemáticas
 */
void analyzeProblematicPositions(uint8_t *workingBaudrates, uint8_t workingCount) {
    Serial.println(F("\n=== ANÁLISIS DE POSICIONES 7 y 8 (PROBLEMÁTICAS) ==="));
    Serial.println(F("Según la documentación del fabricante, estas posiciones deberían ser:"));
    Serial.println(F("- Posición 7: 2400 baud"));
    Serial.println(F("- Posición 8: 4800 baud"));
    Serial.println();
    Serial.println(F("Resultados de la prueba:"));
    
    // Analizar posición 7
    Serial.print(F("Posición 7: "));
    bool pos7Works = false;
    for (uint8_t i = 0; i < workingCount; i++) {
        if (workingBaudrates[i] == 7) {
            pos7Works = true;
            break;
        }
    }
    
    if (pos7Works) {
        Serial.println(F("✓ FUNCIONA - Confirmado que 2400 baud es correcto"));
    } else {
        Serial.println(F("✗ NO FUNCIONA - El valor 2400 baud no es reconocido"));
    }
    
    // Analizar posición 8
    Serial.print(F("Posición 8: "));
    bool pos8Works = false;
    for (uint8_t i = 0; i < workingCount; i++) {
        if (workingBaudrates[i] == 8) {
            pos8Works = true;
            break;
        }
    }
    
    if (pos8Works) {
        Serial.println(F("✓ FUNCIONA - Confirmado que 4800 baud es correcto"));
    } else {
        Serial.println(F("✗ NO FUNCIONA - El valor 4800 baud no es reconocido"));
    }
    
    // Conclusiones
    Serial.println(F("\n=== CONCLUSIONES ==="));
    if (pos7Works && pos8Works) {
        Serial.println(F("✓ Los baudrates documentados (2400 y 4800) SON CORRECTOS"));
        Serial.println(F("✓ El problema debe estar en otro lugar (conexiones, configuración, etc.)"));
    } else if (!pos7Works && !pos8Works) {
        Serial.println(F("✗ Los baudrates documentados (2400 y 4800) NO FUNCIONAN"));
        Serial.println(F("✗ Se necesita investigación adicional de los valores reales"));
    } else {
        Serial.println(F("⚠️ Resultados mixtos - una posición funciona, la otra no"));
        Serial.println(F("⚠️ Se requiere análisis más detallado"));
    }
}

/**
 * Muestra información de ayuda sobre el uso del ejemplo
 */
void showHelp() {
    Serial.println(F("\n=== HERRAMIENTA DE DIAGNÓSTICO XY7025 - BAUDRATES ==="));
    Serial.println(F(""));
    Serial.println(F("PROPÓSITO:"));
    Serial.println(F("  - Verificar los baudrates almacenados en la memoria del XY7025"));
    Serial.println(F("  - Confirmar si las posiciones 7 y 8 son realmente 2400 y 4800 baud"));
    Serial.println(F("  - Identificar baudrates reales vs documentación del fabricante"));
    Serial.println(F(""));
    Serial.println(F("BAUDRATES VERIFICADOS (según XY7025_Address_Finder):"));
    Serial.println(F("  Posiciones 0-6: Valores estándar y funcionales"));
    Serial.println(F("  Posición 7: 2400 baud (PROBLEMÁTICA según documentación)"));
    Serial.println(F("  Posición 8: 4800 baud (PROBLEMÁTICA según documentación)"));
    Serial.println(F(""));
    Serial.println(F("CONEXIONES:"));
    Serial.println(F("  XY7025 D+ (TX) -> Arduino Pin 2 (RX SoftwareSerial)"));
    Serial.println(F("  XY7025 D- (RX) -> Arduino Pin 3 (TX SoftwareSerial)"));
    Serial.println(F("  XY7025 GND     -> Arduino GND"));
    Serial.println(F(""));
    Serial.println(F("FUNCIONES:"));
    Serial.println(F("  - Prueba conectividad básica"));
    Serial.println(F("  - Lee configuración actual del dispositivo"));
    Serial.println(F("  - Prueba todos los baudrates disponibles"));
    Serial.println(F("  - Analiza específicamente posiciones 7 y 8"));
    Serial.println(F("  - Proporciona conclusiones basadas en resultados"));
    Serial.println(F(""));
}

// ================================================================================================
// SETUP Y LOOP PRINCIPALES
// ================================================================================================

void setup() {
    // Inicializar comunicación serial para debug
    Serial.begin(SERIAL_BAUDRATE);
    while (!Serial) {
        ; // Esperar a que la conexión serial esté lista
    }
    
    // Mostrar información de ayuda
    showHelp();
    
    // Inicializar comunicación con XY7025
    Serial.print(F("Iniciando comunicación con XY7025 a "));
    Serial.print(XY7025_BAUDRATE);
    Serial.println(F(" baud..."));
    
    xy7025Serial.begin(XY7025_BAUDRATE);
    delay(200);
    
    // Inicializar XY7025_Modbus
    if (xy7025.begin(XY7025_BAUDRATE)) {
        Serial.println(F("✓ XY7025_Modbus inicializado correctamente"));
    } else {
        Serial.println(F("✗ Error inicializando XY7025_Modbus"));
        Serial.println(F("Verifique conexiones y configuración"));
    }
    
    delay(200);
    
    Serial.println(F("Presione cualquier tecla y Enter para comenzar el diagnóstico..."));
    
    // Esperar a que el usuario esté listo
    while (Serial.available() == 0) {
        delay(100);
    }
    
    // Limpiar buffer serial
    while (Serial.available()) {
        Serial.read();
    }
    
    Serial.println(F("\nIniciando diagnóstico...\n"));
}

void loop() {
    // Array para almacenar baudrates que funcionan
    uint8_t workingBaudrates[BAUDRATE_COUNT];
    uint8_t workingCount = 0;
    
    // Prueba de conectividad básica
    Serial.println(F("=== PRUEBA DE CONECTIVIDAD BÁSICA ==="));
    if (testBasicConnectivity()) {
        Serial.println(F("✓ Conexión exitosa con XY7025"));
    } else {
        Serial.println(F("✗ No se puede establecer conexión con XY7025"));
        Serial.println(F("Verifique:"));
        Serial.println(F("  - Conexiones físicas"));
        Serial.println(F("  - Baudrate correcto (115200)"));
        Serial.println(F("  - Dirección del esclavo (1)"));
        Serial.println(F("  - XY7025 encendido y funcionando"));
        
        Serial.println(F("\n¿Desea continuar de todos modos? (s/n): "));
        while (Serial.available() == 0) {
            delay(100);
        }
        
        char response = Serial.read();
        while (Serial.available()) {
            Serial.read(); // Limpiar buffer
        }
        
        if (response != 's' && response != 'S') {
            Serial.println(F("Diagnóstico cancelado."));
            while (true) {
                delay(1000); // Mantener el programa corriendo
            }
        }
    }
    
    // Lectura detallada de configuración
    readDetailedConfiguration();
    
    // Probar todos los baudrates
    workingCount = testAllBaudrates(workingBaudrates, BAUDRATE_COUNT);
    
    // Mostrar resumen
    showBaudrateSummary(workingBaudrates, workingCount);
    
    // Analizar posiciones problemáticas
    analyzeProblematicPositions(workingBaudrates, workingCount);
    
    // Preguntar si desea repetir el diagnóstico
    Serial.println(F("\n¿Desea repetir el diagnóstico? (s/n): "));
    
    while (Serial.available() == 0) {
        delay(100);
    }
    
    char response = Serial.read();
    while (Serial.available()) {
        Serial.read(); // Limpiar buffer
    }
    
    if (response != 's' && response != 'S') {
        Serial.println(F("\nDiagnóstico completado."));
        while (true) {
            delay(1000); // Mantener el programa corriendo
        }
    }
    
    Serial.println(F("\n================================================="));
    delay(1000); // Pausa antes de repetir
}