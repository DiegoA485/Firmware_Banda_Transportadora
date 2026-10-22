
#include "fsm_control.h"
#include "hal_motor.h"

// ==========================================
// ESTADO ACTUAL DE LA FSM
// ==========================================

static State currentState = State::IDLE;

// ==========================================
// ACCIONES DE LOS ESTADOS
// ==========================================

void onEnterIdle() {
    motorCoast();
    Serial.println("[FSM] -> IDLE");
}

void onEnterRunning() {
    motorForward(MOTOR_SPEED);
    Serial.println("[FSM] -> RUNNING");
}

void onEnterFault() {
    motorBrake();
    Serial.println("[FSM] -> FAULT");
}

void onNoOp() {
    // Sin cambios
}

// ==========================================
// MATRIZ DE TRANSICIONES CONSTEXPR
// ==========================================

// Columnas: NONE, START, EMERGENCY, RESET
// Filas: IDLE, RUNNING, FAULT

constexpr Transition transitionTable[3][4] = {

    // Estado IDLE
    {
        {State::IDLE, onNoOp},
        {State::RUNNING, onEnterRunning},
        {State::FAULT, onEnterFault},
        {State::IDLE, onNoOp}
    },

    // Estado RUNNING
    {
        {State::RUNNING, onNoOp},
        {State::RUNNING, onNoOp},
        {State::FAULT, onEnterFault},
        {State::RUNNING, onNoOp}
    },

    // Estado FAULT
    {
        {State::FAULT, onNoOp},
        {State::FAULT, onNoOp},
        {State::FAULT, onNoOp},
        {State::IDLE, onEnterIdle}
    }
};

// ==========================================
// INICIALIZACIÓN DE LA FSM
// ==========================================

void fsmInit() {

    currentState = State::IDLE;
    onEnterIdle();
}

// ==========================================
// PROCESAMIENTO DE EVENTOS
// ==========================================

void processEvent(Event event) {

    size_t stateIndex =
        static_cast<size_t>(currentState);

    size_t eventIndex =
        static_cast<size_t>(event);

    if (stateIndex >= static_cast<size_t>(State::COUNT) ||
        eventIndex >= static_cast<size_t>(Event::COUNT)) {
        return;
    }

    Transition transition =
        transitionTable[stateIndex][eventIndex];

    if (transition.nextState != currentState) {

        currentState = transition.nextState;

        if (transition.action != nullptr) {
            transition.action();
        }
    }
}

// ==========================================
// CONSULTAR ESTADO ACTUAL
// ==========================================

State getCurrentState() {
    return currentState;
}
