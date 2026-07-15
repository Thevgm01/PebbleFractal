#pragma once
#include <pebble.h>

GRect cells_largest_rect;

void cells_init(GPoint center, int16_t min_dim, int16_t inset);
bool cells_sensitive_overwritten();
void cells_reset_occupied();
void cells_reset_sensitive();
void cells_mark_occupied(GPoint pos);
void cells_update_largest_rect(); // https://www.geeksforgeeks.org/dsa/maximum-size-rectangle-binary-sub-matrix-1s/
GRect cells_local_to_pixel_space(GRect rect);
void cells_debug_draw(GContext *ctx, GColor sensitive_color, GColor filled_color, GColor empty_color, GColor rect_color);