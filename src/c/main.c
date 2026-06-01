#include <pebble.h>
#include "cells.h"

#define FASTMODE
//#define RANDOM

//#define CIRCLES

#define MINUTE_HAND_LENGTH 60
#define TRUE_HAND_MULT 0 / 3
#define HOUR_HAND_SCALE 7 / 10
#define THICKNESS_MULT 0 / 8

#define MAX_RECURSION_DEPTH 12
#define RECURSE_SCALE 16 / 20

#define min(a, b) a < b ? a : b

// --- Static Variables --- //
static Window *s_window;
static Layer *s_fractal_layer;
static Layer *s_notch_layer;
static TextLayer *s_date_layer;

static Animation *s_animation;
static int8_t s_max_depth;
static int16_t s_length_mult_for_max_depth;

static int16_t s_hour_angle;
static int16_t s_minute_angle;
static GContext *s_fractal_ctx;

// --- Drawing Functions --- //

static GPoint add_to_GPoint(GPoint a, int16_t x, int16_t y) {
  return (GPoint){ .x = a.x + x, .y = a.y + y };
}

static GPoint add_GPoints(GPoint a, GPoint b) {
  return (GPoint){ .x = a.x + b.x, .y = a.y + b.y };
}

// Create a GPoint <radius> pixels away from <origin> rotated by <angle>
static GPoint point_on_circle(GPoint origin, int16_t angle, int16_t radius) {
  return add_to_GPoint(origin,
     sin_lookup(angle) * radius / TRIG_MAX_RATIO,
    -cos_lookup(angle) * radius / TRIG_MAX_RATIO);
}

static int32_t add_angles2(int16_t angle1, int16_t angle2) {
  return (angle1 + angle2) % TRIG_MAX_ANGLE;
}

static void draw_hands_recursive(GPoint origin, int16_t base_angle, int16_t length, int8_t depth) {
  if (length == 0) return;
  
  cells_mark_occupied(&origin);
  
  // Figure out where my hands should be pointing
  int16_t new_hour_angle = add_angles2(base_angle, s_hour_angle);
  int16_t new_minute_angle = add_angles2(base_angle, s_minute_angle);
  if (depth == s_max_depth) {
    length = length * s_length_mult_for_max_depth * MAX_RECURSION_DEPTH / (ANIMATION_NORMALIZED_MAX + 1);
  }
  GPoint minute_point = point_on_circle(origin, new_minute_angle, length);
  GPoint hour_point = point_on_circle(origin, new_hour_angle, length * HOUR_HAND_SCALE);

  // Recurse before drawing so that earlier branches appear on top
  if (depth < s_max_depth) {
    draw_hands_recursive(minute_point, new_minute_angle, length * RECURSE_SCALE, depth + 1);
    draw_hands_recursive(hour_point, new_hour_angle, length * HOUR_HAND_SCALE * RECURSE_SCALE, depth + 1);
  }
  
  int16_t half_width = (MAX_RECURSION_DEPTH - depth + 1) * THICKNESS_MULT;
  
  #ifdef CIRCLES
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_circle(ctx, minute_point, half_width);
    graphics_draw_circle(ctx, hour_point, half_width);
  #else
    // If the hands are too thin, just draw them as individual lines
    if (half_width <= 1) {
      // Make the hands white, for the uppermost hands
      if (depth == 0) {
        graphics_context_set_stroke_color(s_fractal_ctx, GColorWhite);
        graphics_draw_circle(s_fractal_ctx, origin, half_width);
      }
      
      if (length == 1) { // And if they're too short, draw them as points
        graphics_draw_pixel(s_fractal_ctx, minute_point);
        graphics_draw_pixel(s_fractal_ctx, hour_point);
      } else {
        graphics_draw_line(s_fractal_ctx, origin, minute_point);
        graphics_draw_line(s_fractal_ctx, origin, hour_point);
      }
    } else {
      int16_t next_half_width = half_width - 1;
      int16_t minute_normal_angle = add_angles2(new_minute_angle, TRIG_MAX_ANGLE / 4);
      int16_t hour_normal_angle = add_angles2(new_hour_angle, TRIG_MAX_ANGLE / 4);
      
      // Draw a circle at the very center, and make the hands white, for the uppermost hands
      if (depth == 0) {
        graphics_context_set_stroke_color(s_fractal_ctx, GColorWhite);
        graphics_draw_circle(s_fractal_ctx, origin, half_width);
        graphics_context_set_antialiased(s_fractal_ctx, true);
        graphics_context_set_stroke_width(s_fractal_ctx, 3);
        
        // Draw additional long lines for the true hands
        if (TRUE_HAND_MULT > 0 && s_max_depth >= 1) {
          int16_t true_hand_length = MINUTE_HAND_LENGTH * TRUE_HAND_MULT - MINUTE_HAND_LENGTH;
          if (s_max_depth == 1) {
            true_hand_length = true_hand_length * s_length_mult_for_max_depth * MAX_RECURSION_DEPTH / (ANIMATION_NORMALIZED_MAX + 1);
          }
          graphics_draw_line(s_fractal_ctx, minute_point, point_on_circle(origin, s_minute_angle, length + true_hand_length));
          graphics_draw_line(s_fractal_ctx, hour_point, point_on_circle(origin, s_hour_angle, (length + true_hand_length) * HOUR_HAND_SCALE));
        }
        
        graphics_context_set_stroke_width(s_fractal_ctx, 1);
        graphics_context_set_antialiased(s_fractal_ctx, false);
      }
      // Draw the sides of the clock hands
      graphics_draw_line(s_fractal_ctx, // Left minute line
                         point_on_circle(origin, minute_normal_angle, half_width), 
                         point_on_circle(minute_point, minute_normal_angle, next_half_width));
      graphics_draw_line(s_fractal_ctx, // Right minute line
                         point_on_circle(origin, minute_normal_angle, -half_width), 
                         point_on_circle(minute_point, minute_normal_angle, -next_half_width));
      graphics_draw_line(s_fractal_ctx, // Left hour line
                         point_on_circle(origin, hour_normal_angle, half_width), 
                         point_on_circle(hour_point, hour_normal_angle, next_half_width));
      graphics_draw_line(s_fractal_ctx, // Right hour line
                         point_on_circle(origin, hour_normal_angle, -half_width), 
                         point_on_circle(hour_point, hour_normal_angle, -next_half_width));
    }
  #endif
}

