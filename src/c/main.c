#include <pebble.h>
#include "settings.h"
#include "cells.h"
#include "utility.h"

//#define SCREENSHOTMODE
//#define RANDOM
//#define CIRCLES

#define MAX_RECURSION_DEPTH 13
#define DATE_CROP 0

// --- Static Variables --- //

// Windows
static Window *s_window;
static GRect s_window_bounds;
static Layer *s_fractal_layer;
static Layer *s_notch_layer;
static TextLayer *s_date_layer;

// Animation
static Animation *s_animation;
static int8_t s_max_animation_depth;
static int16_t s_length_mult_for_max_depth;

// Fractal drawing
static int16_t s_hour_hand_scale;
static uint16_t s_hour_angle;
static uint16_t s_minute_angle;
static GContext *s_fractal_ctx;
static bool s_mark_points;
static GRect s_date_rect;

// Screenshot mode
static int16_t s_screenshot_frame = 0;

// --- Drawing Functions --- //

static GPoint add_gpoints(GPoint a, GPoint b) { return GPoint(a.x + b.x, a.y + b.y); }
static GPoint sub_gpoints(GPoint a, GPoint b) { return GPoint(a.x - b.x, a.y - b.y); }

// Create a GPoint <radius> pixels away from <origin> rotated by <angle>
static GPoint point_on_circle(GPoint origin, int16_t angle, int16_t radius) {
  return gpoint_shift(origin,
     sin_lookup(angle) * radius / TRIG_MAX_RATIO,
    -cos_lookup(angle) * radius / TRIG_MAX_RATIO);
}

static int32_t add_angles2(int16_t angle1, int16_t angle2) {
  return (angle1 + angle2) % TRIG_MAX_ANGLE;
}

