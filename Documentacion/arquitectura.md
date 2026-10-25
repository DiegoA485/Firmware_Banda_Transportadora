# Arquitectura del Proyecto
## Firmware de Banda Transportadora con ESP32

### 1. Descripción general

El proyecto consiste en desarrollar un firmware para controlar una banda transportadora utilizando un microcontrolador ESP32 integrado en la placa IdeaBoard.

El sistema utiliza PWM por hardware para controlar la velocidad del motor, una máquina de estados finitos (FSM) para gestionar su funcionamiento y una interrupción para detectar la activación del botón de paro de emergencia.

Además, implementa mecanismos de concurrencia de FreeRTOS para proteger las variables compartidas entre la interrupción y el programa principal, evitando condiciones de carrera.

### 2. Arquitectura del software

El firmware está dividido en tres módulos principales:

#### main.cpp

Es el archivo principal del programa.

**Responsabilidades:**
- Inicializar el sistema.
- Configurar el botón de emergencia en GPIO33.
- Gestionar la interrupción de hardware (ISR).
- Implementar el debounce mediante el contador de ciclos del procesador.
- Proteger las variables compartidas mediante spinlocks de FreeRTOS.
- Detectar y procesar los eventos de emergencia.
- Recibir los comandos START y RESET mediante comunicación serial.
- Enviar los eventos correspondientes a la FSM.
- Registrar y mostrar el contador de pulsaciones del botón de emergencia.

#### hal_motor.h / hal_motor.cpp

Módulo encargado del control del motor mediante el puente H L9110S integrado en la IdeaBoard.

**Responsabilidades:**
- Configurar el periférico PWM LEDC del ESP32.
- Establecer la frecuencia y resolución del PWM.
- Controlar la velocidad del motor.
- Ejecutar el movimiento hacia adelante.
- Aplicar frenado dinámico.
- Permitir el movimiento libre por inercia.

#### fsm_control.h / fsm_control.cpp

Módulo encargado de la máquina de estados finitos.

**Responsabilidades:**
- Definir los estados del sistema mediante `enum class`.
- Definir los eventos del sistema.
- Implementar una matriz bidimensional `constexpr` de transiciones.
- Gestionar los cambios de estado.
- Ejecutar las acciones correspondientes al estado.
- Coordinar el funcionamiento del motor según los eventos recibidos.

La matriz de transiciones permite determinar el siguiente estado mediante una consulta de complejidad constante O(1).

### 3. Estados del sistema

**IDLE:**

Es el estado inicial del sistema. El motor permanece detenido y sus salidas se configuran para permitir el movimiento libre por inercia mediante `motorCoast()`.

**RUNNING:**

Es el estado de funcionamiento. El motor gira hacia adelante utilizando PWM mediante la función `motorForward()`.

La velocidad configurada actualmente corresponde a un valor de 700 sobre un máximo de 1023, equivalente aproximadamente al 68,4 % del ciclo de trabajo.

**FAULT:**

Es el estado de emergencia. Se activa al detectar el botón de paro de emergencia y ejecuta la función `motorBrake()` para aplicar frenado dinámico al motor.

Mientras el sistema permanezca en este estado, no se permite iniciar nuevamente el motor mediante el comando START.

### 4. Eventos del sistema

**NONE:**

No se presenta ningún evento que requiera una transición de estado.

**START:**

Solicita iniciar el funcionamiento del motor desde el estado IDLE.

Se activa mediante el comando `START` enviado desde el monitor serial.

**EMERGENCY:**

Se genera al detectar la activación del botón de paro de emergencia conectado al GPIO33.

Provoca la transición al estado FAULT y aplica frenado dinámico al motor.

**RESET:**

Permite regresar del estado FAULT al estado IDLE mediante el comando `RESET` enviado desde el monitor serial.

Para aceptar este comando, el botón de emergencia debe estar liberado.

Los comandos START y RESET están implementados en `main.cpp` y se procesan mediante la máquina de estados finitos.

### 5. Hardware utilizado

