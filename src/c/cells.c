#include <pebble.h>
#include "cells.h"
#include "math.h"

#define BITS 16
const int16_t default_grid[BITS] = { // 16x16 bit matrix
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

GRect screen_region;

static int16_t occupied_grid[BITS];
static int16_t sensitive_grid[BITS];

static int16_t histogram_stack[BITS];
static int16_t histogram_stack_count = 0;
static void histogram_stack_push(int16_t value) { histogram_stack[histogram_stack_count++] = value; }
static int16_t histogram_stack_pop() { return histogram_stack[--histogram_stack_count]; }
static int16_t histogram_stack_peek() { return histogram_stack[histogram_stack_count - 1]; }

void cells_init(GPoint center, int16_t diameter, int16_t inset) {
  // Set the pixel region
  screen_region = grect_crop(GRect(center.x - diameter / 2, center.y - diameter / 2, diameter, diameter), inset);
  
  // Initialize the bit matrix
  cells_largest_rect = GRect(0, 0, 16, 16);
  cells_reset_occupied();
}

bool cells_sensitive_overwritten() {
  for (int16_t i = 0; i < BITS; i++)
    if (occupied_grid[i] & sensitive_grid[i] & ~default_grid[i])
      return true;
  return false;
}

void cells_reset_occupied() {
  for (int16_t i = 0; i < BITS; i++)
    occupied_grid[i] = default_grid[i];
}

void cells_reset_sensitive(GRect rect) {
  for (int16_t i = 0; i < BITS; i++)
    sensitive_grid[i] = 0;

  cells_mark_rect_sensitive(cells_world_to_local(rect));
}

void cells_mark_point(GPoint pos) {
  int16_t y = (pos.y - screen_region.origin.y) * BITS / screen_region.size.h;
  if (y >= 0 && y < BITS) {
    int16_t x = (pos.x - screen_region.origin.x) * BITS / screen_region.size.w;
    if (x >= 0 && x < BITS) {
      occupied_grid[y] |= 1 << x;
    }
  }
}

void cells_mark_rect_sensitive(GRect rect) {
  GRect local_rect = cells_world_to_local(rect);
  for (int16_t y = max(local_rect.origin.y, 0); y < min(local_rect.size.h, BITS); y++)
    for (int16_t x = max(local_rect.origin.x, 0); x < min(local_rect.size.w, BITS); x++)
      sensitive_grid[y] |= 1 << x;
}

GRect cells_world_to_local(GRect rect) {
  return GRect((rect.origin.x - screen_region.origin.x) * BITS / screen_region.size.w,
               (rect.origin.y - screen_region.origin.y) * BITS / screen_region.size.h,
               rect.size.w * BITS / screen_region.size.w,
               rect.size.h * BITS / screen_region.size.h);
}

GRect cells_local_to_world(GRect rect) {
  return GRect(rect.origin.x * screen_region.size.w / BITS + screen_region.origin.x,
               rect.origin.y * screen_region.size.h / BITS + screen_region.origin.y,
               rect.size.w * screen_region.size.w / BITS + 2,
               rect.size.h * screen_region.size.h / BITS + 2);
}

// --- Drawing --- //

GRect cells_pixel_to_grid(GRect rect) {
  return GRectZero;
}

int16_t cells_pixels_per_cell() {
  return screen_region.size.w / BITS;
}

void cells_debug_draw(GContext *ctx, GColor sensitive_color, GColor filled_color, GColor empty_color, GColor rect_color) {
  // Draw a pixel in the center of each cell
  for (int y = 0; y < BITS; y++) {
    for (int x = 0; x < BITS; x++) {
      GPoint point = GPoint(
        screen_region.origin.x + (2 * x + 1) * screen_region.size.w / BITS / 2,
        screen_region.origin.y + (2 * y + 1) * screen_region.size.h / BITS / 2);
      
      bool cell_sensitive = (sensitive_grid[y] & (1 << x)) != 0;
      if (cell_sensitive) {
        graphics_context_set_stroke_color(ctx, sensitive_color);
        graphics_draw_circle(ctx, point, 2);
      } else {
        bool cell_filled = (occupied_grid[y] & (1 << x)) != 0;
        graphics_context_set_stroke_color(ctx, cell_filled ? filled_color : empty_color);
        graphics_draw_pixel(ctx, point);
      }
    }
  }
  // Draw the largest rectangle
  graphics_context_set_stroke_color(ctx, rect_color);
  graphics_draw_rect(ctx, cells_local_to_world(cells_largest_rect));
}

// --- Largest rect caclulation --- //

// Slightly prefer wider rects, even if they would otherwise have the same area as a tall rect
// Strongly prefer rects that meet a minimum width and height
static int16_t gsize_score(GSize size) {
  return (size.w - 2) * size.h + (size.w >= 6 && size.h >= 2 ? 1000 : 0);
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
    if (gsize_score(size) > gsize_score(result.size)) {
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
void cells_update_largest_rect() {
  
  cells_largest_rect = GRectZero;
  
  static int16_t histogram[BITS];
  
  // Generate histograms by scanning one row at a time
  for (int16_t i = 0; i < BITS; i++)
    histogram[i] = 0; // Reset histogram values since static arrays don't do that apparently

  for (int16_t y = BITS - 1; y >= 0; y--) {
    for (int16_t x = 0; x < BITS; x++)
      histogram[x] = (occupied_grid[y] & (1 << x)) != 0 ? 0 : histogram[x] + 1; // Add or reset

    // Scan through each histogram looking for the largest rect
    GRect possible_largest = compute_histogram_rect(histogram, y);

    // Compare with the previous largest
    if (gsize_score(possible_largest.size) > gsize_score(cells_largest_rect.size)) {
      cells_largest_rect = possible_largest;
    }
  }
}