
#include "hal_motor.h"
#include "driver/ledc.h"

// ==========================================
// INICIALIZACIÓN DEL MOTOR
// ==========================================

void motorInit() {

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
}

// ==========================================
// MOTOR HACIA ADELANTE
// ==========================================

void motorForward(uint32_t velocidad) {

    if (velocidad > 1023) velocidad = 1023;

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, velocidad);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

// ==========================================
// FRENADO DINÁMICO
// ==========================================

void motorBrake() {

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 1023);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 1023);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
}

// ==========================================
// MOTOR LIBRE POR INERCIA
// ==========================================

void motorCoast() {

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
}