**Microcontrolador:**

ESP32 integrado en la placa IdeaBoard.

**Controlador del motor:**

Puente H L9110S integrado en la IdeaBoard.

**Motor:**

Motorreductor DC utilizado para accionar la banda transportadora.

**Conexiones:**
- GPIO12: MOTOR_A.
- GPIO14: MOTOR_B.
- GPIO33: botón de paro de emergencia conectado a GND.

El botón utiliza la resistencia pull-up interna del ESP32 mediante `INPUT_PULLUP`.

**Configuración PWM:**
- Periférico: LEDC.
- Temporizador: `LEDC_TIMER_0`.
- Modo: `LEDC_HIGH_SPEED_MODE`.
- Frecuencia: 1000 Hz.
- Resolución: 10 bits.
- Velocidad configurada: 700/1023.
- Canal 0: MOTOR_A.
- Canal 1: MOTOR_B.

El motor dispone de tres operaciones principales:

- `motorForward()`: movimiento hacia adelante mediante PWM.
- `motorBrake()`: frenado dinámico.
- `motorCoast()`: movimiento libre por inercia.

Para la alimentación física del motor, el proyecto contempla una fuente externa de 5 V y 3 A y la configuración del jumper SELECT-Vin, según las instrucciones del profesor.

### 6. Seguridad y concurrencia

La interrupción de emergencia utiliza:

- ISR con el atributo `IRAM_ATTR`.
- Interrupción por flanco de bajada (`FALLING`).
- Debounce mediante `XTHAL_GET_CCOUNT()`.
- Intervalo mínimo de 4.000.000 de ciclos del procesador entre interrupciones aceptadas, excepto la primera pulsación.
- Variables compartidas declaradas como `volatile`.
- Spinlocks de FreeRTOS mediante `portMUX_TYPE`.
- Secciones críticas mediante `portENTER_CRITICAL_ISR()` y `portENTER_CRITICAL()`.

El tiempo equivalente a 4.000.000 de ciclos depende de la frecuencia del procesador. Corresponde aproximadamente a 25 ms a 160 MHz y a 16,7 ms a 240 MHz.

La ISR registra el evento de emergencia y actualiza el contador de pulsaciones. Posteriormente, el bucle principal consulta de forma protegida las variables compartidas y procesa el evento mediante la FSM.

La primera interrupción válida puede ser aceptada inmediatamente mediante una bandera de inicialización, evitando que sea descartada por el valor inicial del contador de ciclos.

El programa también comprueba que el botón de emergencia esté liberado antes de aceptar los comandos START y RESET.

El bucle principal no utiliza instrucciones `delay()`, permitiendo el procesamiento continuo de los eventos sin esperas bloqueantes explícitas.

El paro de emergencia implementado es una función de software y no sustituye un sistema físico de seguridad independiente. Las comprobaciones del botón reducen los riesgos de operación indebida, pero no garantizan la eliminación de todas las condiciones de carrera.

### 7. Estado actual del desarrollo

- Control PWM del motor implementado.
- Configuración PWM de 1000 Hz y 10 bits.
- Funciones de avance, frenado dinámico e inercia implementadas.
- Interrupción de emergencia en GPIO33 implementada.
- Debounce mediante contador de ciclos implementado.
- Protección de variables compartidas mediante spinlocks de FreeRTOS.
- Máquina de estados finitos implementada mediante matriz `constexpr`.
- Comandos seriales START y RESET implementados.
- Contador de pulsaciones de emergencia implementado.
- Arquitectura modular organizada.
- Compilación realizada correctamente en PlatformIO.
- Repositorio GitHub creado con historial de commits.
- Últimas correcciones pendientes de actualizar en GitHub.
- Pruebas físicas con la IdeaBoard pendientes.

El firmware se encuentra implementado y compila correctamente. Sin embargo, todavía no se ha verificado su funcionamiento con el hardware físico, por lo que las pruebas del motor, del botón de emergencia y de las transiciones de estado permanecen pendientes.