static void draw_hands_recursive(GPoint origin, int16_t base_angle, int16_t length, int8_t depth) {
  #define SCALE_WITH_ANIMATION(var) var * s_length_mult_for_max_depth * MAX_RECURSION_DEPTH / ANIMATION_NORMALIZED_MAX
  #define TRUE_HAND_LENGTH settings.MinuteHandLength * settings.FirstHandScale / 100 - settings.MinuteHandLength
  
  // Animate the length per depth
  if (depth == s_max_animation_depth) length = SCALE_WITH_ANIMATION(length);
  
  // Early return if our length is zero (nothing to draw)
  if (length == 0)
    return;
  int16_t hour_length = length * s_hour_hand_scale / 100;
  
  // Figure out where our hands should be pointing
  int16_t new_hour_angle = add_angles2(base_angle, s_hour_angle);
  int16_t new_minute_angle = add_angles2(base_angle, s_minute_angle);
  GPoint minute_point = point_on_circle(origin, new_minute_angle, length);
  GPoint hour_point = point_on_circle(origin, new_hour_angle, hour_length);
  
  // Mark the next points occupied for determining the date placement
  if (s_mark_points) {
    if (depth == 0) {
      cells_mark_line(cells_grids.fractal, origin, point_on_circle(origin, s_minute_angle, length + TRUE_HAND_LENGTH));
      cells_mark_line(cells_grids.fractal, origin, point_on_circle(origin, s_hour_angle, (length + TRUE_HAND_LENGTH) * s_hour_hand_scale / 100));
    }
    else {
      if (length > cells_pixels_per_cell * 3) cells_mark_line(cells_grids.fractal, origin, minute_point);
      else cells_mark_point(cells_grids.fractal, minute_point);
      
      if (hour_length > cells_pixels_per_cell * 3) cells_mark_line(cells_grids.fractal, origin, hour_point);
      else cells_mark_point(cells_grids.fractal, hour_point);
    }
  }
  
  // Recurse before drawing so that earlier branches appear on top
  if (depth < s_max_animation_depth) {
    draw_hands_recursive(minute_point, new_minute_angle, length * settings.RecurseScale / 100, depth + 1);
    draw_hands_recursive(hour_point, new_hour_angle, length * s_hour_hand_scale * settings.RecurseScale / 10000, depth + 1);
  }
    
  // Return if we have no context to draw to (will only happen during the first run for initial date placement)
  if (s_fractal_ctx == NULL)
    return;
  
  // Determine line width (can be zero, which will draw a 1-pixel line)
  int16_t half_width = (MAX_RECURSION_DEPTH - depth + 1) * settings.WidthScale / 100;
  
  // Set the colors based on the line length
  if (length > settings.MinuteHandLength / 2)
    // Make the hands white for the uppermost layer
    graphics_context_set_stroke_color(s_fractal_ctx, settings.PrimaryColor);
  else if (length > settings.MinuteHandLength / 6)
    graphics_context_set_stroke_color(s_fractal_ctx, settings.SecondaryColor);
  else
    graphics_context_set_stroke_color(s_fractal_ctx, settings.TertiaryColor);
  
  #ifdef CIRCLES // Draw circles
    if (half_width > 1) {
      graphics_draw_circle(s_fractal_ctx, minute_point, half_width);
      graphics_draw_circle(s_fractal_ctx, hour_point, half_width);
    } else { // Circles too small, draw a point
      graphics_draw_pixel(s_fractal_ctx, minute_point);
      graphics_draw_pixel(s_fractal_ctx, hour_point);
    }
  #else // Draw lines
    if (half_width <= 1) { // Hands are thin, draw them as individual lines
      if (length > 1) {
        graphics_draw_line(s_fractal_ctx, origin, minute_point);
        graphics_draw_line(s_fractal_ctx, origin, hour_point);
      } else { // Line's too short, draw a point
        graphics_draw_pixel(s_fractal_ctx, minute_point);
        graphics_draw_pixel(s_fractal_ctx, hour_point);
      }
    } else { // Hands are thick, draw two lines on either side
      int16_t next_half_width = half_width - 1;
      int16_t minute_normal_angle = add_angles2(new_minute_angle, TRIG_MAX_ANGLE / 4);
      int16_t hour_normal_angle = add_angles2(new_hour_angle, TRIG_MAX_ANGLE / 4);

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
  
  if (depth == 0) {
    if (half_width > 1) {
      // Draw a circle at the very center
      graphics_draw_circle(s_fractal_ctx, origin, half_width);
    }

    // Draw additional long lines for the true hands
    if (settings.FirstHandScale > 0 && s_max_animation_depth >= 1) {
      int16_t true_hand_length = TRUE_HAND_LENGTH;
      if (s_max_animation_depth == 1) true_hand_length = SCALE_WITH_ANIMATION(true_hand_length);
      graphics_draw_line(s_fractal_ctx, minute_point,
                         point_on_circle(origin, s_minute_angle, length + true_hand_length));
      graphics_draw_line(s_fractal_ctx, hour_point,
                         point_on_circle(origin, s_hour_angle, (length + true_hand_length) * s_hour_hand_scale / 100));
    }
  }
  #endif
  
  #undef SCALE_WITH_ANIMATION
}

static void fractal_update_proc(Layer *layer, GContext *ctx) {
  // Early return if we're unable to render anything
  if (s_max_animation_depth == 0 && s_length_mult_for_max_depth == 0)
    return;
  
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  // Current time
  time_t now = time(NULL);
  struct tm* t = localtime(&now);
  
  // Convert to angle
  #ifdef RANDOM
    srand(now);
    s_hour_angle = rand() % TRIG_MAX_ANGLE;
    s_minute_angle = rand() % TRIG_MAX_ANGLE;
    move_date = true;
  #elif defined(SCREENSHOTMODE)
    s_hour_angle = TRIG_MAX_ANGLE * 1 / 4;
    s_minute_angle = TRIG_MAX_ANGLE * s_screenshot_frame / 60;
    s_mark_points = true;
  #else
    s_hour_angle = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 60) + t->tm_min) / (12 * 60);
    s_minute_angle = TRIG_MAX_ANGLE * (t->tm_min * 60 + t->tm_sec) / (60 * 60);
    if (settings.DebugSpeed) {
      s_hour_angle = s_minute_angle;
      s_minute_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
    }
  #endif
  
  // Check if we need to move the date twice per minute, or on first load
  s_mark_points = settings.ShowDate && (ctx == NULL || (!s_animation && (settings.DebugSpeed || t->tm_sec % 30 == 0)));
  if (s_mark_points) cells_reset_grid(cells_grids.fractal);
  
  // Draw fractal
  s_fractal_ctx = ctx;
  if (ctx != NULL) {
    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_stroke_width(ctx, 1);
  }
  draw_hands_recursive(center, 0, settings.MinuteHandLength, 0);
  
  s_mark_points = false;
  
  // Move/update date
  if (settings.ShowDate) {
    // Change the text every minute, or on first load
    bool update_date = ctx == NULL || (!s_animation && t->tm_sec == 0);
    if (update_date) {
      static char date_buf[16];
      strftime(date_buf, sizeof(date_buf), "%a %b %d", t);
      text_layer_set_text(s_date_layer, date_buf);
      s_date_rect.size = text_layer_get_content_size(s_date_layer);
      cells_set_min_size(cells_world_to_local_rect(grect_crop(s_date_rect, DATE_CROP)).size);
    }
    
    // Move the date if the fractal passed over it, or on first load
    bool move_date = ctx == NULL || cells_sensitive_overwritten();
    if (move_date) {
      
      // Calculate the largest rect (expensive!)
      cells_update_largest_rect();
      
      s_date_rect.origin = center_in_rect(s_date_rect.size, cells_local_to_world_rect(cells_largest_rect));
      
      // The text seems to appear at the bottom of the reported rect, so manually shift the layer up a bit
      // Probably needs to be adjusted on a per-font-basis
      layer_set_frame(text_layer_get_layer(s_date_layer), GRect(
        s_date_rect.origin.x, 
        s_date_rect.origin.y - 4, 
        200, 
        30));
      
      APP_LOG_GRECT(APP_LOG_LEVEL_DEBUG, "Date overwritten: ", s_date_rect);
      
      cells_reset_grid(cells_grids.sensitive);
      cells_sensitive_force = cells_mark_rect(cells_grids.sensitive, grect_crop(s_date_rect, DATE_CROP));
      //cells_debug_print(cells_sensitive_grid);
    }
    
    if (ctx != NULL && settings.DebugGrid) {
      if (!move_date)
        cells_update_largest_rect();

      cells_debug_draw(ctx);

      graphics_context_set_stroke_color(ctx, GColorCyan);
      graphics_draw_rect(ctx, grect_crop(s_date_rect, DATE_CROP));
    }
  }
}

