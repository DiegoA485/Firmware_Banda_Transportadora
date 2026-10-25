
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
volatile bool firstPress = true;

portMUX_TYPE isrMux = portMUX_INITIALIZER_UNLOCKED;

// ==========================================
// INTERRUPCION DE EMERGENCIA
// ==========================================

void IRAM_ATTR emergencyISR() {

    uint32_t currentCycles = XTHAL_GET_CCOUNT();

    portENTER_CRITICAL_ISR(&isrMux);

    if (firstPress ||
        (uint32_t)(currentCycles - lastCycle)
            >= DEBOUNCE_CYCLES) {

        firstPress = false;
        lastCycle = currentCycles;

        emergencyEvent = true;
        pressCount++;
    }

    portEXIT_CRITICAL_ISR(&isrMux);
}

// ==========================================
// COMANDOS SERIALES
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

        // No arrancar con emergencia presionada
        if (digitalRead(BUTTON_PIN) == LOW) {
            Serial.println("[ERROR] Emergencia activada");
            return;
        }

        // No arrancar si existe un evento pendiente
        bool pendingEmergency;

        portENTER_CRITICAL(&isrMux);
        pendingEmergency = emergencyEvent;
        portEXIT_CRITICAL(&isrMux);

        if (pendingEmergency) {
            Serial.println("[ERROR] Emergencia pendiente");
            return;
        }

        if (getCurrentState() == State::IDLE) {
            processEvent(Event::START);
        }

    } else if (command == "RESET") {

        if (digitalRead(BUTTON_PIN) == LOW) {
            Serial.println("[ERROR] Libere la emergencia");
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

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    motorInit();
    fsmInit();

    // Detectar emergencia al encender
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
    uint32_t currentCount = 0;

    // Acceso protegido a variables compartidas
    portENTER_CRITICAL(&isrMux);

    eventoDetectado = emergencyEvent;
    emergencyEvent = false;
    currentCount = pressCount;

    portEXIT_CRITICAL(&isrMux);

    // La emergencia tiene prioridad
    if (eventoDetectado ||
        digitalRead(BUTTON_PIN) == LOW) {

        if (getCurrentState() != State::FAULT) {
            processEvent(Event::EMERGENCY);
        }

        if (eventoDetectado) {
            Serial.printf(
                "[Metrics] Pulsaciones: %lu\n",
                (unsigned long)currentCount
            );
        }
    }

    processSerialCommand();

    // Verificacion adicional tras procesar comandos
    if (digitalRead(BUTTON_PIN) == LOW &&
        getCurrentState() != State::FAULT) {

        processEvent(Event::EMERGENCY);
    }
}
