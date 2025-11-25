/**
 * Ejemplo completo de lectura y escritura con XY7025_Modbus
 * 
 * Este ejemplo demuestra todas las funciones de la librería XY7025_Modbus:
 * - Lectura de registros individuales
 * - Lectura de registros de perfiles M0-M9
 * - Escritura de registros individuales
 * - Escritura de registros de perfiles
 * - Lectura de múltiples registros consecutivos
 * - Lectura con visualización hexadecimal
 * - Escritura con valores en formato hexadecimal
 * - Menú interactivo por consola serial
 * 
 * Autor: XY7025 Team
 * Versión: 1.0
 * Fecha: 2025-11-25
 */

#include <XY7025_Modbus.h>
#include <SoftwareSerial.h>

// Configuración de pines para SoftwareSerial
const uint8_t RX_PIN = 2;
const uint8_t TX_PIN = 3;
// #define RE_DE_PIN 12;  // Pin de control RE/DE para RS485


// Pines de comunicación
const uint8_t MODBUS_RX = 2;   // RX Arduino -> RX XY7025
const uint8_t MODBUS_TX = 3;   // TX Arduino -> TX XY7025
// const uint8_t RE_DE_PIN 12;  // Pin de control RE/DE para RS485

// Configuración de comunicación
const long MODBUS_BAUDRATE = 115200;
const long SERIAL_BAUDRATE = 115200;
const uint8_t SLAVE_ADDRESS = 1;

// Crear objeto SoftwareSerial para comunicación Modbus
SoftwareSerial modbusSerial(MODBUS_RX, MODBUS_TX);

// Crear objeto XY7025_Modbus
XY7025_Modbus xy7025(modbusSerial, SLAVE_ADDRESS);

// Variables globales
String inputString = "";
bool stringComplete = false;

// Prototipos de funciones
void printMenu();
void processCommand(char command);
uint16_t readSingleRegister(uint16_t address);
uint16_t readProfileRegister(uint8_t profile, ProfileOffset offset);
bool writeSingleRegister(uint16_t address, uint16_t value);
bool writeProfileRegister(uint8_t profile, ProfileOffset offset, uint16_t value);
bool readMultipleRegisters(uint16_t startAddress, uint8_t count);
void readRegisterWithHex(uint16_t address);
bool writeRegisterWithHex(uint16_t address, String hexValue);
uint16_t hexStringToUint16(String hexString);
void printRegisterValue(uint16_t address, uint16_t value);
void printProfileValue(uint8_t profile, ProfileOffset offset, uint16_t value);

void setup() {
  // Inicializar Serial para consola de depuración
  Serial.begin(SERIAL_BAUDRATE);
  while (!Serial) {
    ; // Esperar a que el puerto serial esté listo
  }
  
  // Inicializar SoftwareSerial para Modbus
  modbusSerial.begin(MODBUS_BAUDRATE);
  
  // Inicializar XY7025_Modbus
  if (!xy7025.begin(MODBUS_BAUDRATE)) {
    Serial.println(F("Error inicializando XY7025_Modbus"));
    while (1) {
      ; // Bucle infinito en caso de error
    }
  }
  
  // Configurar modo debug
  xy7025.enableDebug(true);
  
  // Configurar timeout y reintentos
  xy7025.setTimeout(2000);  // 2 segundos
  xy7025.setRetries(3);     // 3 reintentos
  
  // Configurar pin de control RE/DE
  // pinMode(RE_DE_PIN, OUTPUT);
  //digitalWrite(RE_DE_PIN, LOW); // Modo recepción por defecto
  
  // Mensaje de bienvenida
  Serial.println(F("=== XY7025 Modbus ReadWrite Example ==="));
  Serial.println(F("Versión 1.0 - XY7025 Team"));
  Serial.println(F("Baudrate: 115200, Slave: 1"));
  Serial.println();
  
  // Mostrar menú inicial
  printMenu();
  
  // Limpiar buffer de entrada
  inputString.reserve(20);
  inputString = "";
  stringComplete = false;
}

void loop() {
  // Leer comandos del usuario
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') {
      stringComplete = true;
    } else if (inChar != '\r') {
      inputString += inChar;
    }
  }
  
  if (stringComplete) {
    if (inputString.length() > 0) {
      char command = inputString.charAt(0);
      processCommand(command);
    }
    
    // Limpiar buffer para siguiente comando
    inputString = "";
    stringComplete = false;
  }
}


