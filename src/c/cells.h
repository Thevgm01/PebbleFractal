#pragma once
#include <pebble.h>

#if defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_CHALK)
  // Use a coarser grid for the older/smaller devices
  #define BITS 16
  typedef int16_t grid_t;
#else
  // Otherwise use a nice and roomy grid
  #define BITS 32
  typedef int32_t grid_t;
#endif

typedef struct {
  grid_t base[BITS];
  grid_t screen[BITS];
  grid_t fractal[BITS];
  grid_t sensitive[BITS];
} CellsGrids;

extern int16_t cells_pixels_per_cell;
extern GRect cells_largest_rect;
extern CellsGrids cells_grids;

void cells_init(GPoint center, int16_t diameter, int16_t inset);

void cells_reset_grid(grid_t grid[]);
void cells_mark_point(grid_t grid[], GPoint world_pos);
void cells_mark_line(grid_t grid[], GPoint world_origin, GPoint world_destination);
bool cells_mark_rect(grid_t grid[], GRect world_rect);
bool cells_sensitive_overwritten();
void cells_set_preferred_size(GSize size);

GPoint cells_world_to_local_point(GPoint point);
GRect cells_world_to_local_rect(GRect rect);
GRect cells_local_to_world_rect(GRect rect);

void cells_update_largest_rect(); // https://www.geeksforgeeks.org/dsa/maximum-size-rectangle-binary-sub-matrix-1s/

void cells_debug_draw(GContext *ctx, bool sensitive_alternate_color);
void cells_debug_print(grid_t grid[]);
