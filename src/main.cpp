/**
 * Proyecto Arduino UNO - Blink LED básico
 * Programa de parpadeo del LED integrado (pin 13)
 * 
 * Este código hace parpadear el LED integrado en el Arduino UNO
 * con un intervalo de 1 segundo (500ms encendido, 500ms apagado)
 */

#include <Arduino.h>

// Pin del LED integrado en Arduino UNO
const int LED_PIN = 13;

void setup() {
  // Inicializar el pin del LED como salida
  pinMode(LED_PIN, OUTPUT);
  
  // Opcional: inicializar comunicación serial para debugging
  Serial.begin(9600);
  Serial.println("Proyecto Blink LED - Arduino UNO iniciado");
}

void loop() {
  // Encender el LED
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED ENCENDIDO");
  
  // Esperar 500 milisegundos
  delay(500);
  
  // Apagar el LED
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED APAGADO");
  
  // Esperar 500 milisegundos
  delay(500);
}