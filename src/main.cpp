
#include <Arduino.h>
#include "xtensa/core-macros.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#include "hal_motor.h"
#include "fsm_control.h"

// ==========================================
// CONFIGURACION DE EMERGENCIA
// ==========================================

constexpr uint8_t BUTTON_PIN = 33;
constexpr uint32_t DEBOUNCE_CYCLES = 4000000UL;

// Variables compartidas con la ISR
volatile bool emergencyEvent = false;
volatile uint32_t pressCount = 0;
volatile uint32_t lastCycle = 0;

portMUX_TYPE isrMux = portMUX_INITIALIZER_UNLOCKED;

// ==========================================
// INTERRUPCION DE EMERGENCIA
// ==========================================

void IRAM_ATTR emergencyISR() {

    uint32_t currentCycles = XTHAL_GET_CCOUNT();

    portENTER_CRITICAL_ISR(&isrMux);

    if ((uint32_t)(currentCycles - lastCycle)
        >= DEBOUNCE_CYCLES) {

        emergencyEvent = true;
        pressCount++;
        lastCycle = currentCycles;
    }

    portEXIT_CRITICAL_ISR(&isrMux);
}

// ==========================================
// COMANDOS SERIALES NO BLOQUEANTES
// ==========================================

char serialBuffer[16];
uint8_t serialIndex = 0;

void processSerialCommand() {

    if (Serial.available() == 0) {
        return;
    }

    char received = Serial.read();

    if (received == '\r') {
        return;
    }

    if (received != '\n') {

        if (serialIndex < sizeof(serialBuffer) - 1) {
            serialBuffer[serialIndex++] = received;
        } else {
            serialIndex = 0;
        }

        return;
    }

    serialBuffer[serialIndex] = '\0';
    serialIndex = 0;

    String command = String(serialBuffer);
    command.trim();
    command.toUpperCase();

    if (command == "START") {

        if (digitalRead(BUTTON_PIN) == LOW) {
            Serial.println(
                "[ERROR] Emergencia activada"
            );
            return;
        }

        if (getCurrentState() == State::IDLE) {
            processEvent(Event::START);
        }

    } else if (command == "RESET") {

        if (digitalRead(BUTTON_PIN) == LOW) {
            Serial.println(
                "[ERROR] Libere el boton de emergencia"
            );
            return;
        }

        if (getCurrentState() == State::FAULT) {
            processEvent(Event::RESET);
        }

    } else if (command.length() > 0) {

        Serial.println("[ERROR] Comando desconocido");
    }
}

// ==========================================
// CONFIGURACION INICIAL
// ==========================================

void setup() {

    Serial.begin(115200);

    // Configurar boton antes de habilitar la ISR
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Inicializar motor y FSM
    motorInit();
    fsmInit();

    // Si el boton ya esta presionado al arrancar
    if (digitalRead(BUTTON_PIN) == LOW) {
        processEvent(Event::EMERGENCY);
    }

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        emergencyISR,
        FALLING
    );

    Serial.println("Sistema inicializado");
    Serial.println("Comandos: START / RESET");
}

// ==========================================
// BUCLE PRINCIPAL
// ==========================================

void loop() {

    bool eventoDetectado = false;

    // Leer y limpiar bandera de forma segura
    portENTER_CRITICAL(&isrMux);

    eventoDetectado = emergencyEvent;
    emergencyEvent = false;

    portEXIT_CRITICAL(&isrMux);

    // Prioridad para el paro de emergencia
    if (eventoDetectado ||
        digitalRead(BUTTON_PIN) == LOW) {

        if (getCurrentState() != State::FAULT) {
            processEvent(Event::EMERGENCY);
        }
    }

    // Procesar comandos seriales
    processSerialCommand();
}
