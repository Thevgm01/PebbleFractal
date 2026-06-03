#pragma once
#include <pebble.h>

GRect cells_largest_rect;

void cells_init(GPoint center, int16_t min_dim, int16_t inset);
void cells_mark_occupied(GPoint pos);
GRect cells_get_largest_rect(); // https://www.geeksforgeeks.org/dsa/maximum-size-rectangle-binary-sub-matrix-1s/
void cells_debug_draw(GContext *ctx, GColor filled_color, GColor empty_color, GColor rect_color);