// Gets called on tick
static void fractal_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  // Background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  // Current time
  time_t now = time(NULL);
  struct tm* t = localtime(&now);

  // Convert to angle
  #ifdef RANDOM
    srand(now);
    s_hour_angle = rand() % TRIG_MAX_ANGLE;
    s_minute_angle = rand() % TRIG_MAX_ANGLE;
  #else
    s_hour_angle = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 60) + t->tm_min) / (12 * 60);
    s_minute_angle = TRIG_MAX_ANGLE * (t->tm_min * 60 + t->tm_sec) / (60 * 60);
    #ifdef FASTMODE
      s_hour_angle = s_minute_angle;
      s_minute_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
    #endif
  #endif

  cells_reset();
  
  // Draw fractal
  s_fractal_ctx = ctx;
  graphics_context_set_antialiased(ctx, false);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  draw_hands_recursive(center, 0, MINUTE_HAND_LENGTH, 0);
  
  cells_debug_draw(ctx, GColorRed, GColorGreen);
}

static void notch_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  GSize size = bounds.size;
  
  int16_t min_bound = (size.w < size.h ? size.w : size.h) / 2;
  int16_t squircleish_offsets[] = {0, 1, 2, 4, 7, 11, 15, 15, 15, 11, 7, 4, 2, 1, 0};
  
  // Tick marks
  for (int8_t i = 0; i < 60; i++) {
    int16_t angle = TRIG_MAX_ANGLE * i / 60;
    bool is_hour = (i % 5 == 0);
    int16_t outer_r = min_bound + PBL_IF_ROUND_ELSE(0, squircleish_offsets[i % 15]);
    int16_t inner_r = outer_r - (is_hour ? 12 : 5);
    GPoint outer = point_on_circle(center, angle, outer_r);
    GPoint inner = point_on_circle(center, angle, inner_r);

    graphics_context_set_stroke_color(ctx, is_hour ? GColorWhite : GColorLightGray);
    graphics_context_set_stroke_width(ctx, is_hour ? 3 : 1);
    graphics_draw_line(ctx, inner, outer);
  }
}

// --- Animation --- //