static int16_t angle_to_squircle_offset(int32_t angle) {
  // Angle goes from 0-TRIG_MAX_ANGLE
  // Constrain to the first 90°
  angle = angle % (TRIG_MAX_ANGLE / 4);
  // Reverse direction if between 45° and 90°
  if (angle > TRIG_MAX_ANGLE / 8) angle = TRIG_MAX_ANGLE / 4 - angle;
  // Calculate offset
  return angle * angle / 3000000;
}

static void notch_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  GSize size = bounds.size;
  
  graphics_context_set_antialiased(ctx, true);
  
  int16_t radius = min(size.w, size.h) / 2 - 8;
    
  // Tick marks
  for (int8_t i = 0; i < 15; i++) {
    int16_t angle = i * TRIG_MAX_ANGLE / 60;
    bool is_hour = (i % 5 == 0);
    int16_t outer_r = radius + PBL_IF_ROUND_ELSE(0, angle_to_squircle_offset(angle));
    
    int32_t cos = cos_lookup(angle);
    int32_t sin = sin_lookup(angle);
    int16_t x1 = cos * outer_r / TRIG_MAX_RATIO;
    int16_t y1 = sin * outer_r / TRIG_MAX_RATIO;
    
    //graphics_context_set_stroke_color(ctx, is_hour ? settings.PrimaryColor : settings.SecondaryColor);
    if (is_hour) {
      int16_t inner_r = outer_r - 8;
      int16_t x2 = cos * inner_r / TRIG_MAX_RATIO;
      int16_t y2 = sin * inner_r / TRIG_MAX_RATIO;
      graphics_context_set_stroke_color(ctx, settings.PrimaryColor);
      graphics_context_set_stroke_width(ctx, 3);
      graphics_draw_line(ctx, gpoint_shift(center, x1, y1), gpoint_shift(center, x2, y2));
      graphics_draw_line(ctx, gpoint_shift(center, -y1, x1), gpoint_shift(center, -y2, x2));
      graphics_draw_line(ctx, gpoint_shift(center, -x1, -y1), gpoint_shift(center, -x2, -y2));
      graphics_draw_line(ctx, gpoint_shift(center, y1, -x1), gpoint_shift(center, y2, -x2));
    }
    else {
      graphics_context_set_fill_color(ctx, settings.PrimaryColor);
      graphics_fill_circle(ctx, gpoint_shift(center, x1, y1), 1);
      graphics_fill_circle(ctx, gpoint_shift(center, -y1, x1), 1);
      graphics_fill_circle(ctx, gpoint_shift(center, -x1, -y1), 1);
      graphics_fill_circle(ctx, gpoint_shift(center, y1, -x1), 1);
    }
  }
  
  // Draw minute/hour notches  
  GPoint hour = point_on_circle(center, s_hour_angle, radius +
                                PBL_IF_ROUND_ELSE(0, angle_to_squircle_offset(s_hour_angle)));
  GPoint minute = point_on_circle(center, s_minute_angle, radius +
                                  PBL_IF_ROUND_ELSE(0, angle_to_squircle_offset(s_minute_angle)));
  GPoint cross_offset = point_on_circle(GPointZero, s_minute_angle, 8);
  /*
  graphics_context_set_stroke_color(ctx, settings.BackgroundColor);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_circle(ctx, hour, 8);
  graphics_context_set_stroke_width(ctx, 7);
  graphics_draw_line(ctx, gpoint_shift(minute, cross_offset.x, cross_offset.y), gpoint_shift(minute, -cross_offset.x, -cross_offset.y));
  graphics_draw_line(ctx, gpoint_shift(minute, -cross_offset.y, cross_offset.x), gpoint_shift(minute, cross_offset.y, -cross_offset.x));
  */
  graphics_context_set_stroke_color(ctx, settings.PrimaryColor);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_circle(ctx, hour, 6);
  graphics_draw_line(ctx, gpoint_shift(minute, cross_offset.x, cross_offset.y), gpoint_shift(minute, -cross_offset.x, -cross_offset.y));
  graphics_draw_line(ctx, gpoint_shift(minute, -cross_offset.y, cross_offset.x), gpoint_shift(minute, cross_offset.y, -cross_offset.x));
}