// Imprimir menú de opciones
void printMenu() {
  Serial.println(F("=== MENÚ PRINCIPAL XY7025 ==="));
  Serial.println(F("1. Leer registro individual"));
  Serial.println(F("2. Leer registro de perfil"));
  Serial.println(F("3. Escribir registro individual"));
  Serial.println(F("4. Escribir registro de perfil"));
  Serial.println(F("5. Leer múltiples registros"));
  Serial.println(F("6. Leer registro con valor hex"));
  Serial.println(F("7. Escribir registro con valor hex"));
  Serial.println(F("8. Leer todos los registros"));
  Serial.println(F("9. Ayuda (help)"));
  Serial.println(F("0. Salir"));
  Serial.println();
  Serial.print(F("Ingrese comando: "));
}

// Procesar comando del usuario
void processCommand(char command) {
  Serial.println();
  
  switch (command) {
    case '1':
      // Leer registro individual
      {
        Serial.print(F("Ingrese dirección del registro (HEX): "));
        while (!Serial.available()) {
          delay(10);
        }
        String hexAddress = Serial.readStringUntil('\n');
        hexAddress.trim();
        
        uint16_t address = (uint16_t)strtol(hexAddress.c_str(), NULL, 16);
        uint16_t value = readSingleRegister(address);
        
        if (value != 0xFFFF) {
          printRegisterValue(address, value);
        } else {
          Serial.print(F("Error leyendo registro 0x"));
          Serial.println(address, HEX);
        }
      }
      break;
      
    case '2':
      // Leer registro de perfil
      {
        Serial.print(F("Ingrese número de perfil (0-9): "));
        while (!Serial.available()) {
          delay(10);
        }
        String profileStr = Serial.readStringUntil('\n');
        profileStr.trim();
        
        uint8_t profile = profileStr.toInt();
        
        Serial.print(F("Ingrese offset del registro (0-13): "));
        while (!Serial.available()) {
          delay(10);
        }
        String offsetStr = Serial.readStringUntil('\n');
        offsetStr.trim();
        
        uint8_t offset = offsetStr.toInt();
        
        if (profile <= 9 && offset <= 13) {
          uint16_t value = readProfileRegister(profile, (ProfileOffset)offset);
          if (value != 0xFFFF) {
            printProfileValue(profile, (ProfileOffset)offset, value);
          } else {
            Serial.print(F("Error leyendo perfil M"));
            Serial.print(profile);
            Serial.print(F(", offset 0x"));
            Serial.println(offset, HEX);
          }
        } else {
          Serial.println(F("Perfil o offset fuera de rango"));
        }
      }
      break;
      
    case '3':
      // Escribir registro individual
      {
        Serial.print(F("Ingrese dirección del registro (HEX): "));
        while (!Serial.available()) {
          delay(10);
        }
        String hexAddress = Serial.readStringUntil('\n');
        hexAddress.trim();
        
        Serial.print(F("Ingrese valor a escribir: "));
        while (!Serial.available()) {
          delay(10);
        }
        String valueStr = Serial.readStringUntil('\n');
        valueStr.trim();
        
        uint16_t address = (uint16_t)strtol(hexAddress.c_str(), NULL, 16);
        uint16_t value = valueStr.toInt();
        
        bool success = writeSingleRegister(address, value);
        
        Serial.print(F("Escribiendo "));
        Serial.print(value);
        Serial.print(F(" en registro 0x"));
        Serial.print(address, HEX);
        Serial.print(F("... "));
        Serial.println(success ? F("ÉXITO") : F("ERROR"));
      }
      break;
      
    case '4':
      // Escribir registro de perfil
      {
        Serial.print(F("Ingrese número de perfil (0-9): "));
        while (!Serial.available()) {
          delay(10);
        }
        String profileStr = Serial.readStringUntil('\n');
        profileStr.trim();
        
        Serial.print(F("Ingrese offset del registro (0-13): "));
        while (!Serial.available()) {
          delay(10);
        }
        String offsetStr = Serial.readStringUntil('\n');
        offsetStr.trim();
        
        Serial.print(F("Ingrese valor a escribir: "));
        while (!Serial.available()) {
          delay(10);
        }
        String valueStr = Serial.readStringUntil('\n');
        valueStr.trim();
        
        uint8_t profile = profileStr.toInt();
        uint8_t offset = offsetStr.toInt();
        uint16_t value = valueStr.toInt();
        
        if (profile <= 9 && offset <= 13) {
          bool success = writeProfileRegister(profile, (ProfileOffset)offset, value);
          
          Serial.print(F("Escribiendo "));
          Serial.print(value);
          Serial.print(F(" en perfil M"));
          Serial.print(profile);
          Serial.print(F(", offset 0x"));
          Serial.print(offset, HEX);
          Serial.print(F("... "));
          Serial.println(success ? F("ÉXITO") : F("ERROR"));
        } else {
          Serial.println(F("Perfil o offset fuera de rango"));
        }
      }
      break;
      
    case '5':
      // Leer múltiples registros
      {
        Serial.print(F("Ingrese dirección inicial (HEX): "));
        while (!Serial.available()) {
          delay(10);
        }
        String hexAddress = Serial.readStringUntil('\n');
        hexAddress.trim();
        
        Serial.print(F("Ingrese cantidad de registros (1-10): "));
        while (!Serial.available()) {
          delay(10);
        }
        String countStr = Serial.readStringUntil('\n');
        countStr.trim();
        
        uint16_t startAddress = (uint16_t)strtol(hexAddress.c_str(), NULL, 16);
        uint8_t count = countStr.toInt();
        
        if (count > 0 && count <= 10) {
          readMultipleRegisters(startAddress, count);
        } else {
          Serial.println(F("Cantidad de registros fuera de rango (1-10)"));
        }
      }
      break;
      
    case '6':
      // Leer registro con valor hex
      {
        Serial.print(F("Ingrese dirección del registro (HEX): "));
        while (!Serial.available()) {
          delay(10);
        }
        String hexAddress = Serial.readStringUntil('\n');
        hexAddress.trim();
        
        uint16_t address = (uint16_t)strtol(hexAddress.c_str(), NULL, 16);
        readRegisterWithHex(address);
      }
      break;
      
    case '7':
      // Escribir registro con valor hex
      {
        Serial.print(F("Ingrese dirección del registro (HEX): "));
        while (!Serial.available()) {
          delay(10);
        }
        String hexAddress = Serial.readStringUntil('\n');
        hexAddress.trim();
        
        Serial.print(F("Ingrese valor hexadecimal (4 dígitos): "));
        while (!Serial.available()) {
          delay(10);
        }
        String hexValue = Serial.readStringUntil('\n');
        hexValue.trim();
        
        uint16_t address = (uint16_t)strtol(hexAddress.c_str(), NULL, 16);
        
        bool success = writeRegisterWithHex(address, hexValue);
        
        Serial.print(F("Escribiendo 0x"));
        Serial.print(hexValue);
        Serial.print(F(" en registro 0x"));
        Serial.print(address, HEX);
        Serial.print(F("... "));
        Serial.println(success ? F("ÉXITO") : F("ERROR"));
      }
      break;
      
    case '8':
      // Leer todos los registros
      {
        Serial.println(F("Leyendo todos los registros principales..."));
        xy7025.readAllRegisters(Serial);
      }
      break;
      
    case '9':
      // Ayuda
      {
        Serial.println(F("=== AYUDA XY7025 ==="));
        Serial.println(F("Este ejemplo demuestra todas las funciones de la librería XY7025_Modbus:"));
        Serial.println(F("- Lectura de registros individuales"));
        Serial.println(F("- Lectura de registros de perfiles M0-M9"));
        Serial.println(F("- Escritura de registros individuales"));
        Serial.println(F("- Escritura de registros de perfiles"));
        Serial.println(F("- Lectura de múltiples registros consecutivos"));
        Serial.println(F("- Lectura con visualización hexadecimal"));
        Serial.println(F("- Escritura con valores en formato hexadecimal"));
        Serial.println();
        Serial.println(F("Configuración:"));
        Serial.print(F("- Baudrate: ")); Serial.println(MODBUS_BAUDRATE);
        Serial.print(F("- Slave Address: ")); Serial.println(SLAVE_ADDRESS);
        Serial.print(F("- RX Pin: ")); Serial.println(RX_PIN);
        Serial.print(F("- TX Pin: ")); Serial.println(TX_PIN);
        Serial.println();
        Serial.println(F("Para más información, consulte el archivo README.md"));
      }
      break;
      
    case '0':
      // Salir
      Serial.println(F("Saliendo del programa..."));
      Serial.println(F("¡Hasta luego!"));
      while (1) {
        delay(1000); // Bucle infinito
      }
      break;
      
    default:
      Serial.println(F("Comando no válido"));
      break;
  }
  
  Serial.println();
  printMenu();
}

