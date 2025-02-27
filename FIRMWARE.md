# Especificação do Firmware - Sistema de Monitoramento de Temperatura

## 📊 Blocos Funcionais

```
┌─────────────────────────────────────────────────────────┐
│                    Interface de Usuário                  │
│  (Menu, Display, Interação com Botões, Alertas Visuais) │
├─────────────────┬───────────────────┬──────────────────┤
│   Gerenciador   │    Gerenciador    │   Gerenciador    │
│   de Display    │    de Estados     │   de Alertas     │
├─────────────────┴───────────────────┴──────────────────┤
│              Processamento de Dados                     │
│     (Amostragem, Estatísticas, Histórico, Gráficos)    │
├─────────────────┬───────────────────┬──────────────────┤
│    Driver ADC   │    Driver I2C     │   Driver PWM     │
│  (Temperatura)  │    (Display)      │   (LED RGB)      │
└─────────────────┴───────────────────┴──────────────────┘
```

## 🔍 Descrição das Funcionalidades

### 1. Interface de Usuário

- Gerenciamento do menu principal
- Tratamento de entrada dos botões
- Renderização de telas
- Sistema de navegação

### 2. Gerenciador de Display

- Controle do display OLED
- Renderização de gráficos
- Desenho de caracteres
- Buffer de display

### 3. Gerenciador de Estados

- Controle de estados do sistema
- Transições entre telas
- Gerenciamento de modos de operação

### 4. Gerenciador de Alertas

- Monitoramento de limites
- Controle do LED RGB
- Geração de alertas visuais

### 5. Processamento de Dados

- Aquisição de temperatura
- Cálculo de estatísticas
- Manutenção do histórico
- Geração de gráficos

## 📝 Definição das Variáveis Principais

```c
// Estados do Sistema
typedef enum {
    STATE_SPLASH,
    STATE_MENU,
    STATE_MONITOR,
    STATE_HISTORY,
    STATE_CONFIG,
    STATE_STATS,
    STATE_ALERTS
} SystemState;

// Estrutura de Temperatura
typedef struct {
    float temp_min;
    float temp_max;
    float current_min;
    float current_max;
} TempScale;

// Configuração de Alertas
typedef struct {
    float temp_normal_max;
    float temp_attention_max;
    float temp_urgent_max;
    AlertType current_alert;
} AlertConfig;

// Buffer Circular para Histórico
struct {
    float temperatures[HISTORY_SIZE];
    int count;
    int newest_index;
    int scroll_position;
} temperature_history;
```

## 🔄 Fluxograma do Sistema

```
┌──────────────┐
│  Inicialização│
└───────┬──────┘
        ▼
┌──────────────┐
│ Tela Inicial │
└───────┬──────┘
        ▼
┌──────────────┐
│ Menu Principal│◄─────────────┐
└───────┬──────┘              │
        ▼                     │
┌──────────────┐              │
│ Seleção Modo │              │
└───────┬──────┘              │
        ▼                     │
┌──────────────┐              │
│ Execução Modo│──────────────┘
└──────────────┘
```

## 🚀 Processo de Inicialização

1. Configuração do Sistema

```c
stdio_init_all();
adc_init();
i2c_init(I2C_PORT, 400 * 1000);
```

2. Configuração de GPIOs

```c
gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
gpio_pull_up(I2C_SDA);
gpio_pull_up(I2C_SCL);
```

3. Inicialização do Display

```c
ssd1306_init(&ssd, 128, 64, false, DISPLAY_ADDR, I2C_PORT);
ssd1306_config(&ssd);
```

4. Configuração de Interrupções

```c
gpio_set_irq_enabled_with_callback(BUTTON_A_PIN,
    GPIO_IRQ_EDGE_FALL,
    true,
    &gpio_callback);
```

## ⚙️ Configurações dos Registros

### ADC

```c
// Configuração ADC
adc_gpio_init(26);
adc_select_input(0);
```

### PWM

```c
// Configuração PWM para LED RGB
pwm_set_clkdiv(slice_num, 1250);
pwm_set_wrap(slice_num, 2000);
pwm_set_enabled(slice_num, true);
```

### I2C

```c
// Configuração I2C
i2c_init(i2c1, 400000);
gpio_set_function(14, GPIO_FUNC_I2C);
gpio_set_function(15, GPIO_FUNC_I2C);
```

## 📊 Estrutura e Formato dos Dados

### 1. Buffer de Temperatura

- Tipo: Array Circular
- Tamanho: 128 amostras
- Formato: Float (°C)
- Resolução: 12 bits

### 2. Dados de Display

- Buffer: 1024 bytes (128x64 pixels)
- Formato: Bitmap monocromático
- Orientação: Horizontal

### 3. Configurações

- Limites de temperatura: Float
- Estados: Enum
- Temporizações: uint32_t

## 🔌 Protocolo de Comunicação

### I2C (Display OLED)

- Velocidade: 400kHz
- Endereço: 0x3C
- Modo: Master

#### Formato dos Comandos

```c
typedef enum {
    SET_CONTRAST = 0x81,
    SET_ENTIRE_ON = 0xA4,
    SET_NORM_INV = 0xA6,
    SET_DISP = 0xAE,
    SET_MEM_ADDR = 0x20,
    SET_COL_ADDR = 0x21,
    SET_PAGE_ADDR = 0x22
} ssd1306_command_t;
```

## 📦 Formato do Pacote de Dados

### Pacote de Display

```
Byte 0: Control Byte (0x00 para comando, 0x40 para dados)
Byte 1: Comando/Dado
[Bytes 2-n]: Dados adicionais (se necessário)
```

### Buffer de Temperatura

```
struct temp_record {
    uint32_t timestamp;  // 4 bytes
    float temperature;   // 4 bytes
} __attribute__((packed));
```

## ⏱️ Temporização

- Amostragem de temperatura: 1000ms
- Atualização do display: 50ms
- Debounce dos botões: 200ms
- Duração da tela inicial: 3000ms

## 🔄 Ciclo de Execução Principal

```c
while (true) {
    // Tratamento de botões
    handle_button_events();

    // Atualização de estado
    update_system_state();

    // Processamento de temperatura
    process_temperature();

    // Atualização do display
    update_display();

    // Delay para controle de CPU
    sleep_ms(50);
}
```
