#include <pebble.h>
#include "cells.h"

#define SIZE 16
const int16_t LARGEST_BIT = (1 << (SIZE - 1));
static int16_t grid[SIZE]; // 16x16

static GRect region;

void cells_init(GPoint center, int16_t min_dim, int16_t inset) {
  region = grect_crop(GRect(center.x - min_dim / 2, center.y - min_dim / 2, min_dim, min_dim), inset);
}

void cells_reset() {
  for (int16_t i = 0; i < SIZE; i++) {
    grid[i] = 0;
  }
}

void cells_mark_occupied(GPoint pos) {
  int16_t y = (pos.y - region.origin.y) * SIZE / region.size.h;
  if (y >= 0 && y < SIZE) {
    int16_t x = (pos.x - region.origin.x) * SIZE / region.size.w;
    if (x >= 0 && x < SIZE) {
      grid[y] |= 1 << x;
    }
  }
}

bool is_cell_filled(int16_t x, int16_t y) {
  return (grid[y] & (1 << x)) > 0;
}

void cells_debug_draw(GContext *ctx, GColor filled_color, GColor empty_color) {
  for (int y = 0; y < SIZE; y++) {
    for (int x = 0; x < SIZE; x++) {
      graphics_context_set_stroke_color(ctx, is_cell_filled(x, y) ? filled_color : empty_color);
      graphics_draw_pixel(ctx, GPoint(
        region.origin.x + (2 * x + 1) * region.size.w / SIZE / 2,
        region.origin.y + (2 * y + 1) * region.size.h / SIZE / 2));
    }
  }
  GRect visual = GRect(cells_largest_rect.origin.x * region.size.w / SIZE + region.origin.x,
                       cells_largest_rect.origin.y * region.size.w / SIZE + region.origin.y,
                       cells_largest_rect.size.w * region.size.w / SIZE,
                       cells_largest_rect.size.h * region.size.w / SIZE);
  graphics_context_set_stroke_color(ctx, GColorOrange);
  graphics_draw_rect(ctx, visual);
}

// True if a > b
bool gsize_larger(GSize a, GSize b) {
  return a.w * a.h > b.w * b.h;
}

// https://www.geeksforgeeks.org/dsa/maximum-size-rectangle-binary-sub-matrix-1s/
GRect compute_histogram_rect(int16_t histogram[], int16_t y) {
  int16_t stack[SIZE] = {0};
  int16_t stack_idx = -1;
  GRect result = GRectZero;
  
  for (int16_t i = 0; i < SIZE; i++) {
    while (stack_idx >= 0 && histogram[stack[stack_idx]] >= histogram[i]) {
      int16_t top_idx = stack[stack_idx--]; // Pop
      int16_t width = stack_idx < 0 ? i : i - stack[stack_idx] - 1;
      GSize size = GSize(width, histogram[top_idx]);
      if (gsize_larger(size, result.size)) {
        int16_t x = stack_idx < 0 ? 0 : stack[stack_idx] + 1;
        result = GRect(x, y, size.w, size.h);
      }
    }
    stack[++stack_idx] = i; // Push
  }
  
  while (stack_idx >= 0) {
    int16_t top_idx = stack[stack_idx--]; // Pop
    int16_t width = stack_idx < 0 ? SIZE : SIZE - stack[stack_idx] - 1;
    GSize size = GSize(width, histogram[top_idx]);
    if (gsize_larger(size, result.size)) {
      int16_t x = stack_idx < 0 ? 0 : stack[stack_idx] + 1;
      result = GRect(x, y, size.w, size.h);
    }
  }
  
  return result;
}

void cells_update_largest_rect() {
  int16_t histogram[SIZE] = {0};
  cells_largest_rect = GRectZero;
  
  for (int16_t y = SIZE - 1; y >= 0; y--) {
    for (int16_t x = 0; x < SIZE; x++) {
      histogram[x] = (grid[y] & (1 << x)) > 0 ? 0 : histogram[x] + 1;
    }
    
    GRect possible_answer = compute_histogram_rect(histogram, y);
    if (gsize_larger(possible_answer.size, cells_largest_rect.size)) {
      cells_largest_rect = possible_answer;
    }
  }
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "x: %d, y: %d, w: %d, h: %d", 
          cells_largest_rect.origin.x, cells_largest_rect.origin.y,
          cells_largest_rect.size.w, cells_largest_rect.size.h);
}