// --- Ticks --- //

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Redraw the fractal every 10 seconds
  if (!s_animation && (settings.DebugSpeed || tick_time->tm_sec % 10 == 0)) {

    #ifdef SCREENSHOTMODE
    s_screenshot_frame = (s_screenshot_frame + 1) % 60;
    #endif

    layer_mark_dirty(s_fractal_layer);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Dirty: tick_handler");
  }
}

// --- Animation --- //

static void animation_update_proc(Animation *animation, const AnimationProgress progress) {
  s_max_animation_depth = progress * MAX_RECURSION_DEPTH / ANIMATION_NORMALIZED_MAX;
  s_length_mult_for_max_depth = progress - (s_max_animation_depth * ANIMATION_NORMALIZED_MAX / MAX_RECURSION_DEPTH);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Dirty: animation_update_proc (Depth: %d, progress: %d)", s_max_animation_depth, s_length_mult_for_max_depth);
  layer_mark_dirty(s_fractal_layer);
}

static void animation_stopped_proc(Animation *animation, bool finished, void *context) {
  s_max_animation_depth = MAX_RECURSION_DEPTH;
  s_length_mult_for_max_depth = 0;
  s_animation = NULL;
}

static AnimationImplementation s_animation_impl = { .update = animation_update_proc };

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

// --- Settings --- //

