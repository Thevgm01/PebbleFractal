#include <pebble.h>

//#define FASTMODE

//#define CIRCLES

#define MINUTE_HAND_LENGTH 40
#define HOUR_HAND_SCALE 6 / 10
#define THICKNESS_MULT 1 / 2
#define TRUE_HAND_MULT 5 / 2

#define MAX_RECURSION_DEPTH 8
#define RECURSE_SCALE 18 / 20

// --- Static Variables --- //
static Window *s_window;
static Layer *s_fractal_layer;
static Layer *s_notch_layer;
static TextLayer *s_date_layer;

static Animation *s_animation;
static int16_t s_total_length;
static int16_t s_animated_max_length;
static int16_t s_length_per_depth[MAX_RECURSION_DEPTH];
static int8_t s_max_depth;

static int16_t s_hour_angle;
static int16_t s_minute_angle;

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

static void draw_hands_recursive(GContext *ctx, GPoint origin, int16_t base_angle, int8_t depth) {  
  // Figure out where my hands should be pointing
  int16_t new_hour_angle = add_angles2(base_angle, s_hour_angle);
  int16_t new_minute_angle = add_angles2(base_angle, s_minute_angle);
  int16_t length = s_length_per_depth[depth];
  GPoint minute_point = point_on_circle(origin, new_minute_angle, length);
  GPoint hour_point = point_on_circle(origin, new_hour_angle, length * HOUR_HAND_SCALE);

  // Recurse before drawing so that earlier branches appear on top
  if (depth < s_max_depth) {
    draw_hands_recursive(ctx, minute_point, new_minute_angle, depth + 1);
    draw_hands_recursive(ctx, hour_point, new_hour_angle, depth + 1);
  }
  
  uint16_t half_width = (MAX_RECURSION_DEPTH - depth + 1) * THICKNESS_MULT;
  
  #ifdef CIRCLES
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_circle(ctx, minute_point, half_width);
    graphics_draw_circle(ctx, hour_point, half_width);
  #else
    // If the lines are too thin, just draw them as individual lines
    if (half_width <= 1) {
      graphics_draw_line(ctx, origin, minute_point);
      graphics_draw_line(ctx, origin, hour_point);
    }
    else {
      uint16_t next_half_width = half_width - 1;
      uint16_t minute_normal_angle = add_angles2(new_minute_angle, TRIG_MAX_ANGLE / 4);
      uint16_t hour_normal_angle = add_angles2(new_hour_angle, TRIG_MAX_ANGLE / 4);
      
      // Draw a circle at the very center, and make the hands white
      if (depth == 0) {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_draw_circle(ctx, origin, half_width);
        
        graphics_draw_line(ctx, minute_point, point_on_circle(origin, s_minute_angle, MINUTE_HAND_LENGTH * TRUE_HAND_MULT));
        graphics_draw_line(ctx, hour_point, point_on_circle(origin, s_hour_angle, MINUTE_HAND_LENGTH * TRUE_HAND_MULT * HOUR_HAND_SCALE));
      }
      graphics_draw_line(ctx, // Left minute line
                         point_on_circle(origin, minute_normal_angle, half_width), 
                         point_on_circle(minute_point, minute_normal_angle, next_half_width));
      graphics_draw_line(ctx, // Right minute line
                         point_on_circle(origin, minute_normal_angle, -half_width), 
                         point_on_circle(minute_point, minute_normal_angle, -next_half_width));
      graphics_draw_line(ctx, // Left hour line
                         point_on_circle(origin, hour_normal_angle, half_width), 
                         point_on_circle(hour_point, hour_normal_angle, next_half_width));
      graphics_draw_line(ctx, // Right hour line
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
  s_hour_angle = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 60) + t->tm_min) / (12 * 60);
  s_minute_angle = TRIG_MAX_ANGLE * (t->tm_min * 60 + t->tm_sec) / (60 * 60);
  #ifdef FASTMODE
  s_hour_angle = s_minute_angle;
  s_minute_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  #endif

  // Draw fractal
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Minute max length: %d", s_animated_max_length);
  graphics_context_set_antialiased(ctx, false);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  draw_hands_recursive(ctx, center, 0, 0);
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

static void animation_update_proc(Animation *animation, const AnimationProgress progress) {
  if (progress == 0) { // Note: Maybe just don't mark dirty to skip rendering in the first place?
    s_animated_max_length = 0;
    s_length_per_depth[0] = 0;
    s_max_depth = 0;
  }
  else {
    s_animated_max_length = s_total_length * progress / ANIMATION_NORMALIZED_MAX;

    int8_t i = 0;
    int16_t cumulative_length = 0;
    int16_t desired_depth_length = MINUTE_HAND_LENGTH;
    while (i < MAX_RECURSION_DEPTH) {
      if (s_animated_max_length - cumulative_length < desired_depth_length) {
        s_length_per_depth[i] = s_animated_max_length - cumulative_length;
        s_max_depth = i;
        break;
      }
      else {
        s_length_per_depth[i] = desired_depth_length;
        cumulative_length += desired_depth_length;
        desired_depth_length = desired_depth_length * RECURSE_SCALE;
        i++;
      }
    }
  }
  
  //for (int8_t i = 0; i < MAX_RECURSION_DEPTH; i++) { APP_LOG(APP_LOG_LEVEL_DEBUG, "[%d] %d", i, s_length_per_depth[i]); }
  layer_mark_dirty(s_fractal_layer);
}
static void animation_stopped_proc(Animation *animation, bool finished, void *context) {
  s_animation = NULL;
}
static AnimationImplementation s_animation_impl = {
  .update = animation_update_proc,
};

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
static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  // Fractal layer
  s_fractal_layer = layer_create(bounds);
  layer_set_update_proc(s_fractal_layer, fractal_update_proc);
  layer_add_child(root, s_fractal_layer);
  
  // Notch layer
  s_notch_layer = layer_create(bounds);
  layer_set_update_proc(s_notch_layer, notch_update_proc);
  layer_add_child(root, s_notch_layer);

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
  
  // Start the animation
  if (s_animation) {
    animation_destroy(s_animation);
  }
  s_animation = animation_create();
  animation_set_implementation(s_animation, &s_animation_impl);
  animation_set_handlers(s_animation, (AnimationHandlers) { .stopped = animation_stopped_proc }, NULL);
  animation_set_duration(s_animation, 1500);
  animation_set_curve(s_animation, AnimationCurveEaseOut);
  animation_schedule(s_animation);
}

static void window_unload(Window *window) {
  layer_destroy(s_fractal_layer);
  layer_destroy(s_notch_layer);
  text_layer_destroy(s_date_layer);
  if (s_animation) {
    animation_destroy(s_animation);
  }
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
  
  // Calculate the total length of the minute hand for animation purposes
  s_total_length = 0;
  int16_t last_length = MINUTE_HAND_LENGTH;
  for (int8_t i = 0; i < MAX_RECURSION_DEPTH; i++){
    s_total_length += last_length;
    last_length = last_length * RECURSE_SCALE;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Total length: %d", s_total_length);
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