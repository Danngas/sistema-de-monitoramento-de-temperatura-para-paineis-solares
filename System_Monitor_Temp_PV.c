#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
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
    STATE_CONFIG,
    STATE_STATS,
    STATE_ALERTS // Novo estado
} SystemState;

// Opções do menu principal
typedef enum
{
    MENU_MONITOR,
    MENU_HISTORY,
    MENU_CONFIG,
    MENU_STATS,
    MENU_ALERTS, // Nova opção
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

// Adicione estas definições no início do arquivo, após os outros #defines
#define HISTORY_SIZE 128           // Tamanho baseado na largura do display
#define DISPLAY_LINES 4            // Número de linhas no modo histórico
#define TEMP_READ_INTERVAL_MS 1000 // Intervalo de 1 segundo

// Definições dos canais ADC
#define ADC_CHANNEL_TEMP 0   // Canal 0 para o sensor de temperatura
#define ADC_CHANNEL_SCROLL 1 // Canal 1 para o controle de rolagem

// Estrutura para armazenar o histórico
struct
{
    float temperatures[HISTORY_SIZE];
    int count;
    int newest_index;
    int scroll_position;
    float min_temp;
    float max_temp;
} temperature_history = {
    .temperatures = {0},
    .count = 0,
    .newest_index = 0,
    .scroll_position = 0,
    .min_temp = 100.0f,
    .max_temp = 0.0f};

// Variável para controle de atualização do display
volatile bool new_temperature_available = false;

// Definições para o condicionamento do gráfico
#define GRAPH_Y_MIN 0  // Valor mínimo do eixo Y (pixels)
#define GRAPH_Y_MAX 52 // Valor máximo do eixo Y (pixels)
#define TEMP_MIN_SENSOR 0
#define TEMP_MAX_SENSOR 100

uint16_t adc_value_x;
char TEMP_REAL[5];
// Limites de temperatura ajustáveis
struct
{
    float temp_min;    // Temperatura mínima para escala
    float temp_max;    // Temperatura máxima para escala
    float current_min; // Menor temperatura registrada
    float current_max; // Maior temperatura registrada
} temp_scale = {
    .temp_min = 20.0f, // Limite inferior inicial
    .temp_max = 40.0f, // Limite superior inicial
    .current_min = 100.0f,
    .current_max = 0.0f};

#define ADC_MAX_VALUE 4095 // Valor máximo do ADC (12 bits)
#define ADC_MID_VALUE 2047 // Valor médio do ADC
#define GRAPH_Y_MID 26     // Ponto médio do gráfico

// Definir tipos de alerta
typedef enum
{
    ALERT_NORMAL,
    ALERT_ATTENTION,
    ALERT_URGENT
} AlertType;

// Estrutura para configuração de alertas
typedef struct
{
    float temp_normal_max;    // Limite superior para operação normal
    float temp_attention_max; // Limite superior para atenção
    float temp_urgent_max;    // Limite superior para urgente
    AlertType current_alert;  // Estado atual do alerta
} AlertConfig;

// Inicialização da configuração de alertas
AlertConfig alert_config = {
    .temp_normal_max = 45.0f,    // Operação normal até 45°C
    .temp_attention_max = 65.0f, // Atenção até 65°C
    .temp_urgent_max = 65.0f,    // Urgente acima de 65°C
    .current_alert = ALERT_NORMAL};

// Definições dos pinos do LED RGB

#define LED_R 13 // GPIO do LED vermelho
#define LED_G 11 // GPIO do LED verde
#define LED_B 12 // GPIO do LED azul

// Definições para PWM
#define PWM_FREQ 1000
#define PWM_DUTY 50
#define PWM_WRAP 65535 // Wrap value for 16-bit PWM

// Controle de intensidade do LED (0.0 a 1.0)
// Ajuste este valor para encontrar a intensidade ideal
#define LED_INTENSITY 0.1f // 10% de intensidade - ajuste conforme necessário

// Estrutura para controle do LED RGB
typedef struct
{
    uint slice_red;
    uint slice_green;
    uint slice_blue;
} RGB_LED_Control;

RGB_LED_Control rgb_led;

// Função para inicializar o LED RGB

// Função para calcular o nível PWM baseado na intensidade
uint16_t get_pwm_level(void)
{
    // Garante que a intensidade está entre 0 e 1
    float intensity = (LED_INTENSITY < 0.0f) ? 0.0f : (LED_INTENSITY > 1.0f) ? 1.0f
                                                                             : LED_INTENSITY;

    // Como os LEDs são ativos em nível baixo:
    // - PWM_WRAP = LED desligado
    // - 0 = LED com brilho máximo
    return (uint16_t)(PWM_WRAP * intensity);
}

void pwm_set_duty(uint gpio, uint16_t value)
{
    pwm_set_gpio_level(gpio, value);
}

// Função para atualizar o LED baseado no status
void update_led_status(AlertType alert_status)
{
    uint16_t pwm_level = get_pwm_level();

    switch (alert_status)
    {
    case ALERT_NORMAL:
        // Verde ligado, outros desligados
        pwm_set_duty(LED_R, 0);   // Desligado
        pwm_set_duty(LED_G, 500); // Verde ligado
        pwm_set_duty(LED_B, 0);   // Desligado
        break;

    case ALERT_ATTENTION:
        // Amarelo (vermelho + verde)
        pwm_set_duty(LED_R, 500); // Vermelho ligado
        pwm_set_duty(LED_G, 500); // Verde ligado
        pwm_set_duty(LED_B, 0);   // Desligado
        break;

    case ALERT_URGENT:
        // Vermelho piscante
        static bool led_state = false;
        led_state = !led_state;
        pwm_set_duty(LED_R, 500); // Vermelho piscando
        pwm_set_duty(LED_G, 0);   // Desligado
        pwm_set_duty(LED_B, 0);   // Desligado
        break;
    }
}

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

float map_value(float x, float x1, float x2, float y1, float y2)
{
    return y1 + ((x - x1) * (y2 - y1)) / (x2 - x1);
}

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

#define MENU_ITEMS_VISIBLE 4 // Número máximo de itens visíveis no display

void draw_menu_screen(ssd1306_t *ssd)
{
    ssd1306_fill(ssd, false);
    const char *menu_items[] = {"Monitor", "Historico", "Config", "Stats", "Alertas"};

    // Calcula o primeiro item visível baseado na seleção atual
    int first_visible = 0;
    if (selected_menu_item >= MENU_ITEMS_VISIBLE)
    {
        first_visible = selected_menu_item - MENU_ITEMS_VISIBLE + 1;
    }

    // Desenha apenas os itens visíveis
    for (int i = 0; i < MENU_ITEMS_VISIBLE && (i + first_visible) < MENU_COUNT; i++)
    {
        char line[32];
        int current_item = i + first_visible;

        // Adiciona indicadores de scroll se necessário
        if (first_visible > 0 && i == 0)
        {
            ssd1306_draw_string(ssd, "^", 60, 0); // Seta para cima
        }
        if ((first_visible + MENU_ITEMS_VISIBLE) < MENU_COUNT && i == (MENU_ITEMS_VISIBLE - 1))
        {
            ssd1306_draw_string(ssd, "v", 60, 45); // Seta para baixo
        }

        snprintf(line, sizeof(line), "%s%s",
                 (current_item == selected_menu_item) ? "> " : "  ",
                 menu_items[current_item]);
        ssd1306_draw_string(ssd, line, 10, i * 15);
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

// Função para adicionar nova temperatura ao histórico
void add_temperature_to_history(float temp)
{
    temperature_history.temperatures[temperature_history.newest_index] = temp;
    temperature_history.newest_index = (temperature_history.newest_index + 1) % HISTORY_SIZE;
    if (temperature_history.count < HISTORY_SIZE)
    {
        temperature_history.count++;
    }
}

// Função modificada para debug
void draw_history_screen(ssd1306_t *ssd)
{
    ssd1306_fill(ssd, false);
    ssd1306_draw_string(ssd, "Historico Temp.", 5, 0);
    ssd1306_line(ssd, 0, 10, 128, 10, true);

    // Debug: mostra quantidade de temperaturas armazenadas
    char debug_str[16];
    snprintf(debug_str, sizeof(debug_str), "Total: %d", temperature_history.count);
    ssd1306_draw_string(ssd, debug_str, 5, 15);

    // Lê o valor do joystick para rolagem
    adc_select_input(ADC_CHANNEL_SCROLL);
    uint16_t scroll_raw = adc_read();

    // Ajusta a posição de rolagem baseado no joystick
    if (scroll_raw > 3000)
    {
        if (temperature_history.scroll_position > 0)
        {
            temperature_history.scroll_position--;
        }
    }
    else if (scroll_raw < 1000)
    {
        if (temperature_history.scroll_position < (temperature_history.count - DISPLAY_LINES))
        {
            temperature_history.scroll_position++;
        }
    }

    // Desenha as temperaturas visíveis
    for (int i = 0; i < DISPLAY_LINES && i < temperature_history.count; i++)
    {
        char temp_str[16];
        int display_index = i + temperature_history.scroll_position;

        // Calcula o índice real no array circular
        int actual_index = (temperature_history.newest_index - 1 - display_index + HISTORY_SIZE) % HISTORY_SIZE;

        snprintf(temp_str, sizeof(temp_str), "%2d: %.1f C",
                 display_index + 1,
                 map_value(temperature_history.temperatures[actual_index], 0, 4095, TEMP_MIN_SENSOR, TEMP_MAX_SENSOR));

        ssd1306_draw_string(ssd, temp_str, 5, 27 + (i * 10));
    }

    // Indicadores de rolagem
    if (temperature_history.count > DISPLAY_LINES)
    {
        if (temperature_history.scroll_position > 0)
        {
            ssd1306_draw_string(ssd, "^", 120, 15);
        }
        if (temperature_history.scroll_position < (temperature_history.count - DISPLAY_LINES))
        {
            ssd1306_draw_string(ssd, "v", 120, 50);
        }
    }
}

// Função para converter valor do ADC em posição Y no gráfico
int adc_to_y_position(uint16_t adc_value)
{
    // Conversão linear: y = (52 * adc_value) / 4095
    return (GRAPH_Y_MAX * adc_value) / ADC_MAX_VALUE;
}

// Função atualizada para desenhar o gráfico
void draw_graph_screen(ssd1306_t *ssd)
{
    ssd1306_fill(ssd, false);

    // Desenha eixos
    ssd1306_line(ssd, 10, 0, 10, GRAPH_Y_MAX, true);            // Eixo Y
    ssd1306_line(ssd, 10, GRAPH_Y_MAX, 127, GRAPH_Y_MAX, true); // Eixo X

    // Desenha marcações de escala no eixo Y
    char value_str[8];
    // Marca valor máximo (4095)
    // snprintf(value_str, sizeof(value_str), "4095");
    // ssd1306_draw_string(ssd, value_str, 0, 0);

    // Marca valor médio (2047)
    // snprintf(value_str, sizeof(value_str), "2047");
    // ssd1306_draw_string(ssd, value_str, 0, GRAPH_Y_MID - 4);

    // Marca valor mínimo (0)
    // snprintf(value_str, sizeof(value_str), "0");
    // ssd1306_draw_string(ssd, value_str, 0, GRAPH_Y_MAX - 8);

    // Desenha linha de referência no ponto médio
    // ssd1306_line(ssd, 10, GRAPH_Y_MID, 15, GRAPH_Y_MID, true);

    // Plota os pontos usando valores diretos do ADC
    for (int i = 0; i < temperature_history.count - 1 && i < 116; i++)
    {
        int idx = (temperature_history.newest_index - 1 - i + HISTORY_SIZE) % HISTORY_SIZE;
        int next_idx = (temperature_history.newest_index - 2 - i + HISTORY_SIZE) % HISTORY_SIZE;

        uint16_t adc_value = temperature_history.temperatures[idx];
        uint16_t next_adc_value = temperature_history.temperatures[next_idx];

        // Converte valores ADC para posições Y
        int y1 = GRAPH_Y_MAX - adc_to_y_position(adc_value);
        int y2 = GRAPH_Y_MAX - adc_to_y_position(next_adc_value);

        ssd1306_line(ssd, 127 - i, y1, 126 - i, y2, true);
    }

    // Mostra valor ADC atual
    uint16_t current_adc = temperature_history.temperatures[(temperature_history.newest_index - 1 + HISTORY_SIZE) % HISTORY_SIZE];
    // snprintf(value_str, sizeof(value_str), "ADC:%d", current_adc);

    // conversao para valores de temperatura REAL
    ssd1306_draw_string(ssd, value_str, 30, 0);
    // Buffer para armazenar a string
    sprintf(TEMP_REAL, "%.2f", map_value(current_adc, 0, 4095, TEMP_MIN_SENSOR, TEMP_MAX_SENSOR)); // Converte o inteiro em string

    ssd1306_draw_string(ssd, TEMP_REAL, 20, 0); // Desenha uma string
}

// Função para verificar e atualizar o estado do alerta
void update_alert_status(float current_temp)
{
    if (current_temp > alert_config.temp_urgent_max)
    {
        alert_config.current_alert = ALERT_URGENT;
    }
    else if (current_temp > alert_config.temp_normal_max)
    {
        alert_config.current_alert = ALERT_ATTENTION;
    }
    else
    {
        alert_config.current_alert = ALERT_NORMAL;
    }
}

// Função para desenhar a tela de alertas
void draw_alerts_screen(ssd1306_t *ssd)
{
    ssd1306_fill(ssd, false);

    // Título
    ssd1306_draw_string(ssd, "Status System", 10, 0);
    ssd1306_line(ssd, 0, 10, 128, 10, true);

    // Temperatura atual
    char temp_str[32];
    float current_temp = map_value(adc_read(), 0, 4095, TEMP_MIN_SENSOR, TEMP_MAX_SENSOR);
    snprintf(temp_str, sizeof(temp_str), "Temp: %.1f C", current_temp);
    ssd1306_draw_string(ssd, temp_str, 5, 15);

    // Status do alerta
    const char *alert_str;
    switch (alert_config.current_alert)
    {
    case ALERT_URGENT:
        alert_str = "URGENTE!";
        // Desenha uma borda piscante para alertas urgentes

        ssd1306_rect(ssd, 40, 20, 100, 20, true, false); // último parâmetro: fill = false

        break;
    case ALERT_ATTENTION:
        alert_str = "Atencao!";
        // Desenha uma borda simples para atenção
        ssd1306_rect(ssd, 40, 20, 100, 20, true, false); // último parâmetro: fill = false
        break;
    default:
        alert_str = "Normal";
        pwm_set_duty(LED_R, 0);   // LED vermelho com eixo X
        pwm_set_duty(LED_G, 500); // LED verde com eixo Y
        pwm_set_duty(LED_B, 0);   // LED azul com eixo Z
        break;
    }

    ssd1306_draw_string(ssd, alert_str, 45, 45);
}

// Atualizar a função de configuração
void draw_config_screen(ssd1306_t *ssd)
{
    static int selected_option = 0;
    static bool editing = false;

    ssd1306_fill(ssd, false);
    ssd1306_draw_string(ssd, "Configuracao", 20, 0);
    ssd1306_line(ssd, 0, 10, 128, 10, true);

    char temp_str[32];

    // Opções de configuração
    snprintf(temp_str, sizeof(temp_str), "%sNormal: %.1f",
             (selected_option == 0 && editing) ? ">" : " ",
             alert_config.temp_normal_max);
    ssd1306_draw_string(ssd, temp_str, 5, 20);

    snprintf(temp_str, sizeof(temp_str), "%sAtencao: %.1f",
             (selected_option == 1 && editing) ? ">" : " ",
             alert_config.temp_attention_max);
    ssd1306_draw_string(ssd, temp_str, 5, 35);

    snprintf(temp_str, sizeof(temp_str), "%sUrgente: %.1f",
             (selected_option == 2 && editing) ? ">" : " ",
             alert_config.temp_urgent_max);
    ssd1306_draw_string(ssd, temp_str, 5, 50);

    // Instruções
    if (editing)
    {
        ssd1306_draw_string(ssd, "A:+/B:-", 90, 55);
    }
    else
    {
        ssd1306_draw_string(ssd, "A:Sel/B:Edit", 70, 55);
    }
}

// Atualizar o callback do timer
bool temperature_alarm_callback(struct repeating_timer *t)
{
    adc_select_input(ADC_CHANNEL_TEMP);
    uint16_t adc_value = adc_read();
    float current_temp = map_value(adc_value, 0, 4095, TEMP_MIN_SENSOR, TEMP_MAX_SENSOR);

    // Atualiza o estado do alerta
    update_alert_status(current_temp);

    // Atualiza o LED RGB baseado no estado atual do alerta
    update_led_status(alert_config.current_alert);

    // Atualiza máximos e mínimos
    if (current_temp > temp_scale.current_max)
    {
        temp_scale.current_max = current_temp;
    }
    if (current_temp < temp_scale.current_min)
    {
        temp_scale.current_min = current_temp;
    }

    // Armazena o valor bruto do ADC
    temperature_history.temperatures[temperature_history.newest_index] = adc_value;
    temperature_history.newest_index = (temperature_history.newest_index + 1) % HISTORY_SIZE;
    if (temperature_history.count < HISTORY_SIZE)
    {
        temperature_history.count++;
    }

    new_temperature_available = true;
    return true;
}

// Adicionar função para desenhar a tela de estatísticas
void draw_stats_screen(ssd1306_t *ssd)
{
    ssd1306_fill(ssd, false);

    // Título
    ssd1306_draw_string(ssd, "Estatisticas", 20, 0);
    ssd1306_line(ssd, 0, 10, 128, 10, true);

    // Temperatura atual
    char temp_str[32];
    float current_temp = map_value(adc_read(), 0, 4095, TEMP_MIN_SENSOR, TEMP_MAX_SENSOR);
    snprintf(temp_str, sizeof(temp_str), "Atual: %.1f C", current_temp);
    ssd1306_draw_string(ssd, temp_str, 5, 20);

    // Temperatura máxima
    snprintf(temp_str, sizeof(temp_str), "Max: %.1f C", temp_scale.current_max);
    ssd1306_draw_string(ssd, temp_str, 5, 35);

    // Temperatura mínima
    snprintf(temp_str, sizeof(temp_str), "Min: %.1f C", temp_scale.current_min);
    ssd1306_draw_string(ssd, temp_str, 5, 50);
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

    // Configura os LEDs para modo PWM
    gpio_set_function(LED_R, GPIO_FUNC_PWM);
    gpio_set_function(LED_G, GPIO_FUNC_PWM);
    gpio_set_function(LED_B, GPIO_FUNC_PWM);

    // Configuração do PWM para os LEDs
    uint slice_num_r = pwm_gpio_to_slice_num(LED_R);
    uint slice_num_g = pwm_gpio_to_slice_num(LED_G);
    uint slice_num_b = pwm_gpio_to_slice_num(LED_B);

    // Configura PWM 50Hz para todos os LEDs
    pwm_set_clkdiv(slice_num_r, 1250);
    pwm_set_clkdiv(slice_num_g, 1250);
    pwm_set_clkdiv(slice_num_b, 1250);

    pwm_set_wrap(slice_num_r, 2000);
    pwm_set_wrap(slice_num_g, 2000);
    pwm_set_wrap(slice_num_b, 2000);

    // Habilita o PWM para todos os LEDs
    pwm_set_enabled(slice_num_r, true);
    pwm_set_enabled(slice_num_g, true);
    pwm_set_enabled(slice_num_b, true);

    // Configuração das interrupções para ambos os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_callback);

    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN,
                                       GPIO_IRQ_EDGE_FALL,
                                       true,
                                       &gpio_callback);

    // Configura o alarme para leitura de temperatura
    struct repeating_timer timer;
    add_repeating_timer_ms(TEMP_READ_INTERVAL_MS, temperature_alarm_callback, NULL, &timer);

    // Inicializa os valores de máximo e mínimo
    temp_scale.current_max = -100.0f; // Começa com um valor baixo
    temp_scale.current_min = 200.0f;  // Começa com um valor alto

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
                case MENU_STATS:
                    current_state = STATE_STATS;
                    break;
                case MENU_ALERTS:
                    current_state = STATE_ALERTS;
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
            draw_graph_screen(&ssd);
            break;

        case STATE_HISTORY:
            draw_history_screen(&ssd);
            break;

        case STATE_CONFIG:
            draw_config_screen(&ssd);
            break;

        case STATE_STATS:
            draw_stats_screen(&ssd);
            break;

        case STATE_ALERTS:
            draw_alerts_screen(&ssd);
            break;
        }

        if (new_temperature_available)
        {
            new_temperature_available = false;
            // Força atualização do display quando há nova temperatura
            ssd1306_send_data(&ssd);
        }

        sleep_ms(50); // Pequeno delay para não sobrecarregar o processador
    }

    return 0;
}