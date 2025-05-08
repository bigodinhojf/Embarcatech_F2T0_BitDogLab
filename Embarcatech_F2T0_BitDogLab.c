// -- Inclusão de bibliotecas
#include <stdio.h>
#include "pico/stdlib.h"
#include "lib/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "pico/bootrom.h"

// -- Definição de constantes
// GPIO
#define button_A 5 // Botão A GPIO 5
#define button_B 6 // Botão B GPIO 6
#define matriz_leds 7 // Matriz de LEDs GPIO 7
#define NUM_LEDS 25 // Número de LEDs na matriz
#define buzzer_A 21 // Buzzer A GPIO 21
#define buzzer_B 10 // Buzzer B GPIO 10
#define LED_Green 11 // LED Verde GPIO 11
#define LED_Blue 12 // LED Azul GPIO 12
#define LED_Red 13 // LED Vermelho GPIO 13
#define joystick_PB 22 // Botão do joystick GPjoystick_PB
#define joystick_Y 26 // VRY do Joystick GPIO 26
#define joystick_X 27 // VRX do Joystick GPIO 27

// Display I2C
#define display_i2c_port i2c1 // Define a porta I2C
#define display_i2c_sda 14 // Define o pino SDA na GPIO 14
#define display_i2c_scl 15 // Define o pino SCL na GPIO 15
#define display_i2c_endereco 0x3C // Define o endereço do I2C
ssd1306_t ssd; // Inicializa a estrutura do display

// PWM
uint slice_A;
uint slice_B;

// -- Variáveis globais
static volatile uint32_t last_time = 0; // Armazena o tempo do último clique dos botões
volatile bool preenchimento = false; // Armazena o preenchimento do quadrado 8x8
volatile int cor_led = 0; // Armazena um dígito referente a cor do LED RGB

// --- Funções necessária para a manipulação da matriz de LEDs

