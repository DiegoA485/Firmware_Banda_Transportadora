
#ifndef HAL_MOTOR_H
#define HAL_MOTOR_H

#include <Arduino.h>

// Pines del puente H L9110S
#define MOTOR_A 12
#define MOTOR_B 14

// Configuración PWM
#define PWM_FREQ 1000
#define PWM_RES LEDC_TIMER_10_BIT
#define MOTOR_SPEED 700

// Declaración de funciones
void motorInit();
void motorForward(uint32_t velocidad);
void motorBrake();
void motorCoast();

#endif
