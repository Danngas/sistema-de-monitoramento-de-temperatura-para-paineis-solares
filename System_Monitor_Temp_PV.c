#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/graphics.h"
#include "string.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADDR 0x3C

#define TEMP_SENSOR_PIN 26   // GPIO para sensor de temperatura
#define SAMPLE_INTERVAL 1000 // Intervalo de amostragem em ms
#define MAX_SAMPLES 100      // Número máximo de amostras para o gráfico

// Adicionar estas definições no início do arquivo
#define GRAPH_Y_START 20
#define GRAPH_WIDTH 128

// Adicione estas definições no início do arquivo, após os outros #defines
#define BUTTON_A_PIN 5 // GPIO para o botão A
#define BUTTON_B_PIN 6 // GPIO para o botão B

// Buffer circular para armazenar as temperaturas
float temp_history[MAX_SAMPLES];
int current_index = 0;
int num_samples = 0;

// Atualizar a estrutura do gráfico
Graph graph = {
    .x_offset = MARGIN_LEFT,
    .y_offset = 5,
    .width = DISPLAY_WIDTH - MARGIN_LEFT - 5,
    .height = GRAPH_HEIGHT,
    .y_min = 0,
    .y_max = 50,
    .y_pixels = GRAPH_HEIGHT,
    .x_divisions = 5,
    .y_divisions = 4};

// Definições do sistema de menu
#define SPLASH_DURATION 3000 // Duração da tela inicial em ms

// Estados do sistema
typedef enum
{
    STATE_SPLASH,
    STATE_MENU,
    STATE_MONITOR,
    STATE_HISTORY,
    STATE_CONFIG
} SystemState;

// Opções do menu principal
typedef enum
{
    MENU_MONITOR,
    MENU_HISTORY,
    MENU_CONFIG,
    MENU_COUNT
} MenuItem;

// Estrutura para armazenar estatísticas
typedef struct
{
    float min_temp;
    float max_temp;
    float avg_temp;
} TempStats;

// Variáveis globais do sistema
SystemState current_state = STATE_SPLASH;
MenuItem selected_menu_item = MENU_MONITOR;
TempStats temp_stats = {.min_temp = 100.0f, .max_temp = -100.0f, .avg_temp = 0.0f};
uint32_t splash_start_time = 0;

// Variáveis globais para controle de estado
volatile bool button_a_pressed = false;
volatile bool button_b_pressed = false;

// Função de callback compartilhada para ambos os botões
void gpio_callback(uint gpio, uint32_t events)
{
    // Debounce por software
    static uint32_t last_interrupt_time_a = 0;
    static uint32_t last_interrupt_time_b = 0;
    uint32_t interrupt_time = time_us_32();

    if (gpio == BUTTON_A_PIN)
    {
        if (interrupt_time - last_interrupt_time_a > 200000)
        { // 200ms debounce
            button_a_pressed = true;
        }
        last_interrupt_time_a = interrupt_time;
    }
    else if (gpio == BUTTON_B_PIN)
    {
        if (interrupt_time - last_interrupt_time_b > 200000)
        { // 200ms debounce
            button_b_pressed = true;
        }
        last_interrupt_time_b = interrupt_time;
    }
}

// Funções para desenhar as diferentes telas
void draw_splash_screen(ssd1306_t *ssd)
{
    ssd1306_fill(ssd, false);
    ssd1306_line(ssd, 0, 0, 128, 0, true); // desenha uma linha horizontal de (6,5) até (120,5)
    ssd1306_line(ssd, 0, 0, 128, 0, true); // desenha uma linha horizontal de (6,5) até (120,5)
    ssd1306_draw_string(ssd, " System Monitor", 4, 10);
    ssd1306_draw_string(ssd, "Painel Solar", 20, 20);
    ssd1306_draw_string(ssd, "Versao 1.0", 25, 40);
    ssd1306_draw_string(ssd, " A:Menu B:Entrar", 0, 55);
}

void draw_menu_screen(ssd1306_t *ssd)
{
    ssd1306_fill(ssd, false);
    const char *menu_items[] = {"Monitor", "Historico", "Config"};

    for (int i = 0; i < MENU_COUNT; i++)
    {
        char line[32];
        snprintf(line, sizeof(line), "%s%s",
                 (i == selected_menu_item) ? "> " : "  ",
                 menu_items[i]);
        ssd1306_draw_string(ssd, line, 10, 20 + (i * 15));
    }
}

