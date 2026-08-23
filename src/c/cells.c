#include <pebble.h>
#include "cells.h"
#include "utility.h"

CellsGrids cells_grids;
GRect cells_largest_rect;
int16_t cells_pixels_per_cell;

GRect screen_region;
GSize min_size;

static int16_t histogram_stack[BITS];
static int16_t histogram_stack_count = 0;
static void histogram_stack_push(int16_t value) { histogram_stack[histogram_stack_count++] = value; }
static int16_t histogram_stack_pop() { return histogram_stack[--histogram_stack_count]; }
static int16_t histogram_stack_peek() { return histogram_stack[histogram_stack_count - 1]; }

void cells_init(GPoint center, int16_t diameter, int16_t inset) {
  // Set the pixel region
  screen_region = grect_crop(GRect(center.x - diameter / 2, center.y - diameter / 2, diameter, diameter), inset);
  cells_pixels_per_cell = (diameter - inset * 2) / BITS;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Pixels per cell: %d", cells_pixels_per_cell);
  
  // Block out corner cells
  for (int16_t y = 0; y < BITS / 2; y++) {
    grid_t row = 0;
    for (int16_t x = 0; x < BITS / 2; x++) {
      const GPoint centered = GPoint(x * 2 + 1 - BITS, y * 2 + 1 - BITS);
      const int16_t spread = BITS * 2;
      row |= PBL_IF_ROUND_ELSE(
        // Simple radius check if round
        centered.x * centered.x + centered.y * centered.y > BITS * BITS, 
        
        // Check 4 offset circles to approximate a squircle if rectangular
        // This is an entirely different method than the one used to determine the notch offsets
        // Just make sure they line up I guess
        (centered.x - spread) * (centered.x - spread) + centered.y * centered.y > BITS * BITS * 9 ||
        (centered.x + spread) * (centered.x + spread) + centered.y * centered.y > BITS * BITS * 9 ||
        centered.x * centered.x + (centered.y - spread) * (centered.y - spread) > BITS * BITS * 9 ||
        centered.x * centered.x + (centered.y + spread) * (centered.y + spread) > BITS * BITS * 9)
        
        // Mirror horizontally
        ? (1 << x) | (1 << (BITS - 1 - x)) : 0;
    }
    // Mirror vertically
    cells_grids.base[y] = row;
    cells_grids.base[BITS - 1 - y] = row;
  }
  
  // Block out center cells
  cells_grids.base[BITS / 2 - 1] |= 0b11 << (BITS / 2 - 1);
  cells_grids.base[BITS / 2]     |= 0b11 << (BITS / 2 - 1);
}

void cells_reset_grid(grid_t grid[]) {
  for (int16_t y = 0; y < BITS; y++) {
    grid[y] = 0;
  }
}

void cells_mark_point(grid_t grid[], GPoint world_pos) {
  int16_t y = (world_pos.y - screen_region.origin.y) * BITS / screen_region.size.h;
  if (y >= 0 && y < BITS) {
    int16_t x = (world_pos.x - screen_region.origin.x) * BITS / screen_region.size.w;
    if (x >= 0 && x < BITS) {
      grid[y] |= 1 << x;
    }
  }
}

void mark_line_low(grid_t grid[], int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  int16_t delta_x = x1 - x0;
  int16_t delta_y = y1 - y0;
  int16_t yi = 1;
  if (delta_y < 0) {
    yi = -1;
    delta_y = -delta_y;
  }
  int16_t d = delta_y * 2 - delta_x;
  int16_t y = y0;
  
  for(int16_t x = x0; x <= x1; x++) {
    // Can this maybe be optimized?
    if (x >= 0 && x < BITS && y >= 0 && y < BITS)
      grid[y] |= 1 << x;
    
    if (d > 0) {
      y += yi;
      d += 2 * (delta_y - delta_x);
    }
    else {
      d += 2 * delta_y;
    }
  }
}

void mark_line_high(grid_t grid[], int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  int16_t delta_x = x1 - x0;
  int16_t delta_y = y1 - y0;
  int16_t xi = 1;
  if (delta_x < 0) {
    xi = -1;
    delta_x = -delta_x;
  }
  int16_t d = (2 * delta_x) - delta_y;
  int16_t x = x0;
  
  for (int16_t y = y0; y <= y1; y++) {
    if (x >= 0 && x < BITS && y >= 0 && y < BITS)
      grid[y] |= 1 << x;
    
    if (d > 0) {
      x += xi;
      d += 2 * (delta_x - delta_y);
    }
    else {
      d += 2 * delta_x;
    }
  }
}

