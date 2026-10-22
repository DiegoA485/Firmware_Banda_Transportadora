#include <Arduino.h>
#include "driver/ledc.h"
#include "xtensa/core-macros.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

// Pines del puente H L9110S
#define MOTOR_A 12
#define MOTOR_B 14

// Configuración PWM
#define PWM_FREQ 1000
#define PWM_RES LEDC_TIMER_10_BIT

// Pin del botón de emergencia
#define BUTTON_PIN 33

// Tiempo de debounce: 4 millones de ciclos
#define DEBOUNCE_CYCLES 4000000UL

// Variables compartidas con la interrupción
volatile bool emergencyEvent = false;
volatile uint32_t pressCount = 0;
volatile uint32_t lastCycle = 0;

// Spinlock para proteger las variables
portMUX_TYPE isrMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR emergencyISR() {

    uint32_t currentCycles = XTHAL_GET_CCOUNT();

    portENTER_CRITICAL_ISR(&isrMux);

    if ((uint32_t)(currentCycles - lastCycle) >= DEBOUNCE_CYCLES) {

        emergencyEvent = true;
        pressCount++;
        lastCycle = currentCycles;
    }

    portEXIT_CRITICAL_ISR(&isrMux);
}

// Motor hacia adelante
void motorForward(uint32_t velocidad) {
    if (velocidad > 1023) velocidad = 1023;

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, velocidad);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

// Frenado dinámico
void motorBrake() {
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 1023);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 1023);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
}

// Motor libre por inercia
void motorCoast() {
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
}



void setup() {

    ledc_timer_config_t timer = {};
    timer.speed_mode = LEDC_HIGH_SPEED_MODE;
    timer.duty_resolution = PWM_RES;
    timer.timer_num = LEDC_TIMER_0;
    timer.freq_hz = PWM_FREQ;
    timer.clk_cfg = LEDC_AUTO_CLK;

    ledc_timer_config(&timer);

    ledc_channel_config_t canalA = {};
    canalA.gpio_num = MOTOR_A;
    canalA.speed_mode = LEDC_HIGH_SPEED_MODE;
    canalA.channel = LEDC_CHANNEL_0;
    canalA.timer_sel = LEDC_TIMER_0;
    canalA.duty = 0;

    ledc_channel_config(&canalA);

    ledc_channel_config_t canalB = canalA;
    canalB.gpio_num = MOTOR_B;
    canalB.channel = LEDC_CHANNEL_1;

    ledc_channel_config(&canalB);


// Configurar botón de emergencia
pinMode(BUTTON_PIN, INPUT_PULLUP);

// Activar interrupción en flanco de bajada
attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    emergencyISR,
    FALLING
);

} // Cierre de setup()

void loop() {

    bool eventoDetectado = false;

    // Acceder de forma segura a la variable compartida
    portENTER_CRITICAL(&isrMux);

    eventoDetectado = emergencyEvent;
    emergencyEvent = false;

    portEXIT_CRITICAL(&isrMux);

    // Procesar el evento fuera de la sección crítica
    if (eventoDetectado) {
        // Aquí conectaremos posteriormente la FSM
    }
}