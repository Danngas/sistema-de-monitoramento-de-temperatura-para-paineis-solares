#include "graphics.h"
#include "ssd1306.h"

void draw_axis(ssd1306_t *ssd, const char *x_label, const char *y_label)
{
    // Desenha eixo Y
    ssd1306_line(ssd, MARGIN_LEFT, 5, MARGIN_LEFT, DISPLAY_HEIGHT - MARGIN_BOTTOM, true);
    // Seta do eixo Y
    ssd1306_line(ssd, MARGIN_LEFT - 2, 5, MARGIN_LEFT, 0, true);
    ssd1306_line(ssd, MARGIN_LEFT + 2, 5, MARGIN_LEFT, 0, true);

    // Desenha eixo X
    ssd1306_line(ssd, MARGIN_LEFT, DISPLAY_HEIGHT - MARGIN_BOTTOM,
                 DISPLAY_WIDTH - 5, DISPLAY_HEIGHT - MARGIN_BOTTOM, true);
    // Seta do eixo X
    ssd1306_line(ssd, DISPLAY_WIDTH - 5, DISPLAY_HEIGHT - MARGIN_BOTTOM - 2,
                 DISPLAY_WIDTH, DISPLAY_HEIGHT - MARGIN_BOTTOM, true);
    ssd1306_line(ssd, DISPLAY_WIDTH - 5, DISPLAY_HEIGHT - MARGIN_BOTTOM + 2,
                 DISPLAY_WIDTH, DISPLAY_HEIGHT - MARGIN_BOTTOM, true);

    // Escreve "TEMP" verticalmente
    ssd1306_draw_string(ssd, "G", 2,0);
    ssd1306_draw_string(ssd, "R", 2,10);
    ssd1306_draw_string(ssd, "A", 2,20);
    ssd1306_draw_string(ssd, "U", 2,30);
    ssd1306_draw_string(ssd, "C", 2,40);
    
}

void draw_point(ssd1306_t *ssd, Graph *graph, float x, float y)
{
    uint8_t screen_x = scale_x(graph, x);
    uint8_t screen_y = scale_y(graph, y);

    // Desenha um ponto 2x2 pixels
    for (int8_t dx = -1; dx <= 1; dx++)
    {
        for (int8_t dy = -1; dy <= 1; dy++)
        {
            ssd1306_pixel(ssd, screen_x + dx, screen_y + dy, true);
        }
    }
}

void draw_line_graph(ssd1306_t *ssd, Graph *graph, float *values, uint8_t num_points)
{
    if (num_points < 2)
        return;

    float x_step = (float)graph->width / (num_points - 1);

    for (uint8_t i = 0; i < num_points - 1; i++)
    {
        uint8_t x1 = graph->x_offset + (i * x_step);
        uint8_t y1 = scale_y(graph, values[i]);
        uint8_t x2 = graph->x_offset + ((i + 1) * x_step);
        uint8_t y2 = scale_y(graph, values[i + 1]);

        ssd1306_line(ssd, x1, y1, x2, y2, true);
    }
}

void draw_bar_graph(ssd1306_t *ssd, Graph *graph, float *values, uint8_t num_bars)
{
    float bar_width = (float)graph->width / num_bars;

    for (uint8_t i = 0; i < num_bars; i++)
    {
        uint8_t x = graph->x_offset + (i * bar_width);
        uint8_t y = scale_y(graph, values[i]);
        uint8_t height = graph->y_offset + graph->height - y;

        ssd1306_rect(ssd, y, x, bar_width - 1, height, true, true);
    }
}

void draw_grid(ssd1306_t *ssd, Graph *graph)
{
    // Desenha linhas horizontais da grade
    for (uint8_t i = 1; i < graph->y_divisions; i++)
    {
        uint8_t y = graph->y_offset + graph->height - (i * graph->height / graph->y_divisions);
        ssd1306_hline(ssd,
                      graph->x_offset,
                      graph->x_offset + graph->width,
                      y,
                      true);
    }

    // Desenha linhas verticais da grade
    for (uint8_t i = 1; i < graph->x_divisions; i++)
    {
        uint8_t x = graph->x_offset + (i * graph->width / graph->x_divisions);
        ssd1306_vline(ssd,
                      x,
                      graph->y_offset,
                      graph->y_offset + graph->height,
                      true);
    }
}

void clear_graph_area(ssd1306_t *ssd, Graph *graph)
{
    for (uint8_t x = graph->x_offset; x < graph->x_offset + graph->width; x++)
    {
        for (uint8_t y = graph->y_offset; y < graph->y_offset + graph->height; y++)
        {
            ssd1306_pixel(ssd, x, y, false);
        }
    }
}

uint8_t scale_x(Graph *graph, float x)
{
    return graph->x_offset + (x * graph->width);
}

uint8_t scale_y(Graph *graph, float y)
{
    // Limita o valor ao range definido
    if (y > graph->y_max) y = graph->y_max;
    if (y < graph->y_min) y = graph->y_min;

    // Converte o valor para a escala de pixels
    float scale = (float)graph->height / (graph->y_max - graph->y_min);
    return graph->y_offset + graph->height - ((y - graph->y_min) * scale);
}

Graph create_graph(float y_min, float y_max)
{
    Graph graph = {
        .x_offset = MARGIN_LEFT,
        .y_offset = 5,
        .width = DISPLAY_WIDTH - MARGIN_LEFT - 5,
        .height = GRAPH_HEIGHT,
        .y_min = y_min,
        .y_max = y_max,
        .y_pixels = GRAPH_HEIGHT,
        .x_divisions = 5, // Valor padrão
        .y_divisions = 5  // Valor padrão
    };
    return graph;
}