void cells_mark_line(grid_t grid[], GPoint world_origin, GPoint world_destintation) {
  
  GPoint origin = cells_world_to_local_point(world_origin);
  GPoint destination = cells_world_to_local_point(world_destintation);
  
  if (abs(destination.y - origin.y) < abs(destination.x - origin.x)) {
    if (origin.x > destination.x)
      mark_line_low(grid, destination.x, destination.y, origin.x, origin.y);
    else
      mark_line_low(grid, origin.x, origin.y, destination.x, destination.y);
  }
  else {
    if (origin.y > destination.y)
      mark_line_high(grid, destination.x, destination.y, origin.x, origin.y);
    else
      mark_line_high(grid, origin.x, origin.y, destination.x, destination.y);
  }
}

void cells_mark_rect(grid_t grid[], GRect world_rect) {
  
  GRect local_rect = cells_world_to_local_rect(world_rect);
  
  // If we're over the left edge, shrink size and shift to origin
  if (local_rect.origin.x < 0) {
    local_rect.size.w += local_rect.origin.x;
    local_rect.origin.x = 0;
  }
  
  // If we're over the right edge, shrink size
  local_rect.size.w = min(BITS, local_rect.size.w + 1);
  
  // This might fail if the size is wider than BITS? Not 100% sure
  grid_t row = ((1 << local_rect.size.w) - 1) << local_rect.origin.x;
  
  // Fill out the rows
  int16_t start_y = max(local_rect.origin.y, 0);
  int16_t end_y = min(local_rect.origin.y + local_rect.size.h, BITS - 1);
  for (int16_t y = start_y; y <= end_y; y++) {
    grid[y] |= row;
  }
}

bool cells_sensitive_overwritten() {
  for (int16_t y = 0; y < BITS; y++)
    if ((cells_grids.fractal[y] | cells_grids.screen[y]) & cells_grids.sensitive[y])
      return true;
  return false;
}

void cells_set_min_size(GSize size) {
  min_size = size;
}

GPoint cells_world_to_local_point(GPoint point) {
  return GPoint(
    (point.x - screen_region.origin.x) * BITS / screen_region.size.w,
    (point.y - screen_region.origin.y) * BITS / screen_region.size.h);
}

GRect cells_world_to_local_rect(GRect rect) {  
  GPoint lower_right = GPoint(rect.origin.x + rect.size.w, rect.origin.y + rect.size.h);
  GPoint local_upper_left = cells_world_to_local_point(rect.origin);
  GPoint local_lower_right = cells_world_to_local_point(lower_right);
  
  return GRect(
    local_upper_left.x,
    local_upper_left.y,
    local_lower_right.x - local_upper_left.x,
    local_lower_right.y - local_upper_left.y);
}

GRect cells_local_to_world_rect(GRect rect) {
  return GRect(rect.origin.x * screen_region.size.w / BITS + screen_region.origin.x - 1,
               rect.origin.y * screen_region.size.h / BITS + screen_region.origin.y - 1,
               rect.size.w * screen_region.size.w / BITS + 2,
               rect.size.h * screen_region.size.h / BITS + 2);
}

// --- Drawing --- //

void cells_debug_draw(GContext *ctx) {  
  // Draw a pixel in the center of each cell
  for (int y = 0; y < BITS; y++) {
    for (int x = 0; x < BITS; x++) {
      grid_t x_bit = 1 << x;
      graphics_context_set_stroke_color(ctx,
        cells_grids.screen[y] & x_bit ? GColorYellow
        : cells_grids.base[y] & x_bit ? GColorBlue 
        : cells_grids.fractal[y] & x_bit ? GColorPurple
        : cells_grids.sensitive[y] & x_bit ? GColorRed
        : GColorGreen); // Empty
      bool is_cross = (cells_grids.screen[y] | cells_grids.fractal[y] | cells_grids.sensitive[y]) & x_bit;
      
      if (is_cross) { // Cross
        #define SIZE_SCALE screen_region.size.w / BITS
        GPoint origin = GPoint(screen_region.origin.x + x * SIZE_SCALE, screen_region.origin.y + y * SIZE_SCALE);
        graphics_draw_line(ctx, origin, GPoint(origin.x + SIZE_SCALE, origin.y + SIZE_SCALE));
        graphics_draw_line(ctx, GPoint(origin.x + SIZE_SCALE, origin.y), GPoint(origin.x, origin.y + SIZE_SCALE));
        #undef SIZE_SCALE
      }
      else { // Point
        graphics_draw_pixel(ctx, GPoint(
          screen_region.origin.x + (2 * x + 1) * screen_region.size.w / BITS / 2,
          screen_region.origin.y + (2 * y + 1) * screen_region.size.h / BITS / 2));
      }
    }
  }
  // Draw the largest rectangle
  graphics_context_set_stroke_color(ctx, GColorOrange);
  graphics_draw_rect(ctx, cells_local_to_world_rect(cells_largest_rect));
}

