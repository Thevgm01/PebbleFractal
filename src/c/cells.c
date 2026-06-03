#include <pebble.h>
#include "cells.h"

#define BITS 16
static int16_t grid[BITS]; // 16x16 bit matrix
static int16_t new_grid[BITS];
static bool check_for_change = true;
static int16_t default_grid[BITS] = {
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
    0b1111000000001111,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000110000000,
    0b0000000110000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1111000000001111,
  #endif
};

static GRect region;
static GRect largest_rect;

static int16_t histogram_stack[BITS];
static int16_t histogram_stack_count = 0;
static void histogram_stack_push(int16_t value) { histogram_stack[histogram_stack_count++] = value; }
static int16_t histogram_stack_pop() { return histogram_stack[--histogram_stack_count]; }
static int16_t histogram_stack_peek() { return histogram_stack[histogram_stack_count - 1]; }

void cells_init(GPoint center, int16_t min_dim, int16_t inset) {
  // Set the pixel region
  region = grect_crop(GRect(center.x - min_dim / 2, center.y - min_dim / 2, min_dim, min_dim), inset);
  
  // Initialize the bit matrix
  for (int16_t i = 0; i < BITS; i++) {
    grid[i] = default_grid[i];
    new_grid[i] = default_grid[i];
  }
}

void cells_mark_occupied(GPoint pos) {
  int16_t y = (pos.y - region.origin.y) * BITS / region.size.h;
  if (y >= 0 && y < BITS) {
    int16_t x = (pos.x - region.origin.x) * BITS / region.size.w;
    if (x >= 0 && x < BITS) {
      check_for_change = true;
      new_grid[y] |= 1 << x;
    }
  }
}

// --- Drawing --- //

GRect cells_local_to_pixel_space(GRect rect) {
  return GRect(rect.origin.x * region.size.w / BITS + region.origin.x,
               rect.origin.y * region.size.h / BITS + region.origin.y,
               rect.size.w * region.size.w / BITS,
               rect.size.h * region.size.h / BITS);
}

void cells_debug_draw(GContext *ctx, GColor filled_color, GColor empty_color, GColor rect_color) {
  // Draw a pixel in the center of each cell
  for (int y = 0; y < BITS; y++) {
    for (int x = 0; x < BITS; x++) {
      bool cell_filled = (grid[y] & (1 << x)) != 0;
      graphics_context_set_stroke_color(ctx, cell_filled ? filled_color : empty_color);
      graphics_draw_pixel(ctx, GPoint(
        region.origin.x + (2 * x + 1) * region.size.w / BITS / 2,
        region.origin.y + (2 * y + 1) * region.size.h / BITS / 2));
    }
  }
  // Draw the largest rectangle
  graphics_context_set_stroke_color(ctx, rect_color);
  graphics_draw_rect(ctx, cells_local_to_pixel_space(largest_rect));
}

// --- Largest rect caclulation --- //

// Slightly prefer wider rects, even if they would otherwise have the same area as a tall rect
static int16_t gsize_score(GSize size) {
  return (size.w - 2) * size.h;
}

// True if a > b
// Enforce minimum width and height
static bool gsize_larger(GSize a, GSize b) {
  return a.w >= 6 && a.h >= 2 && gsize_score(a) > gsize_score(b);
}

// Find the largest rect within a 1D histogram
static GRect compute_histogram_rect(int16_t histogram[], int16_t y) {  
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
  
  // Pop remaining indices
  while (histogram_stack_count > 0) {
    check_next_rect(BITS);
  }
  
  return result;
}

// Find the largest rect within the bit matrix
GRect cells_get_largest_rect() {
  // Ensure that any bits have been set before we do any work
  if (!check_for_change)
    return largest_rect;
  check_for_change = false;
  
  // Flush out the in-progress grid, and determine if anything has truly changed since last time
  int16_t changed_bits = 0;
  for (int16_t i = 0; i < BITS; i++) {
    changed_bits |= grid[i] ^ new_grid[i];
    grid[i] = new_grid[i];
    new_grid[i] = default_grid[i];
  }
  
  // If we set the same bits as last time, no need to reclaculate the rect
  if (changed_bits == 0)
    return largest_rect;
  
  // However, if something did change...
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Bit matrix updated");
  changed_bits = 0;
  largest_rect = GRectZero;

  // Generate histograms by scanning one row at a time
  static int16_t histogram[BITS];
  for (int16_t i = 0; i < BITS; i++)
    histogram[i] = 0; // Reset histogram values since static arrays don't do that apparently

  for (int16_t y = BITS - 1; y >= 0; y--) {
    for (int16_t x = 0; x < BITS; x++)
      histogram[x] = (grid[y] & (1 << x)) != 0 ? 0 : histogram[x] + 1; // Add or reset

    // Scan through each histogram looking for the largest rect
    GRect possible_largest = compute_histogram_rect(histogram, y);

    // Compare with the previous largest
    if (gsize_larger(possible_largest.size, largest_rect.size)) {
      largest_rect = possible_largest;
    }
  }
  
  // Note: this could still end up being the same rect as before (but at least we're sure about it)
  return largest_rect;
}