// Estrutura do pixel GRB (Padrão do WS2812)
struct pixel_t {
    uint8_t G, R, B; // Define variáveis de 8-bits (0 a 255) para armazenar a cor
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Permite declarar variáveis utilizando apenas "npLED_t"

// Declaração da Array que representa a matriz de LEDs
npLED_t leds[NUM_LEDS];

// Variáveis para máquina PIO
PIO np_pio;
uint sm;

// Função para definir a cor de um LED específico
void cor(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

// Função para desligar todos os LEDs
void desliga() {
    for (uint i = 0; i < NUM_LEDS; ++i) {
        cor(i, 0, 0, 0);
    }
}

// Função para enviar o estado atual dos LEDs ao hardware  - buffer de saída
void buffer() {
    for (uint i = 0; i < NUM_LEDS; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
}

// Função para converter a posição da matriz para uma posição do vetor.
int getIndex(int x, int y) {
    // Se a linha for par (0, 2, 4), percorremos da esquerda para a direita.
    // Se a linha for ímpar (1, 3), percorremos da direita para a esquerda.
    if (y % 2 == 0) {
        return 24-(y * 5 + x); // Linha par (esquerda para direita).
    } else {
        return 24-(y * 5 + (4 - x)); // Linha ímpar (direita para esquerda).
    }
}

// --- Final das funções necessária para a manipulação da matriz de LEDs

// Função para definir a posição do quadrado 8x8 no Display
void posicao_quadrado(){
    // Leitura dos valores do ADC do Joystick
    adc_select_input(0); // Seleciona o ADC0 referente ao VRY do Joystick (GPIO 26)
    uint16_t value_vry = adc_read(); // Ler o valor do ADC selecionado (ADC0 - VRY) e guarda
    adc_select_input(1); // Seleciona o ADC1 referente ao VRX do Joystick (GPIO 27)
    uint16_t value_vrx = adc_read(); // Ler o valor do ADC selecionado (ADC1 - VRX) e guarda

    uint y = (1-(value_vry/4095))*54; // Admensionaliza, multiplica pelo valor máximo e inverte o valor
    uint x = (value_vrx/4095)*118; // Admensionaliza e multiplica pelo valor máximo
    
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_rect(&ssd, y, x, 8, 8, true, preenchimento); // Desenha o quadrado 8x8
    ssd1306_rect(&ssd, 0, 0, 127, 63, true, false); // Desenha a borda do display
}

// Função para definir a cor do LED RGB
void led_rgb(){
    switch (cor_led){
    case 0:
        // Desligado
        gpio_put(LED_Green, false);
        gpio_put(LED_Blue, false);
        gpio_put(LED_Red, false);
        break;
    case 1:
        // Verde
        gpio_put(LED_Green, true);
        gpio_put(LED_Blue, false);
        gpio_put(LED_Red, false);
        break;
    case 2:
        // Azul
        gpio_put(LED_Green, false);
        gpio_put(LED_Blue, true);
        gpio_put(LED_Red, false);
        break;
    case 3:
        // Vermelho
        gpio_put(LED_Green, false);
        gpio_put(LED_Blue, false);
        gpio_put(LED_Red, true);
        break;
    case 4:
        // Ciano
        gpio_put(LED_Green, true);
        gpio_put(LED_Blue, true);
        gpio_put(LED_Red, false);
        break;
    case 5:
        // Amarelo
        gpio_put(LED_Green, true);
        gpio_put(LED_Blue, false);
        gpio_put(LED_Red, true);
        break;
    case 6:
        // Rosa
        gpio_put(LED_Green, false);
        gpio_put(LED_Blue, true);
        gpio_put(LED_Red, true);
        break;
    case 7:
        // Branco
        gpio_put(LED_Green, true);
        gpio_put(LED_Blue, true);
        gpio_put(LED_Red, true);
        break;
    default:
        break;
    }
}

// Função do sinal sonoro dos buzzers
void beep_buzzer(){
    pwm_set_enabled(slice_A, true);
    pwm_set_enabled(slice_B, true);
    sleep_ms(100);
    pwm_set_enabled(slice_A, false);
    pwm_set_enabled(slice_B, false);
}

// Função que guarda as animações da matriz de LEDs
void num_matriz_leds(char c){
    desliga();
    switch (c){
        case '0': {
            // Frame 0
            int frame0[5][5][3] = {
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},    
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}}
            };
            for (int linha = 0; linha < 5; linha++)
            {
                for (int coluna = 0; coluna < 5; coluna++)
                {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame0[coluna][linha][0], frame0[coluna][linha][1], frame0[coluna][linha][2]);
                }
            };
            buffer();
            break;
        }
        case '1':{
            // Frame 1
            int frame1[5][5][3] = {
                {{0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}}
            };
            for (int linha = 0; linha < 5; linha++)
            {
                for (int coluna = 0; coluna < 5; coluna++)
                {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame1[coluna][linha][0], frame1[coluna][linha][1], frame1[coluna][linha][2]);
                }
            };
            buffer();
            break;
        }
        case '2':{
            // Frame 2
            int frame2[5][5][3] = {
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},        
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}}
            };
            for (int linha = 0; linha < 5; linha++)
            {
                for (int coluna = 0; coluna < 5; coluna++)
                {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame2[coluna][linha][0], frame2[coluna][linha][1], frame2[coluna][linha][2]);
                }
            };
            buffer();
            break;
        }
        case '3':{
            // Frame 3
            int frame3[5][5][3] = {
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}}
            };
            for (int linha = 0; linha < 5; linha++)
            {
                for (int coluna = 0; coluna < 5; coluna++)
                {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame3[coluna][linha][0], frame3[coluna][linha][1], frame3[coluna][linha][2]);
                }
            };
            buffer();
            break;
        }
        case '4':{
            // Frame 4
            int frame4[5][5][3] = {
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},    
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}} 
            };
            for (int linha = 0; linha < 5; linha++)
            {
                for (int coluna = 0; coluna < 5; coluna++)
                {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame4[coluna][linha][0], frame4[coluna][linha][1], frame4[coluna][linha][2]);
                }
            };
            buffer();
            break;
        }
        case '5':{
            // Frame 5
            int frame5[5][5][3] = {
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}}
            };
            for (int linha = 0; linha < 5; linha++)
            {
                for (int coluna = 0; coluna < 5; coluna++)
                {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame5[coluna][linha][0], frame5[coluna][linha][1], frame5[coluna][linha][2]);
                }
            };
            buffer();
            break;
        }
        case '6':{
            // Frame 6
            int frame6[5][5][3] = {
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},        
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},    
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}}
            };
            for (int linha = 0; linha < 5; linha++)
            {
                for (int coluna = 0; coluna < 5; coluna++)
                {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame6[coluna][linha][0], frame6[coluna][linha][1], frame6[coluna][linha][2]);
                }
            };
            buffer();
            break;
        }
        case '7':{
            // Frame 7
            int frame7[5][5][3] = {
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}} 
            };
            for (int linha = 0; linha < 5; linha++)
            {
                for (int coluna = 0; coluna < 5; coluna++)
                {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame7[coluna][linha][0], frame7[coluna][linha][1], frame7[coluna][linha][2]);
                }
            };
            buffer();
            break;
        }
        case '8':{
            // Frame 8
            int frame8[5][5][3] = {
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},    
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}}
            };
            for (int linha = 0; linha < 5; linha++)
            {
                for (int coluna = 0; coluna < 5; coluna++)
                {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame8[coluna][linha][0], frame8[coluna][linha][1], frame8[coluna][linha][2]);
                }
            };
            buffer();
            break;
        }
        case '9':{
            // Frame 9
            int frame9[5][5][3] = {
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 90, 90}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},    
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 90, 90}, {0, 0, 0}},       
                {{0, 0, 0}, {0, 90, 90}, {0, 90, 90}, {0, 90, 90}, {0, 0, 0}}
            };
            for (int linha = 0; linha < 5; linha++)
            {
                for (int coluna = 0; coluna < 5; coluna++)
                {
                int posicao = getIndex(linha, coluna);
                cor(posicao, frame9[coluna][linha][0], frame9[coluna][linha][1], frame9[coluna][linha][2]);
                }
            };
            buffer();
            break;
        }
        default:
            break;
    }
}

