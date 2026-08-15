#pragma once
#include <pebble.h>

GRect cells_largest_rect;

void cells_init(GPoint center, int16_t diameter, int16_t inset);

void cells_reset_occupied();
void cells_reset_sensitive(GRect rect);
bool cells_sensitive_overwritten();

GRect cells_world_to_local(GRect rect);
GRect cells_local_to_world(GRect rect);

void cells_mark_point(GPoint pos);
void cells_mark_rect_sensitive(GRect rect);

void cells_update_largest_rect(); // https://www.geeksforgeeks.org/dsa/maximum-size-rectangle-binary-sub-matrix-1s/

void cells_debug_draw(GContext *ctx, GColor sensitive_color, GColor filled_color, GColor empty_color, GColor rect_color);