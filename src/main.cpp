
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#include "hal_motor.h"
#include "fsm_control.h"

// ==========================================
// FASE 3: INTERRUPCIÓN DE EMERGENCIA
// ==========================================

#define BUTTON_PIN 33
#define DEBOUNCE_MS 25

// Variables compartidas con la interrupción
volatile bool emergencyEvent = false;
volatile uint32_t pressCount = 0;
volatile uint32_t lastPressMs = 0;

// Protección de variables compartidas
portMUX_TYPE isrMux = portMUX_INITIALIZER_UNLOCKED;

// ==========================================
// INTERRUPCIÓN ISR
// ==========================================

void IRAM_ATTR emergencyISR() {

    uint32_t currentMs = millis();

    portENTER_CRITICAL_ISR(&isrMux);

    if ((uint32_t)(currentMs - lastPressMs)
        >= DEBOUNCE_MS) {

        emergencyEvent = true;
        pressCount++;
        lastPressMs = currentMs;
    }

    portEXIT_CRITICAL_ISR(&isrMux);
}

// ==========================================
// CONFIGURACIÓN INICIAL
// ==========================================

void setup() {

    Serial.begin(115200);

    // Inicializar controlador PWM del motor
    motorInit();

    // Inicializar máquina de estados
    fsmInit();

    // Configurar botón de emergencia
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Activar interrupción
    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        emergencyISR,
        FALLING
    );

    Serial.println("Sistema inicializado");
}

// ==========================================
// BUCLE PRINCIPAL
// ==========================================

void loop() {

    bool eventoDetectado = false;

    // Leer evento de manera segura
    portENTER_CRITICAL(&isrMux);

    eventoDetectado = emergencyEvent;
    emergencyEvent = false;

    portEXIT_CRITICAL(&isrMux);

    // Procesar únicamente el paro de emergencia
    if (eventoDetectado) {

        processEvent(Event::EMERGENCY);
    }
}
