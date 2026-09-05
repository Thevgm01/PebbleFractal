#include <pebble.h>
#include "utility.h"

#define SETTINGS_KEY 1

typedef struct {
  GColor PrimaryColor;
  GColor SecondaryColor;
  GColor TertiaryColor;
  GColor BackgroundColor;
  bool ShowDate;
  int16_t MinuteHandLength;
  int16_t HourHandLength;
  int16_t RecurseScale;
  int16_t WidthScale;
  int16_t FirstHandScale;
  int16_t Font;
  bool DebugGrid;
  bool DebugSpeed;
} ClaySettings;

static ClaySettings settings;
static void (*settings_loaded_callback)();

static void settings_restore_default() {
  settings.PrimaryColor = GColorWhite;
  settings.SecondaryColor = GColorLightGray;
  settings.TertiaryColor = GColorDarkGray;
  settings.BackgroundColor = GColorBlack;
  settings.ShowDate = true;
  settings.MinuteHandLength = 40;
  settings.HourHandLength = 30;
  settings.RecurseScale = 85;
  settings.WidthScale = 0;
  settings.FirstHandScale = 1;
  settings.Font = 18;
  settings.DebugGrid = false;
  settings.DebugSpeed = false;
}

static void settings_save() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void settings_load() {
  settings_restore_default();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  settings_loaded_callback();
}

static void settings_inbox_received_callback(DictionaryIterator *iterator, void *ctx) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Settings changed, reading...");
  
  Tuple *t;
  
  #define LOAD(key) t = dict_find(iterator, key)
  #define LOAD_COLOR(var, key) { LOAD(key); if (t) (var) = (GColorFromHEX(t->value->int32)); }
  #define LOAD_INT(var, key) { LOAD(key); if (t) (var) = (t->value->int32); }
  #define LOAD_BOOL(var, key) { LOAD(key); if (t) (var) = (t->value->int32 == 1); }
  LOAD_COLOR(settings.PrimaryColor, MESSAGE_KEY_PrimaryColor);
  LOAD_COLOR(settings.SecondaryColor, MESSAGE_KEY_SecondaryColor);
  LOAD_COLOR(settings.TertiaryColor, MESSAGE_KEY_TertiaryColor);
  LOAD_COLOR(settings.BackgroundColor, MESSAGE_KEY_BackgroundColor);
  LOAD_BOOL(settings.ShowDate, MESSAGE_KEY_ShowDate);
  LOAD_INT(settings.MinuteHandLength, MESSAGE_KEY_MinuteHandLength);
  LOAD_INT(settings.HourHandLength, MESSAGE_KEY_HourHandLength);
  LOAD_INT(settings.RecurseScale, MESSAGE_KEY_RecurseScale);
  LOAD_INT(settings.WidthScale, MESSAGE_KEY_WidthScale);
  LOAD_INT(settings.FirstHandScale, MESSAGE_KEY_FirstHandScale);
  LOAD_BOOL(settings.DebugGrid, MESSAGE_KEY_DebugGrid);
  LOAD_BOOL(settings.DebugSpeed, MESSAGE_KEY_DebugSpeed);
  #undef LOAD
  #undef LOAD_COLOR
  #undef LOAD_INT
  #undef LOAD_BOOL
  
  // The "select" type in Clay always returns a string, so we have to convert it to an int
  t = dict_find(iterator, MESSAGE_KEY_Font);
  if (t) settings.Font = atoi(dict_find(iterator, MESSAGE_KEY_Font)->value->cstring);
  
  // During recursion, the hour hand's length is calculated as a ratio of the minute hand's length
  // Thus if the hour hand is longer than the minute hand, that ratio will be greater than 1
  // If that, times the recursion ratio, is 1 or greater, then the fractal ends up growing instead of shrinking
  // So, we clamp the hour hand so it can't ever be bigger than the minute hand
  
  // Perhaps a better solution would swap the recursion multiplication order if the sizes are inverted
  // Then again, what twisted soul is going to make the hour hand longer than the minute hand?
  settings.HourHandLength = min(settings.MinuteHandLength, settings.HourHandLength);

  settings_save();
  
  if (settings_loaded_callback) settings_loaded_callback();
}
