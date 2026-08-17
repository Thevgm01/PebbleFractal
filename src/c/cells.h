#pragma once
#include <pebble.h>

typedef struct {
  int16_t w_even;
  int16_t w_odd;
  int16_t h_even;
  int16_t h_odd;
} TextCellSizing;

GRect cells_largest_rect;
extern int16_t cells_occupied_grid[];
extern int16_t cells_sensitive_grid[];

void cells_init(GPoint center, int16_t diameter, int16_t inset, TextCellSizing text_sizing);

void cells_reset_grid(int16_t grid[]);
void cells_mark_point(int16_t grid[], GPoint world_pos);
void cells_mark_rect(int16_t grid[], GRect local_rect);
bool cells_sensitive_overwritten();

GRect cells_get_centered_rect(GRect reference_local_rect);
GRect cells_local_to_world(GRect rect);

void cells_update_largest_rect(); // https://www.geeksforgeeks.org/dsa/maximum-size-rectangle-binary-sub-matrix-1s/

void cells_debug_draw(GContext *ctx);
void cells_debug_print(int16_t grid[]);