static void animation_update_proc(Animation *animation, const AnimationProgress progress) {
  s_max_depth = progress * MAX_RECURSION_DEPTH / (ANIMATION_NORMALIZED_MAX + 1);
  s_length_mult_for_max_depth = progress - (s_max_depth * (ANIMATION_NORMALIZED_MAX + 1) / MAX_RECURSION_DEPTH);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Depth: %d\tprogress: %d", s_max_depth, s_length_mult_for_max_depth);
  layer_mark_dirty(s_fractal_layer);
}
static void animation_stopped_proc(Animation *animation, bool finished, void *context) {
  s_animation = NULL;
}
static AnimationImplementation s_animation_impl = {
  .update = animation_update_proc,
};
static void start_animation() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Starting animation");
  if (s_animation) animation_destroy(s_animation);
  s_animation = animation_create();
  animation_set_implementation(s_animation, &s_animation_impl);
  animation_set_handlers(s_animation, (AnimationHandlers) { .stopped = animation_stopped_proc }, NULL);
  animation_set_duration(s_animation, 2000);
  animation_set_curve(s_animation, AnimationCurveLinear);
  animation_schedule(s_animation);
}

// --- Ticks --- //

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Redraw the fractal every second
  #ifndef FASTMODE
  if (tick_time->tm_sec % 5 != 0) return;
  #endif
  
  layer_mark_dirty(s_fractal_layer);

  // Update the date label once a minute (or on first load)
  if (units_changed & MINUTE_UNIT || units_changed & DAY_UNIT) {
    static char date_buf[16];
    strftime(date_buf, sizeof(date_buf), "%a %b %d", tick_time);
    text_layer_set_text(s_date_layer, date_buf);

    GRect bounds = layer_get_bounds(s_notch_layer);
    GPoint center = grect_center_point(&bounds);
    layer_set_frame(
      text_layer_get_layer(s_date_layer),
      GRect(
        sin_lookup(add_angles2(s_minute_angle, TRIG_MAX_ANGLE / 2)) * 25 / TRIG_MAX_RATIO + center.x,
        -cos_lookup(add_angles2(s_minute_angle, TRIG_MAX_ANGLE / 2)) * 50 / TRIG_MAX_RATIO + center.y,
        80, 20));
  }
}

// --- Window --- //

static void focus_handler(bool focus) {
  if (focus) {
    start_animation();
  } else {
    s_max_depth = 0;
    s_length_mult_for_max_depth = 0;
    layer_mark_dirty(s_fractal_layer);
  }
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  GPoint center = grect_center_point(&bounds);
  int16_t min_dim = min(bounds.size.w, bounds.size.h);

  // Fractal layer
  s_fractal_layer = layer_create(bounds);
  layer_set_update_proc(s_fractal_layer, fractal_update_proc);
  layer_add_child(root, s_fractal_layer);
  
  // Notch layer
  s_notch_layer = layer_create(bounds);
  layer_set_update_proc(s_notch_layer, notch_update_proc);
  layer_add_child(root, s_notch_layer);

  cells_init(&center, min_dim, 10);
  
  // Date label — positioned at the 3 o'clock area (right of center)
  // Pebble Time 2 center is (100, 114); 3 o'clock sits ~ x=148
  GRect date_rect = GRect(bounds.size.w / 2, 104, 70, 20);
  s_date_layer = text_layer_create(date_rect);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer,
  fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_date_layer));

  // Trigger an immediate date update so the label isn't blank on launch
  time_t now = time(NULL);
  struct tm *t   = localtime(&now);
  static char date_buf[16];
  strftime(date_buf, sizeof(date_buf), "%a %b %d", t);
  text_layer_set_text(s_date_layer, date_buf);
  
  // Animation stuff
  app_focus_service_subscribe_handlers((AppFocusHandlers) {
    .did_focus = focus_handler
  });
}

static void window_unload(Window *window) {
  layer_destroy(s_fractal_layer);
  layer_destroy(s_notch_layer);
  text_layer_destroy(s_date_layer);
  if (s_animation) animation_destroy(s_animation);
  app_focus_service_unsubscribe();
}

// --- Initialization --- //

static void init(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  // Subscribe to ticks every second
  // Even though we only have a minute-hand, the fractal can move a lot with only small inputs
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}