// Leer un registro individual
uint16_t readSingleRegister(uint16_t address) {
  return xy7025.readRegister(address);
}

// Leer un registro de perfil
uint16_t readProfileRegister(uint8_t profile, ProfileOffset offset) {
  return xy7025.readProfileReg(profile, offset);
}

// Escribir un registro individual
bool writeSingleRegister(uint16_t address, uint16_t value) {
  return xy7025.writeRegister(address, value);
}

// Escribir un registro de perfil
bool writeProfileRegister(uint8_t profile, ProfileOffset offset, uint16_t value) {
  return xy7025.writeProfileReg(profile, offset, value);
}

// Leer múltiples registros consecutivos
bool readMultipleRegisters(uint16_t startAddress, uint8_t count) {
  Serial.print(F("Registros desde 0x"));
  Serial.print(startAddress, HEX);
  Serial.println(F(":"));
  
  for (uint8_t i = 0; i < count; i++) {
    uint16_t address = startAddress + i;
    uint16_t value = readSingleRegister(address);
    
    if (value != 0xFFFF) {
      Serial.print(F("  [0x"));
      if (address < 0x100) Serial.print(F("0"));
      if (address < 0x10) Serial.print(F("0"));
      Serial.print(address, HEX);
      Serial.print(F("]: "));
      
      // Alinear valores para mejor visualización
      if (value < 10000) Serial.print(F(" "));
      if (value < 1000) Serial.print(F(" "));
      if (value < 100) Serial.print(F(" "));
      if (value < 10) Serial.print(F(" "));
      
      Serial.print(value);
      Serial.print(F(" (0x"));
      if (value < 0x1000) Serial.print(F("0"));
      if (value < 0x100) Serial.print(F("0"));
      if (value < 0x10) Serial.print(F("0"));
      Serial.print(value, HEX);
      Serial.println(F(")"));
    } else {
      Serial.print(F("  [0x"));
      if (address < 0x100) Serial.print(F("0"));
      if (address < 0x10) Serial.print(F("0"));
      Serial.print(address, HEX);
      Serial.println(F("]: ERROR"));
    }
    
    delay(50); // Pequeña pausa entre lecturas
  }
  
  return true;
}