void cells_debug_print(grid_t grid[]) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Address of grid: 0x%x", grid);
  for (int16_t y = 0; y < BITS; y += 2) {
    static char output_row[BITS * 3];
    for (int16_t x = 0; x < BITS; x++) {
      bool upper = grid[y] & (1 << x);
      bool lower = grid[y + 1] & (1 << x);
      
      if (upper && lower) memcpy(&output_row[x * 3], "█", 3);
      else if (upper) memcpy(&output_row[x * 3], "▀", 3);
      else if (lower) memcpy(&output_row[x * 3], "▄", 3);
      else memcpy(&output_row[x * 3], " ", 3); // Em Space
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, output_row);
  }
}

// --- Largest rect caclulation --- //

static int16_t gsize_score(GSize size) {
  // Must have some area
  if (size.w == 0 || size.h == 0) return 0;
  // Rapidly gain score as we meet the minimum desired dimensions, but not if we exceed
  int16_t w_score = min(size.w * 1000 / min_size.w, 1000);
  int16_t h_score = min(size.h * 1000 / min_size.h, 1000);
  // Somewhat prefer wider rects, even if they technically have the same area as a tall rect
  return size.w * (size.h + 5) + w_score + h_score;
}

static void check_next_rect(int16_t histogram[], int16_t index, int16_t y, GRect *result, int16_t *result_score) {
  int16_t prev_hist_value = histogram[histogram_stack_pop()];
  int16_t width = histogram_stack_count == 0 ? index : index - histogram_stack_peek() - 1;
  GSize size = GSize(width, prev_hist_value);
  int16_t score = gsize_score(size);
  if (score > *result_score) {
    int16_t x = histogram_stack_count == 0 ? 0 : histogram_stack_peek() + 1;
    *result = GRect(x, y, size.w, size.h);
    *result_score = score;
  }
}

// Find the largest rect within a 1D histogram
static GRect compute_histogram_rect(int16_t histogram[], int16_t y) {  
  GRect result = GRectZero;
  int16_t result_score = 0;
  
  // Go through histogram
  for (int16_t i = 0; i < BITS; i++) {
    while (histogram_stack_count > 0 && histogram[histogram_stack_peek()] >= histogram[i]) {
      check_next_rect(histogram, i, y, &result, &result_score);
    }
    histogram_stack_push(i);
  }
  
  // Pop remaining indices
  while (histogram_stack_count > 0) {
    check_next_rect(histogram, BITS, y, &result, &result_score);
  }
  
  return result;
}

// Find the largest rect within the bit matrix
void cells_update_largest_rect() {
  
  cells_largest_rect = GRectZero;
  int16_t largest_rect_score = 0;
  
  // Not a grid, rather an array of counts per row
  static int16_t histogram[BITS];
  
  // Generate histograms by scanning one row at a time
  for (int16_t i = 0; i < BITS; i++)
    histogram[i] = 0; // Reset histogram values since static arrays don't do that apparently

  for (int16_t y = BITS - 1; y >= 0; y--) {
    grid_t combined_row = cells_grids.base[y] | cells_grids.fractal[y] | cells_grids.screen[y];
    for (int16_t x = 0; x < BITS; x++)
      histogram[x] = combined_row & (1 << x) ? 0 : histogram[x] + 1; // Add if open, reset if occupied

    // Scan through each histogram looking for the largest rect
    GRect possible_largest = compute_histogram_rect(histogram, y);
    int16_t score = gsize_score(possible_largest.size);

    // Compare with the previous largest
    if (score > largest_rect_score) {
      cells_largest_rect = possible_largest;
      largest_rect_score = score;
    }
  }
}