/*
 * Ejemplo de uso de la nueva estructura de perfiles M0-M9
 * 
 * Este ejemplo demuestra cómo usar los métodos genéricos readProfileReg() 
 * y writeProfileReg() para acceder a los perfiles de memoria del XY7025.
 */

#include <SoftwareSerial.h>
#include <XY7025_Modbus.h>

// Configuración de pines para comunicación serial software
#define RX_PIN 10
#define TX_PIN 11

// Crear objeto serial software
SoftwareSerial swSerial(RX_PIN, TX_PIN);

// Crear objeto XY7025 con dirección esclava 1 (por defecto)
XY7025_Modbus mppt(swSerial);

void setup() {
  // Inicializar serial para debug
  Serial.begin(115200);
  while (!Serial) {
    ; // Esperar a que el serial esté listo
  }
  
  Serial.println(F("=== Ejemplo de Gestión de Perfiles XY7025 ==="));
  
  // Inicializar comunicación Modbus
  if (!mppt.begin(115200)) {
    Serial.println(F("Error: No se pudo inicializar Modbus"));
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println(F("Modbus inicializado correctamente"));
  
  // Habilitar modo debug para ver detalles
  mppt.enableDebug(true);
  
  // Ejemplo 1: Leer configuración del perfil M0
  Serial.println(F("\n--- Leyendo perfil M0 ---"));
  readAndPrintProfile(0);
  
  // Ejemplo 2: Configurar un nuevo perfil M5
  Serial.println(F("\n--- Configurando perfil M5 ---"));
  configureProfile(5, 24.0, 5.0); // 24V, 5A
  
  // Ejemplo 3: Leer el perfil M5 configurado
  Serial.println(F("\n--- Leyendo perfil M5 configurado ---"));
  readAndPrintProfile(5);
  
  // Ejemplo 4: Configurar protecciones para M5
  Serial.println(F("\n--- Configurando protecciones M5 ---"));
  configureProtections(5, 22.0, 26.0, 6.0); // LVP 22V, OVP 26V, OCP 6A
  
  Serial.println(F("\n=== Ejemplo completado ==="));
}

void loop() {
  // No hay nada que hacer en el loop para este ejemplo
  delay(10000); // Esperar 10 segundos antes de repetir
}

// Función para leer y mostrar un perfil completo
void readAndPrintProfile(uint8_t profile) {
  Serial.print(F("Perfil M")); Serial.println(profile);
  
  // Leer voltaje objetivo
  uint16_t vSet = mppt.readProfileReg(profile, PROFILE_V_SET);
  if (vSet != 0xFFFF) {
    Serial.print(F("  Voltaje objetivo: "));
    Serial.print(vSet / 100.0); // 2 decimales
    Serial.println(F("V"));
  } else {
    Serial.println(F("  Error leyendo voltaje objetivo"));
  }
  
  // Leer corriente objetivo
  uint16_t iSet = mppt.readProfileReg(profile, PROFILE_I_SET);
  if (iSet != 0xFFFF) {
    Serial.print(F("  Corriente objetivo: "));
    Serial.print(iSet / 1000.0); // 3 decimales
    Serial.println(F("A"));
  } else {
    Serial.println(F("  Error leyendo corriente objetivo"));
  }
  
  // Leer protecciones básicas
  uint16_t lvp = mppt.readProfileReg(profile, PROFILE_S_LVP);
  uint16_t ovp = mppt.readProfileReg(profile, PROFILE_S_OVP);
  uint16_t ocp = mppt.readProfileReg(profile, PROFILE_S_OCP);
  
  if (lvp != 0xFFFF) {
    Serial.print(F("  Protección subtensión (LVP): "));
    Serial.print(lvp / 100.0);
    Serial.println(F("V"));
  }
  
  if (ovp != 0xFFFF) {
    Serial.print(F("  Protección sobretensión (OVP): "));
    Serial.print(ovp / 100.0);
    Serial.println(F("V"));
  }
  
  if (ocp != 0xFFFF) {
    Serial.print(F("  Protección sobrecorriente (OCP): "));
    Serial.print(ocp / 1000.0);
    Serial.println(F("A"));
  }
}

// Función para configurar un perfil con voltaje y corriente básicos
void configureProfile(uint8_t profile, float voltage, float current) {
  Serial.print(F("Configurando M")); 
  Serial.print(profile);
  Serial.print(F(": "));
  Serial.print(voltage);
  Serial.print(F("V, "));
  Serial.print(current);
  Serial.println(F("A"));
  
  // Convertir valores a formato raw
  uint16_t vRaw = voltage * 100;   // 2 decimales
  uint16_t iRaw = current * 1000;  // 3 decimales
  
  // Escribir voltaje objetivo
  bool vSuccess = mppt.writeProfileReg(profile, PROFILE_V_SET, vRaw);
  
  // Escribir corriente objetivo
  bool iSuccess = mppt.writeProfileReg(profile, PROFILE_I_SET, iRaw);
  
  if (vSuccess && iSuccess) {
    Serial.println(F("  ✓ Perfil configurado exitosamente"));
  } else {
    Serial.println(F("  ✗ Error configurando perfil"));
  }
}

// Función para configurar protecciones de un perfil
void configureProtections(uint8_t profile, float lvp, float ovp, float ocp) {
  Serial.print(F("Configurando protecciones M")); 
  Serial.print(profile);
  Serial.print(F(": LVP="));
  Serial.print(lvp);
  Serial.print(F("V, OVP="));
  Serial.print(ovp);
  Serial.print(F("V, OCP="));
  Serial.print(ocp);
  Serial.println(F("A"));
  
  // Convertir valores a formato raw
  uint16_t lvpRaw = lvp * 100;   // 2 decimales
  uint16_t ovpRaw = ovp * 100;   // 2 decimales
  uint16_t ocpRaw = ocp * 1000;  // 3 decimales
  
  // Escribir protecciones
  bool lvpSuccess = mppt.writeProfileReg(profile, PROFILE_S_LVP, lvpRaw);
  bool ovpSuccess = mppt.writeProfileReg(profile, PROFILE_S_OVP, ovpRaw);
  bool ocpSuccess = mppt.writeProfileReg(profile, PROFILE_S_OCP, ocpRaw);
  
  if (lvpSuccess && ovpSuccess && ocpSuccess) {
    Serial.println(F("  ✓ Protecciones configuradas exitosamente"));
  } else {
    Serial.println(F("  ✗ Error configurando protecciones"));
  }
}