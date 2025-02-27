# Especificação do Hardware - Sistema de Monitoramento de Temperatura para Painéis Solares

## 📊 Diagrama em Blocos

```
                                    ┌─────────────┐
                                    │   LED RGB   │
                                    └─────┬───────┘
                                          │
┌─────────────┐    ┌─────────────┐    ┌──┴──────────┐    ┌─────────────┐
│   Sensor    │    │   Botões    │    │             │    │   Display   │
│ Temperatura ├────┤    A/B      ├────┤ Raspberry   ├────┤   OLED     │
│   ADC       │    │             │    │ Pi Pico     │    │  SSD1306   │
└─────────────┘    └─────────────┘    └─────────────┘    └─────────────┘
```

## 🔍 Função de Cada Bloco

### 1. Raspberry Pi Pico (Unidade Central)

- Processamento principal
- Controle de periféricos
- Gerenciamento de estados
- Processamento de interrupções
- Conversão ADC
- Comunicação I2C

### 2. Sensor de Temperatura (ADC)

- Medição de temperatura analógica
- Conversão temperatura/tensão
- Range: 0-100°C
- Resolução: 12 bits (0-4095)

### 3. Display OLED SSD1306

- Interface visual
- Resolução: 128x64 pixels
- Comunicação I2C
- Exibição de gráficos e texto

### 4. LED RGB

- Indicação visual de status
- 3 canais PWM independentes
- Cores:
  - Verde: Operação normal
  - Amarelo: Atenção
  - Vermelho: Alerta

### 5. Botões de Controle

- Interface de usuário
- Debounce por software
- Pull-up interno
- Interrupção por borda de descida

## ⚙️ Configuração dos Blocos

### 1. Raspberry Pi Pico

```c
// Clock Principal
clock_configure(clk_sys,
    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
    125 * MHZ,
    125 * MHZ);

// ADC
adc_init();
adc_gpio_init(26);
adc_select_input(0);

// I2C
i2c_init(i2c1, 400 * 1000);  // 400kHz
gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
```

### 2. Display OLED

```c
// Inicialização SSD1306
ssd1306_init(&ssd, 128, 64, false, DISPLAY_ADDR, I2C_PORT);
ssd1306_config(&ssd);
```

### 3. LED RGB (PWM)

```c
// Configuração PWM
pwm_set_clkdiv(slice_num, 1250);  // 50Hz
pwm_set_wrap(slice_num, 2000);    // Resolução
pwm_set_enabled(slice_num, true);
```

### 4. Botões

```c
// Configuração GPIO com interrupção
gpio_init(BUTTON_A_PIN);
gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
gpio_pull_up(BUTTON_A_PIN);
gpio_set_irq_enabled_with_callback(BUTTON_A_PIN,
    GPIO_IRQ_EDGE_FALL,
    true,
    &gpio_callback);
```

gpio_init(BUTTON_B_PIN);
gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
gpio_pull_up(BUTTON_B_PIN);
gpio_set_irq_enabled_with_callback(BUTTON_B_PIN,
    GPIO_IRQ_EDGE_FALL,
    true,
    &gpio_callback);

## 📍 Pinagem Utilizada

### GPIO Configuração

```
GPIO 26 (ADC0) - Sensor de Temperatura
GPIO 14 (I2C1 SDA) - Display OLED
GPIO 15 (I2C1 SCL) - Display OLED
GPIO 5  - Botão A
GPIO 6  - Botão B
GPIO 13 - LED Vermelho (PWM)
GPIO 11 - LED Verde (PWM)
GPIO 12 - LED Azul (PWM)
```

## 🔌 Circuito Completo

```
                     Vdd (3.3V)
                        │
                        ├─── 10kΩ ─┬─ GPIO5 (Botão A)
                        │          SW1
                        │          │
                        ├─── 10kΩ ─┬─ GPIO6 (Botão B)
                        │          SW2
                        │          │
                        └─── GND   GND

     ┌────────────┐
     │  SSD1306   │
     │            │
SDA ─┤            │
SCL ─┤            │
     │            │
     └────────────┘

     ┌────────────┐
     │ Sensor     │
OUT ─┤ Temp.      │
     │            │
     └────────────┘

        LED RGB
     ┌────────┐
R ───┤        │
G ───┤  RGB   │
B ───┤        │
     └────────┘
```

## 📝 Registradores e Comandos Principais

### ADC

```c
#define ADC_CS                  0x4004c000
#define ADC_RESULT             0x4004c004
#define ADC_FCS                0x4004c008
#define ADC_FIFO              0x4004c00c
#define ADC_DIV               0x4004c010
#define ADC_INTR              0x4004c014
#define ADC_INTE              0x4004c018
#define ADC_INTF              0x4004c01c
#define ADC_INTS              0x4004c020
```

### SSD1306

```c
#define SET_CONTRAST           0x81
#define SET_ENTIRE_ON         0xA4
#define SET_NORM_INV          0xA6
#define SET_DISP              0xAE
#define SET_MEM_ADDR          0x20
#define SET_COL_ADDR          0x21
#define SET_PAGE_ADDR         0x22
#define SET_DISP_START_LINE   0x40
#define SET_SEG_REMAP         0xA0
#define SET_MUX_RATIO         0xA8
```

## ⚡ Especificações Elétricas

- Tensão de Operação: 3.3V
- Corrente Máxima por GPIO: 12mA
- Frequência I2C: 400kHz
- Frequência PWM: 50Hz
- Resolução ADC: 12 bits
- Pull-up Botões: 10kΩ

## 🛠️ Considerações de Montagem

1. Utilize resistores pull-up de 10kΩ para os botões
2. LED RGB deve usar resistores limitadores apropriados
3. Mantenha cabos I2C curtos para evitar interferência
4. Adicione capacitor de desacoplamento próximo ao sensor
5. Observe a polaridade correta do display OLED