static void post_settings_loaded() {
  text_layer_set_text_color(s_date_layer, settings.PrimaryColor);
  window_set_background_color(s_window, settings.BackgroundColor);
  layer_set_hidden(text_layer_get_layer(s_date_layer), !settings.ShowDate); // Also hide if DebugGrid is enabled
  
  s_hour_hand_scale = settings.HourHandLength * 100 / settings.MinuteHandLength;
  
  if (settings.ShowDate) {
    animation_stopped_proc(s_animation, false, NULL);
    
    GFont font;
    switch (settings.Font) {
      case 0: font = fonts_get_system_font(FONT_KEY_GOTHIC_14); break;
      case 1: font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD); break;
      default:
      case 2: font = fonts_get_system_font(FONT_KEY_GOTHIC_18); break;
      case 3: font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD); break;
      case 4: font = fonts_get_system_font(FONT_KEY_GOTHIC_24); break;
      case 5: font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD); break;
      case 6: font = fonts_get_system_font(FONT_KEY_GOTHIC_28); break;
      case 7: font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD); break;
      case 13: font = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21); break;
      case 16: font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS); break;
    }
    text_layer_set_font(s_date_layer, font);
    
    fractal_update_proc(s_fractal_layer, NULL);
  }
}

// --- Window --- //

static void focus_handler(bool focus) {
  if (focus) {
    start_animation();
  } else {
    s_max_animation_depth = 0;
    s_length_mult_for_max_depth = 0;
    layer_mark_dirty(s_fractal_layer);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Dirty: focus_handler");
  }
}

static void area_change_handler(GRect final_unobstructed_screen_area, void *ctx) {  
  int16_t delta_height = s_window_bounds.size.h - final_unobstructed_screen_area.size.h;
  GRect obstructed_area = GRect(0, s_window_bounds.size.h - delta_height, s_window_bounds.size.w, delta_height);
  
  cells_reset_grid(cells_grids.screen);
  
  if (delta_height > 0) {
    cells_mark_rect(cells_grids.screen, obstructed_area);
    if (settings.ShowDate) {
      layer_mark_dirty(s_fractal_layer);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Dirty: area_change_handler");
    }
  }
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  s_window_bounds = layer_get_bounds(root);
  GPoint center = grect_center_point(&s_window_bounds);
  int16_t min_dim = min(s_window_bounds.size.w, s_window_bounds.size.h);

  // Fractal layer
  s_fractal_layer = layer_create(s_window_bounds);
  layer_set_update_proc(s_fractal_layer, fractal_update_proc);
  layer_add_child(root, s_fractal_layer);
  
  // Notch layer
  s_notch_layer = layer_create(s_window_bounds);
  layer_set_update_proc(s_notch_layer, notch_update_proc);
  layer_add_child(root, s_notch_layer);
  
  // Text layer
  GRect date_rect = GRect(0, 0, 200, 30);
  s_date_layer = text_layer_create(date_rect);
  text_layer_set_background_color(s_date_layer, GColorClear);
  layer_add_child(root, text_layer_get_layer(s_date_layer));
  
  // Initialize the occupied screen cell tracker
  cells_init(center, min_dim, 20);

  // Load settings
  settings_loaded_callback = post_settings_loaded;
  settings_load();
    
  // Animation stuff
  #ifndef SCREENSHOTMODE
  s_max_animation_depth = 0;
  app_focus_service_subscribe_handlers((AppFocusHandlers) {
    .did_focus = focus_handler
  });
  #endif
  
  unobstructed_area_service_subscribe((UnobstructedAreaHandlers) {
    .will_change = area_change_handler
  }, NULL);
}

static void window_unload(Window *window) {
  layer_destroy(s_fractal_layer);
  layer_destroy(s_notch_layer);
  text_layer_destroy(s_date_layer);
  if (s_animation) animation_destroy(s_animation);
  app_focus_service_unsubscribe();
  unobstructed_area_service_unsubscribe();
}

// --- Initialization --- //

static void init(void) {  
  // Register settings callbacks
  app_message_register_inbox_received(settings_inbox_received_callback);
  app_message_open(256, 0);
  
  // Create the main window
  s_window = window_create();
  window_set_background_color(s_window, settings.BackgroundColor);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  // The fractal can change a lot over a short time,
  // so tick every second even though we only have a minute hand
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
  app_message_deregister_callbacks();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}