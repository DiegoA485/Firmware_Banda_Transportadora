
#ifndef FSM_CONTROL_H
#define FSM_CONTROL_H

#include <Arduino.h>

// Estados de la banda transportadora
enum class State {
    IDLE,
    RUNNING,
    FAULT,
    COUNT
};

// Eventos del sistema
enum class Event {
    NONE,
    START,
    EMERGENCY,
    RESET,
    COUNT
};

// Estructura de transición
struct Transition {
    State nextState;
    void (*action)();
};

// Funciones de la FSM
void fsmInit();
void processEvent(Event event);
State getCurrentState();

#endif
