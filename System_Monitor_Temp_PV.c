#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/graphics.h"

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

    // Loop principal
    while (true)
    {
        // Lê o valor do joystick ao invés da temperatura
        float value = read_joystick_value();

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

        // Atualiza o display
        ssd1306_send_data(&ssd);

        // Aguarda o próximo intervalo de amostragem
        sleep_ms(SAMPLE_INTERVAL);
    }

    return 0;
}