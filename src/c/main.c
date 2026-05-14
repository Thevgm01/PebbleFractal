#include <pebble.h>

//#define FASTMODE

#define CLOCK_RADIUS 92

#define MINUTE_HAND_LENGTH 30
#define HOUR_HAND_SCALE 7 / 10
#define TRUE_HAND_BORDER_RATIO 20 / 10

#define MAX_RECURSION_DEPTH 8
#define RECURSE_SCALE 9 / 10

// ── Globals ───────────────────────────────────────────────────────────────────
static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer *s_date_layer;

static int32_t s_hour_angle;
static int32_t s_minute_angle;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Create a GPoint <radius> pixels away from <origin> rotated by <angle>
static GPoint point_on_circle(GPoint origin, int32_t angle, int32_t radius) {
  return (GPoint){
    .x = origin.x + (int32_t)(sin_lookup(angle) * radius / TRIG_MAX_RATIO),
    .y = origin.y - (int32_t)(cos_lookup(angle) * radius / TRIG_MAX_RATIO),
  };
}

// The main important function
static void draw_hands_recursive(GContext *ctx, GPoint center, 
                                 int32_t angle, int32_t radius, int8_t depth) {
  
  // Figure out where my hands should be pointing
  int32_t local_hour_angle = (angle + s_hour_angle) % TRIG_MAX_ANGLE;
  int32_t local_minute_angle = (angle + s_minute_angle) % TRIG_MAX_ANGLE;
  GPoint hour_point = point_on_circle(center, local_hour_angle, radius * HOUR_HAND_SCALE);
  GPoint minute_point = point_on_circle(center, local_minute_angle, radius);
  
  // Recurse before drawing so that earlier branches appear on top
  if (depth < MAX_RECURSION_DEPTH) {
    draw_hands_recursive(ctx, minute_point, local_minute_angle, radius * RECURSE_SCALE, depth + 1);
    draw_hands_recursive(ctx, hour_point, local_hour_angle, radius * HOUR_HAND_SCALE * RECURSE_SCALE, depth + 1);
    
    // The initial hands get a thick border around them to make it easier to actually read the time
    if (depth == 0) {
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_context_set_stroke_width(ctx, (MAX_RECURSION_DEPTH + 1) * TRUE_HAND_BORDER_RATIO);
      graphics_draw_line(ctx, center, minute_point);
      graphics_draw_line(ctx, center, hour_point);
      graphics_context_set_stroke_color(ctx, GColorWhite);
    }
  }
  
  // According to the documentation, only odd width values are supported?
  graphics_context_set_stroke_width(ctx, MAX_RECURSION_DEPTH - depth + 1);
  
  // Finally draw the lines
  graphics_draw_line(ctx, center, minute_point);
  graphics_draw_line(ctx, center, hour_point);
}

// ── Canvas draw callback ──────────────────────────────────────────────────────
static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  // Current time
  time_t now = time(NULL);
  struct tm* t = localtime(&now);

  // ── Background ──
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // ── Outer ring ──
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, center, CLOCK_RADIUS);

  // ── Tick marks ──
  for (int i = 0; i < 60; i++) {
    int32_t angle    = TRIG_MAX_ANGLE * i / 60;
    bool    is_hour  = (i % 5 == 0);
    int     outer_r  = CLOCK_RADIUS - 2;
    int     inner_r  = is_hour ? CLOCK_RADIUS - 14 : CLOCK_RADIUS - 7;

    GPoint outer = point_on_circle(center, angle, outer_r);
    GPoint inner = point_on_circle(center, angle, inner_r);

    graphics_context_set_stroke_color(ctx, is_hour ? GColorWhite : GColorDarkGray);
    graphics_context_set_stroke_width(ctx, is_hour ? 3 : 1);
    graphics_draw_line(ctx, inner, outer);
  }

  graphics_context_set_antialiased(ctx, true);
  
  s_hour_angle = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 60) + t->tm_min) / (12 * 60);
  s_minute_angle = TRIG_MAX_ANGLE * (t->tm_min * 60 + t->tm_sec) / (60 * 60);
  #ifdef FASTMODE
  s_hour_angle = s_minute_angle;
  s_minute_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  #endif
  
  graphics_context_set_stroke_color(ctx, GColorWhite);
  draw_hands_recursive(ctx, center, 0, MINUTE_HAND_LENGTH, 0);
}

// --- Ticks --- //
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Redraw the clock hands every second
  layer_mark_dirty(s_canvas_layer);

  // Update the date label once a minute (or on first load)
  if (units_changed & MINUTE_UNIT || units_changed & DAY_UNIT) {
    static char date_buf[16];
    strftime(date_buf, sizeof(date_buf), "%a %d", tick_time);
    text_layer_set_text(s_date_layer, date_buf);
  }
}

// --- Window --- //
static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect  bounds = layer_get_bounds(root);

  // Canvas fills the whole window
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root, s_canvas_layer);

  // Date label — positioned at the 3 o'clock area (right of center)
  // Pebble Time 2 center is (100, 114); 3 o'clock sits ~ x=148
  GRect date_rect = GRect(136, 104, 52, 20);
  s_date_layer = text_layer_create(date_rect);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorLightGray);
  text_layer_set_font(s_date_layer,
    fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(s_canvas_layer, text_layer_get_layer(s_date_layer));

  // Trigger an immediate date update so the label isn't blank on launch
  time_t     now = time(NULL);
  struct tm *t   = localtime(&now);
  static char date_buf[16];
  strftime(date_buf, sizeof(date_buf), "%a %d", t);
  text_layer_set_text(s_date_layer, date_buf);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  text_layer_destroy(s_date_layer);
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