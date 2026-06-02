#include <pebble.h>
#include "cells.h"

#define BITS 16
int16_t grid[BITS]; // 16x16
int16_t default_grid[BITS] = {
  #ifdef PBL_ROUND
    0b1111100000011111,
    0b1110000000000111,
    0b1100000000000011,
    0b1000000000000001,
    0b1000000000000001,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000110000000,
    0b0000000110000000,
    0b0000000000000000,
    0b0000000000000000,
    0b1000000000000001,
    0b1000000000000001,
    0b1100000000000011,
    0b1110000000000111,
    0b1111100000011111,
  #else
    0b1111100000011111,
    0b1100000000000011,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000110000000,
    0b0000000110000000,
    0b0000000000000000,
    0b0000000000000000,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1100000000000011,
    0b1111100000011111,
  #endif
};

static GRect region;

int16_t histogram_stack[BITS];
int16_t histogram_stack_count = 0;
void histogram_stack_push(int16_t value) { histogram_stack[histogram_stack_count++] = value; }
int16_t histogram_stack_pop() { return histogram_stack[--histogram_stack_count]; }
int16_t histogram_stack_peek() { return histogram_stack[histogram_stack_count - 1]; }

void cells_init(GPoint center, int16_t min_dim, int16_t inset) {
  region = grect_crop(GRect(center.x - min_dim / 2, center.y - min_dim / 2, min_dim, min_dim), inset);
}

void cells_reset() {
  for (int16_t i = 0; i < BITS; i++) {
    grid[i] = default_grid[i];
  }
}

void cells_mark_occupied(GPoint pos) {
  int16_t y = (pos.y - region.origin.y) * BITS / region.size.h;
  if (y >= 0 && y < BITS) {
    int16_t x = (pos.x - region.origin.x) * BITS / region.size.w;
    if (x >= 0 && x < BITS) {
      grid[y] |= 1 << x;
    }
  }
}

bool is_cell_filled(int16_t x, int16_t y) {
  return (grid[y] & (1 << x)) > 0;
}

void cells_debug_draw(GContext *ctx, GColor filled_color, GColor empty_color, GColor rect_color) {
  // Draw a pixel in the center of each cell
  for (int y = 0; y < BITS; y++) {
    for (int x = 0; x < BITS; x++) {
      graphics_context_set_stroke_color(ctx, is_cell_filled(x, y) ? filled_color : empty_color);
      graphics_draw_pixel(ctx, GPoint(
        region.origin.x + (2 * x + 1) * region.size.w / BITS / 2,
        region.origin.y + (2 * y + 1) * region.size.h / BITS / 2));
    }
  }
  // Draw the largest rectangle
  graphics_context_set_stroke_color(ctx, rect_color);
  graphics_draw_rect(ctx, cells_largest_rect);
}

// Slightly prefer wider rects
int16_t gsize_score(GSize size) {
  return (size.w - 2) * size.h;
}

// True if a > b
// Enforce minimum width and height
bool gsize_larger(GSize a, GSize b) {
  return a.w >= 6 && a.h >= 2 && gsize_score(a) > gsize_score(b);
}

GRect compute_histogram_rect(int16_t histogram[], int16_t y) {  
  GRect result = GRectZero;
  
  // Seems like nested functions aren't allowed except under cetain compilers?
  // Appears to work fine for Pebble, and it saves several lines of code
  // Apologies if it's uncouth behavior or something, I'm a C# guy usually
  void check_next_rect(int16_t index) {
    int16_t prev_hist_value = histogram[histogram_stack_pop()];
    int16_t width = histogram_stack_count == 0 ? index : index - histogram_stack_peek() - 1;
    GSize size = GSize(width, prev_hist_value);
    if (gsize_larger(size, result.size)) {
      int16_t x = histogram_stack_count == 0 ? 0 : histogram_stack_peek() + 1;
      result = GRect(x, y, size.w, size.h);
    }
  }
  
  // Go through histogram
  for (int16_t i = 0; i < BITS; i++) {
    while (histogram_stack_count > 0 && histogram[histogram_stack_peek()] >= histogram[i]) {
      check_next_rect(i);
    }
    histogram_stack_push(i);
  }
  
  // Pop remaining
  while (histogram_stack_count > 0) {
    check_next_rect(BITS);
  }
  
  return result;
}

void cells_update_largest_rect() {
  cells_largest_rect = GRectZero;
  
  // Generate histograms by scanning down one row at a time
  int16_t histogram[BITS] = {0};
  for (int16_t y = BITS - 1; y >= 0; y--) {
    for (int16_t x = 0; x < BITS; x++) {
      histogram[x] = (grid[y] & (1 << x)) > 0 ? 0 : histogram[x] + 1; // Add or reset
    }
    
    // Scan through each histogram looking for the largest rect
    GRect possible_largest = compute_histogram_rect(histogram, y);
    
    // Compare with the previous largest
    if (gsize_larger(possible_largest.size, cells_largest_rect.size)) {
      cells_largest_rect = possible_largest;
    }
  }
  
  // Remap from local grid to pixel space
  cells_largest_rect = GRect(cells_largest_rect.origin.x * region.size.w / BITS + region.origin.x,
                             cells_largest_rect.origin.y * region.size.h / BITS + region.origin.y,
                             cells_largest_rect.size.w * region.size.w / BITS,
                             cells_largest_rect.size.h * region.size.h / BITS);
}