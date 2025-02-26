#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "ssd1306.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define MARGIN_LEFT 20                                    // Margem para o texto "TEMP"
#define MARGIN_BOTTOM 15                                  // Margem para o texto "tempo (m)"
#define GRAPH_HEIGHT (DISPLAY_HEIGHT - MARGIN_BOTTOM - 5) // Altura útil do gráfico

// Estrutura para representar um ponto
typedef struct
{
    uint8_t x;
    uint8_t y;
} Point;

// Estrutura para representar um gráfico
typedef struct
{
    uint8_t x_offset;
    uint8_t y_offset;
    uint8_t width;
    uint8_t height;
    float y_min;         // Valor mínimo real (ex: 0°C)
    float y_max;         // Valor máximo real (ex: 100°C)
    uint8_t y_pixels;    // Quantidade de pixels no eixo Y
    uint8_t x_divisions; // Novo campo
    uint8_t y_divisions; // Novo campo
} Graph;

// Função para criar e configurar o gráfico
Graph create_graph(float y_min, float y_max);

// Função para converter valor real para posição em pixels
uint8_t scale_value_to_pixel(Graph *graph, float value);

// Funções para desenhar gráficos
void draw_axis(ssd1306_t *ssd, const char *x_label, const char *y_label);
void draw_point(ssd1306_t *ssd, Graph *graph, float x, float y);
void draw_line_graph(ssd1306_t *ssd, Graph *graph, float *values, uint8_t num_points);
void draw_bar_graph(ssd1306_t *ssd, Graph *graph, float *values, uint8_t num_bars);
void clear_graph_area(ssd1306_t *ssd, Graph *graph);

// Funções auxiliares
uint8_t scale_x(Graph *graph, float x);
uint8_t scale_y(Graph *graph, float y);

#endif // GRAPHICS_H