// Leer registro con visualización hexadecimal
void readRegisterWithHex(uint16_t address) {
  Serial.print(F("Leyendo registro 0x"));
  if (address < 0x100) Serial.print(F("0"));
  if (address < 0x10) Serial.print(F("0"));
  Serial.print(address, HEX);
  Serial.println(F("..."));
  
  uint16_t value = readSingleRegister(address);
  
  if (value != 0xFFFF) {
    Serial.print(F("Valor: "));
    Serial.print(value);
    Serial.print(F(" (0x"));
    if (value < 0x1000) Serial.print(F("0"));
    if (value < 0x100) Serial.print(F("0"));
    if (value < 0x10) Serial.print(F("0"));
    Serial.print(value, HEX);
    Serial.println(F(")"));
  } else {
    Serial.println(F("Error leyendo registro"));
  }
}

// Escribir registro con valor hexadecimal
bool writeRegisterWithHex(uint16_t address, String hexValue) {
  uint16_t value = hexStringToUint16(hexValue);
  return writeSingleRegister(address, value);
}

// Convertir string hexadecimal a uint16_t
uint16_t hexStringToUint16(String hexString) {
  hexString.trim();
  if (hexString.startsWith("0x") || hexString.startsWith("0X")) {
    hexString = hexString.substring(2);
  }
  
  return (uint16_t)strtol(hexString.c_str(), NULL, 16);
}

// Imprimir valor de registro con formato
void printRegisterValue(uint16_t address, uint16_t value) {
  Serial.print(F("Registro 0x"));
  if (address < 0x100) Serial.print(F("0"));
  if (address < 0x10) Serial.print(F("0"));
  Serial.print(address, HEX);
  Serial.print(F(": "));
  Serial.print(value);
  Serial.print(F(" (0x"));
  if (value < 0x1000) Serial.print(F("0"));
  if (value < 0x100) Serial.print(F("0"));
  if (value < 0x10) Serial.print(F("0"));
  Serial.print(value, HEX);
  Serial.println(F(")"));
}

// Imprimir valor de perfil con formato
void printProfileValue(uint8_t profile, ProfileOffset offset, uint16_t value) {
  Serial.print(F("Perfil M"));
  Serial.print(profile);
  Serial.print(F(", offset 0x"));
  if (offset < 0x10) Serial.print(F("0"));
  Serial.print(offset, HEX);
  Serial.print(F(": "));
  Serial.print(value);
  Serial.print(F(" (0x"));
  if (value < 0x1000) Serial.print(F("0"));
  if (value < 0x100) Serial.print(F("0"));
  if (value < 0x10) Serial.print(F("0"));
  Serial.print(value, HEX);
  Serial.println(F(")"));
}