// Função para ler e condicionar o sinal do joystick
float read_joystick_value()
{
    adc_select_input(0);       // Seleciona o canal ADC para o pino 26
    uint16_t raw = adc_read(); // Leitura do ADC de 12 bits (0-4095)

    // Condicionamento do sinal:
    // Mapeia a leitura do ADC (0-4095) para o range desejado (0-52)
    float mapped_value = (float)raw * 52.0f / 4095.0f;

    // Garante que o valor está dentro dos limites
    if (mapped_value < 0.0f)
        mapped_value = 0.0f;
    if (mapped_value > 52.0f)
        mapped_value = 52.0f;

    return mapped_value;
}

int main()
{
    // Inicialização do sistema
    stdio_init_all();

    // Inicialização do ADC
    adc_init();
    adc_gpio_init(TEMP_SENSOR_PIN);

    // Inicialização do I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do display OLED
    ssd1306_t ssd;
    ssd1306_init(&ssd, 128, 64, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&ssd);

    // Inicializa o tempo inicial da tela splash
    splash_start_time = to_ms_since_boot(get_absolute_time());

    // Adicione a inicialização do botão A após as outras inicializações
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    // Adicione a inicialização do botão B junto com a do botão A
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Configuração das interrupções para ambos os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_callback);

    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_callback);

    // Loop principal
    while (true)
    {
        // Tratamento da interrupção do botão A
        if (button_a_pressed)
        {
            if (current_state == STATE_MENU)
            {
                selected_menu_item = (selected_menu_item + 1) % MENU_COUNT;
            }
            else if (current_state == STATE_MONITOR)
            {
                current_state = STATE_MENU;
            }
            button_a_pressed = false;
        }

        // Tratamento da interrupção do botão B
        if (button_b_pressed)
        {
            if (current_state == STATE_MENU)
            {
                switch (selected_menu_item)
                {
                case MENU_MONITOR:
                    current_state = STATE_MONITOR;
                    break;
                case MENU_HISTORY:
                    current_state = STATE_HISTORY;
                    break;
                case MENU_CONFIG:
                    current_state = STATE_CONFIG;
                    break;
                }
            }
            else if (current_state != STATE_SPLASH)
            {
                current_state = STATE_MENU;
            }
            button_b_pressed = false;
        }

        float value = read_joystick_value();

        // Atualiza estatísticas
        if (value < temp_stats.min_temp)
            temp_stats.min_temp = value;
        if (value > temp_stats.max_temp)
            temp_stats.max_temp = value;
        temp_stats.avg_temp = (temp_stats.avg_temp * (num_samples - 1) + value) / num_samples;

        // Gerenciamento de estados
        switch (current_state)
        {
        case STATE_SPLASH:
            if (to_ms_since_boot(get_absolute_time()) - splash_start_time > SPLASH_DURATION)
            {
                current_state = STATE_MENU;
            }
            draw_splash_screen(&ssd);
            break;

        case STATE_MENU:
            draw_menu_screen(&ssd);
            break;

        case STATE_MONITOR:
            // Armazena no buffer circular
            temp_history[current_index] = value;
            current_index = (current_index + 1) % MAX_SAMPLES;
            if (num_samples < MAX_SAMPLES)
                num_samples++;

            // Limpa o display
            ssd1306_fill(&ssd, false);

            // Corrigindo a chamada da função draw_axis
            draw_axis(&ssd, "s", "C");

            // Corrigindo o array de valores para exibição
            float display_values[MAX_SAMPLES];
            for (int i = 0; i < num_samples; i++)
            {
                int idx = (current_index - num_samples + i + MAX_SAMPLES) % MAX_SAMPLES;
                display_values[i] = temp_history[idx];
            }

            // Corrigindo a chamada da função draw_line_graph
            draw_line_graph(&ssd, &graph, display_values, num_samples);

            // Exibindo a temperatura atual
            char temp_str[10];
            snprintf(temp_str, sizeof(temp_str), "%.1f C", value);

            // Texto "TEMPO MIN" centralizado na parte inferior
            int text_width = strlen("TEMPO MIN") * 6; // Aproximadamente 6 pixels por caractere
            int x_position = (DISPLAY_WIDTH - text_width) / 2;
            ssd1306_draw_string(&ssd, "    TEMPO MIN", 0, 55); // 55 pixels do topo (próximo à parte inferior)

            break;

            // ... outros estados serão implementados posteriormente ...
        }

        ssd1306_send_data(&ssd);
        sleep_ms(50); // Reduzido para melhor resposta do botão
    }

    return 0;
}