// Função de interrupção dos botões
void gpio_irq_handler(uint gpio, uint32_t events){
    //Debouncing
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Pega o tempo atual e transforma em us
    if(current_time - last_time > 1000000){
        if(gpio == button_A){
            if(cor_led >= 8){
                cor_led = 0;
            }else{
                cor_led += 1;
            }
            led_rgb();
        }else if(gpio == button_B){
            reset_usb_boot(0, 0);
        }else if(gpio == joystick_PB){
            preenchimento == !preenchimento;
        }
        beep_buzzer();
    }
}

int main(){
    // -- Inicializações
    // Monitor serial
    stdio_init_all();
    
    // GPIO
    gpio_init(button_A); // Inicia a GPIO 5 do botão A
    gpio_set_dir(button_A, GPIO_IN); // Define a direção da GPIO 5 do botão A como entrada
    gpio_pull_up(button_A); // Habilita o resistor de pull up da GPIO 5 do botão A
    
    gpio_init(button_B); // Inicia a GPIO 6 do botão B
    gpio_set_dir(button_B, GPIO_IN); // Define a direção da GPIO 6 do botão B como entrada
    gpio_pull_up(button_B); // Habilita o resistor de pull up da GPIO 6 do botão B

    gpio_init(joystick_PB); // Inicia a GPIO 22 do botão do Joystick
    gpio_set_dir(joystick_PB, GPIO_IN); // Define a direção da GPIO 22 do botão do Joystick como entrada
    gpio_pull_up(joystick_PB); // Habilita o resistor de pull up da GPIO 22 do botão do Joystick

    gpio_init(LED_Green); // Inicia a GPIO 11 do LED Verde
    gpio_set_dir(LED_Green, GPIO_OUT); // Define a direção da GPIO 11 do LED Verde como saída
    gpio_put(LED_Green, false); // Estado inicial do LED apagado

    gpio_init(LED_Blue); // Inicia a GPIO 12 do LED Azul
    gpio_set_dir(LED_Blue, GPIO_OUT); // Define a direção da GPIO 12 do LED Azul como saída
    gpio_put(LED_Blue, false); // Estado inicial do LED apagado

    gpio_init(LED_Red); // Inicia a GPIO 13 do LED Vermelho
    gpio_set_dir(LED_Red, GPIO_OUT); // Define a direção da GPIO 13 do LED Vermelho como saída
    gpio_put(LED_Red, false); // Estado inicial do LED aceso

    // Display I2C
    i2c_init(display_i2c_port, 400 * 1000); // Inicializa o I2C usando 400kHz
    gpio_set_function(display_i2c_sda, GPIO_FUNC_I2C); // Define o pino SDA (GPIO 14) na função I2C
    gpio_set_function(display_i2c_scl, GPIO_FUNC_I2C); // Define o pino SCL (GPIO 15) na função I2C
    gpio_pull_up(display_i2c_sda); // Ativa o resistor de pull up para o pino SDA (GPIO 14)
    gpio_pull_up(display_i2c_scl); // Ativa o resistor de pull up para o pino SCL (GPIO 15)
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, display_i2c_endereco, display_i2c_port); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_send_data(&ssd); // Atualiza o display

    // PWM
    gpio_set_function(buzzer_A, GPIO_FUNC_PWM); // Define a função da porta GPIO como PWM
    gpio_set_function(buzzer_B, GPIO_FUNC_PWM); // Define a função da porta GPIO como PWM
    uint freq = 1000; // Frequência do buzzer
    uint clock_div = 4; // Divisor do clock
    uint wrap = (125000000 / (clock_div * freq)) - 1; // Define o valor do wrap para frequência escolhida
    slice_A = pwm_gpio_to_slice_num(buzzer_A); // Define o slice do buzzer A
    slice_B = pwm_gpio_to_slice_num(buzzer_B); // Define o slice do buzzer B
    pwm_set_clkdiv(slice_A, clock_div); // Define o divisor do clock para o buzzer A
    pwm_set_clkdiv(slice_B, clock_div); // Define o divisor do clock para o buzzer B
    pwm_set_wrap(slice_A, wrap); // Define o valor do wrap para o buzzer A
    pwm_set_wrap(slice_B, wrap); // Define o valor do wrap para o buzzer B
    pwm_set_chan_level(slice_A, pwm_gpio_to_channel(buzzer_A), wrap / 40); // Duty cycle para definir o Volume do buzzer A
    pwm_set_chan_level(slice_B, pwm_gpio_to_channel(buzzer_B), wrap / 40); // Duty cycle para definir o volume do buzzer B

    // ADC
    adc_init();
    adc_gpio_init(joystick_Y); // Inicia o ADC para o GPIO 26 do VRY do Joystick
    adc_gpio_init(joystick_X); // Inicia o ADC para o GPIO 27 do VRX do Joystick

    // PIO
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, true);
    uint offset = pio_add_program(pio0, &ws2818b_program);
    ws2818b_program_init(np_pio, sm, offset, matriz_leds, 800000);
    desliga(); // Para limpar o buffer dos LEDs
    buffer();

    // Interrupção dos botões
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(joystick_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Loop principal
    while (true) {
        posicao_quadrado(); // Função que define a posição do quadrado 8x8

        // Monitor serial
        if(stdio_usb_connected()){
            char c; // Variável que guarda o caractere digitado no monitor serial
            if(scanf("%c", &c) == 1){
                printf("Número recebido: %c \n", c);
                if(c >= '0' && c <= '9'){
                    num_matriz_leds(c);
                }else{
                    printf("Número inválido!\n");
                }
            }
        }
        sleep_ms(50);
    }
}