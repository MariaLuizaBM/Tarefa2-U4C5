#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#define LED_PIN_RED 12 // Pino do LED vermelho conectado ao GPIO 12
#define LED_PIN_BLUE 11 // Pino do LED azul conectado ao GPIO 11
#define LED_PIN_GREEN 13 // Pino do LED verde conectado ao GPIO 13
#define BUTTON_PIN 5 // Pino do botão conectado ao GPIO 5

bool led_red_on = false;
bool led_blue_on = false;
bool led_green_on = false;
bool button_pressed = false;
int state = 0; // Variável para rastrear o estado atual

struct repeating_timer timer;

// Função de callback para desligar os LEDs em sequência
int64_t turn_off_callback(alarm_id_t id, void *user_data) {
    
    if (state == 0) {
        gpio_put(LED_PIN_BLUE, 0);
        state = 1;
        // Configura o próximo alarme para desligar o próximo LED após 3 segundos
        add_alarm_in_ms(3000, turn_off_callback, NULL, false);
    } else if (state == 1) {
        gpio_put(LED_PIN_RED, 0);
        state = 2;
        // Configura o próximo alarme para desligar o próximo LED após 3 segundos
        add_alarm_in_ms(3000, turn_off_callback, NULL, false);
    } else if (state == 2) {
        gpio_put(LED_PIN_GREEN, 0);
        state = 0;
        led_red_on = false;
        led_blue_on = false;
        led_green_on = false;
    }
    return 0; // Retorna 0 para indicar que o alarme não deve se repetir automaticamente
}

// Função de callback para ligar os LEDs por 3 segundos
bool turn_on_leds_callback(struct repeating_timer *t) {
    gpio_put(LED_PIN_RED, true);
    gpio_put(LED_PIN_BLUE, true);
    gpio_put(LED_PIN_GREEN, true);
    led_red_on = true;
    led_blue_on = true;
    led_green_on = true;

    // Após 3 segundos, iniciar a sequência de desligamento dos LEDs
    add_alarm_in_ms(3000, turn_off_callback, NULL, false);

    return false; // Retorna false para parar o temporizador depois de ligar os LEDs
}

int main() {
    stdio_init_all();

    // Inicializa os pinos dos LEDs
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);

    // Inicializa o pino do botão
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    while (true) {
        // Verifica se o botão foi pressionado e todos os LEDs estão desligados
        if (!gpio_get(BUTTON_PIN) && !button_pressed && !led_red_on && !led_blue_on && !led_green_on) {
            // Adiciona um pequeno atraso para debounce, evitando leituras errôneas.
            sleep_ms(50);
            if(gpio_get(BUTTON_PIN) == 0){
                button_pressed = true;

                 // Inicia o temporizador para ligar os LEDs por 3 segundos
                add_repeating_timer_ms(0, turn_on_leds_callback, NULL, &timer);
            }
        }

        // Redefine o estado do botão quando for solto
        if (gpio_get(BUTTON_PIN) && button_pressed) {
            button_pressed = false;
        }

        // Introduz uma pequena pausa de 10 ms para reduzir o uso da CPU.
        // Isso evita que o loop seja executado muito rapidamente e consuma recursos desnecessários.
        sleep_ms(10);
    }

    // Retorno de 0, que nunca será alcançado devido ao loop infinito.
    // Isso é apenas uma boa prática em programas com um ponto de entrada main().
    return 0;
}
