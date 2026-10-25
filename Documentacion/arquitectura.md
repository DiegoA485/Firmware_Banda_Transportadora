
# Arquitectura del Proyecto
## Firmware de Banda Transportadora con ESP32

### 1. Descripción general

El proyecto consiste en desarrollar un firmware para controlar una banda transportadora utilizando un ESP32.

El sistema utiliza PWM para controlar la velocidad del motor, una máquina de estados finitos (FSM) para gestionar su funcionamiento y una interrupción para detectar el botón de paro de emergencia.

### 2. Arquitectura del software

El firmware está dividido en tres módulos principales:

#### main.cpp
Es el archivo principal del programa.

Responsabilidades:
- Inicializar el sistema.
- Configurar el botón de emergencia.
- Gestionar la interrupción (ISR).
- Detectar eventos de emergencia.
- Enviar eventos a la FSM.

#### hal_motor.h / hal_motor.cpp
Módulo encargado del control del motor.

Responsabilidades:
- Configurar el PWM del ESP32.
- Controlar la velocidad del motor.
- Ejecutar el frenado dinámico.
- Permitir el movimiento libre por inercia.

#### fsm_control.h / fsm_control.cpp
Módulo encargado de la máquina de estados finitos.

Responsabilidades:
- Definir los estados del sistema.
- Definir los eventos.
- Gestionar las transiciones.
- Ejecutar las acciones correspondientes.

### 3. Estados del sistema

IDLE:
Estado inicial. El motor permanece detenido.

RUNNING:
Estado de funcionamiento. El motor gira utilizando PWM.

FAULT:
Estado de emergencia. Se aplica frenado dinámico al motor.

### 4. Eventos del sistema

NONE:
No se presenta ningún evento.

START:
Solicita iniciar el funcionamiento del motor.

EMERGENCY:
Se genera al detectar el botón de paro de emergencia.

RESET:
Permite regresar de FAULT a IDLE.

Actualmente, START y RESET están definidos en la FSM, pero todavía no tienen un mecanismo de activación implementado.

### 5. Hardware utilizado

Microcontrolador:
ESP32.

Controlador del motor:
Puente H L9110S.

Conexiones:
- GPIO12: MOTOR_A.
- GPIO14: MOTOR_B.
- GPIO33: Botón de emergencia.

Configuración PWM:
- Frecuencia: 1000 Hz.
- Resolución: 10 bits.
- Velocidad configurada: 700/1023.

### 6. Seguridad y concurrencia

La interrupción de emergencia utiliza:

- ISR para detectar el botón.
- Debounce de 25 ms.
- Variables compartidas volátiles.
- Secciones críticas de FreeRTOS.

La ISR registra el evento y el bucle principal lo procesa mediante la FSM.

El paro de emergencia implementado es una función de software y no sustituye un sistema físico de seguridad.

### 7. Estado actual del desarrollo

- Control PWM implementado.
- Interrupción de emergencia implementada.
- FSM implementada.
- Arquitectura modular organizada.
- Compilación realizada correctamente.
- Pruebas físicas pendientes.
- Activación de START y RESET pendiente.
