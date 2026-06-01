#include <pebble.h>
#include "cells.h"

static int16_t grid[16]; // 16x16
static GRect region;

void cells_init(GPoint *center, int16_t min_dim, int16_t inset) {
  region = grect_crop(GRect(center->x - min_dim / 2, center->y - min_dim / 2,min_dim, min_dim), inset);
}

void cells_reset() {
  for (int i = 0; i < 16; i++) {
    grid[i] = 0;
  }
}

void cells_mark_occupied(GPoint *pos) {
  int16_t y = (pos->y - region.origin.y) * 16 / region.size.h;
  if (y >= 0 && y <= 15) {
    int16_t x = (pos->x - region.origin.x) * 16 / region.size.w;
    if (x >= 0 && x <= 15) {
      grid[y] |= 1 << x;
    }
  }
}

void cells_debug_draw(GContext *ctx, GColor filled_color, GColor empty_color) {
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      bool is_filled = (grid[y] & (1 << x)) > 0;
      graphics_context_set_stroke_color(ctx, is_filled ? filled_color : empty_color);
      graphics_draw_pixel(ctx, GPoint(
        region.origin.x + (2 * x + 1) * region.size.w / 32,
        region.origin.y + (2 * y + 1) * region.size.h / 32));
    }
  }
}