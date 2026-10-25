
#pragma once

#include <Arduino.h>

// Estados de la banda transportadora
enum class State : uint8_t {
    IDLE = 0,
    RUNNING,
    FAULT,
    COUNT
};

// Eventos del sistema
enum class Event : uint8_t {
    NONE = 0,
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

// Funciones de la máquina de estados
void fsmInit();
void processEvent(Event event);
State getCurrentState();
