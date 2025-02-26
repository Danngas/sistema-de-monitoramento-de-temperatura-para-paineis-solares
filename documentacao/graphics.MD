# 📊 Biblioteca de Gráficos para Display OLED SSD1306

## 📑 Sumário
1. [📝 Introdução](#introdução)
2. [🔧 Estruturas](#estruturas)
3. [⚙️ Funções](#funções)
4. [💡 Exemplos](#exemplos)
5. [⚡ Configurações](#configurações)

## 📝 Introdução
A biblioteca Graphics foi desenvolvida para facilitar a criação de gráficos em displays OLED SSD1306. Ela oferece funções para desenhar diferentes tipos de gráficos, incluindo gráficos de linha e de barras, com suporte a eixos, grades e escalamento automático.

## 🔧 Estruturas

### 📍 Point
Estrutura que representa um ponto no display:


c
typedef struct {
uint8_t x; // Coordenada X
uint8_t y; // Coordenada Y
} Point;


### 📈 Graph
Estrutura principal que configura o gráfico:

c
typedef struct {
uint8_t x_offset; // Margem esquerda
uint8_t y_offset; // Margem superior
uint8_t width; // Largura do gráfico
uint8_t height; // Altura do gráfico
float y_min; // Valor mínimo do eixo Y
float y_max; // Valor máximo do eixo Y
uint8_t y_pixels; // Pixels disponíveis no eixo Y
uint8_t x_divisions; // Divisões do eixo X
uint8_t y_divisions; // Divisões do eixo Y
} Graph;


## ⚙️ Funções

### 🎨 Inicialização

`Graph create_graph(float y_min, float y_max)`
- **🎯 Descrição**: Cria uma nova instância de gráfico
- **📥 Parâmetros**: 
  - y_min: valor mínimo do eixo Y
  - y_max: valor máximo do eixo Y
- **📤 Retorno**: Estrutura Graph configurada

### 📐 Desenho de Eixos


`void draw_axis(ssd1306_t *ssd, const char *x_label, const char *y_label)`
- **🎯 Descrição**: Desenha os eixos X e Y com rótulos
- **📥 Parâmetros**:
  - ssd: ponteiro para o display
  - x_label: texto do eixo X
  - y_label: texto do eixo Y

### 📉 Gráfico de Linha

`void draw_line_graph(ssd1306_t *ssd, Graph *graph, float *values, uint8_t num_points)`
- **🎯 Descrição**: Desenha gráfico conectando pontos com linhas
- **📥 Parâmetros**:
  - ssd: ponteiro para o display
  - graph: configuração do gráfico
  - values: array de valores
  - num_points: quantidade de pontos

`void draw_bar_graph(ssd1306_t *ssd, Graph *graph, float *values, uint8_t num_points)`
- **🎯 Descrição**: Desenha gráfico de barras
- **📥 Parâmetros**:
  - ssd: ponteiro para o display
  - graph: configuração do gráfico
  - values: array de valores
  - num_bars: quantidade de barras

### 🛠️ Funções Auxiliares

#### 📏 Escalonamento

`uint8_t scale_x(Graph graph, float x)`
`uint8_t scale_y(Graph graph, float y)`

- **🎯 Descrição**: Convertem valores reais para coordenadas do display

#### 🧹 Limpeza
`void clear_graph_area(ssd1306_t *ssd, Graph *graph)`
- **🎯 Descrição**: Limpa a área do gráfico

## 💡 Exemplos

### 📈 Gráfico de Linha Simples


```c
// Inicialização
Graph graph = create_graph(0.0f, 100.0f);
// Dados
float values[] = {25.0, 30.0, 45.0, 35.0, 50.0};
// Desenho
draw_axis(&ssd, "Tempo", "Temp");
draw_line_graph(&ssd, &graph, values, 5);
```

### 📊 Gráfico de Barras

```c
Graph graph = create_graph(0.0f, 50.0f);
float values[] = {10.0, 20.0, 30.0, 40.0};
draw_bar_graph(&ssd, &graph, values, 4)
```



## ⚡ Configurações

### 🖥️ Constantes do Display


```c
#define DISPLAY_WIDTH 128 // Largura total do display
#define DISPLAY_HEIGHT 64 // Altura total do display
#define MARGIN_LEFT 20 // Margem esquerda
#define MARGIN_BOTTOM 15 // Margem inferior
#define GRAPH_HEIGHT 44 // Altura útil do gráfico
```


### 📌 Recomendações de Uso
1. **🚀 Inicialização**:
   - Sempre inicialize o gráfico com `create_graph()`
   - Configure os limites y_min e y_max adequadamente

2. **🔄 Atualização**:
   - Limpe a área antes de redesenhar
   - Mantenha o número de pontos compatível com a resolução

3. **⚡ Performance**:
   - Evite atualizar mais que o necessário
   - Use clear_graph_area() para atualizações parciais

4. **📐 Layout**:
   - Considere as margens ao planejar o layout
   - Use as divisões de grade para melhor legibilidade

### ⚠️ Limitações
- Resolução máxima: 128x64 pixels
- Valores são automaticamente limitados ao range y_min/y_max
- Suporte apenas para gráficos 2D simples
- Texto limitado aos caracteres suportados pelo display

## 📝 Notas de Versão
- 📦 Versão atual: 1.0
- 💻 Compatível com displays SSD1306
- 📚 Requer biblioteca SSD1306 base