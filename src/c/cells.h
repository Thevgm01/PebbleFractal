#pragma once
#include <pebble.h>

#define BITS 32
typedef int32_t grid_t;

typedef struct {
  grid_t base[BITS];
  grid_t screen[BITS];
  grid_t fractal[BITS];
  grid_t sensitive[BITS];
} CellsGrids;

extern GRect cells_largest_rect;
extern CellsGrids cells_grids;

void cells_init(GPoint center, int16_t diameter, int16_t inset);

void cells_reset_grid(grid_t grid[]);
void cells_mark_point(grid_t grid[], GPoint world_pos);
void cells_mark_line(grid_t grid[], GPoint world_origin, GPoint world_destination);
void cells_mark_rect(grid_t grid[], GRect world_rect);
bool cells_sensitive_overwritten();

void cells_set_min_size(GSize size);
GRect cells_world_to_local(GRect rect);
GRect cells_local_to_world(GRect rect);

void cells_update_largest_rect(); // https://www.geeksforgeeks.org/dsa/maximum-size-rectangle-binary-sub-matrix-1s/

void cells_debug_draw(GContext *ctx);
void cells_debug_print(grid_t grid[]);
