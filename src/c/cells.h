#pragma once
#include <pebble.h>

void cells_init(GPoint *center, int16_t min_dim, int16_t inset);
void cells_reset();
void cells_mark_occupied(GPoint *pos);
void cells_debug_draw(GContext *ctx, GColor filled_color, GColor empty_color);