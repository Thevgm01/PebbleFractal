#include <pebble.h>
#include "utility.h"

#define SETTINGS_KEY 1

typedef struct {
  GColor PrimaryColor;
  GColor SecondaryColor;
  GColor TertiaryColor;
  GColor BackgroundColor;
  bool ShowDate;
  bool ShowGizmos;
  bool NotchSquircle;
  int16_t MinuteHandLength;
  int16_t HourHandLength;
  int16_t RecurseScale;
  int8_t Font;
  int8_t NotchInset;
  int8_t PrimaryHandWidth;
  int8_t HourMarkers;
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
  settings.ShowGizmos = false;
  settings.NotchSquircle = true;
  settings.MinuteHandLength = 40;
  settings.HourHandLength = 30;
  settings.RecurseScale = 85;
  settings.Font = 18;
  settings.HourMarkers = 0;
  settings.NotchInset = 10;
  settings.PrimaryHandWidth = 5;
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
  
  #define LOAD_COLOR(var, key) { t = dict_find(iterator, key); if (t) (var) = (GColorFromHEX(t->value->int32)); }
  #define LOAD_INT(var, key) { t = dict_find(iterator, key); if (t) (var) = (t->value->int32); }
  #define LOAD_BOOL(var, key) { t = dict_find(iterator, key); if (t) (var) = (t->value->int32 == 1); }
  #define LOAD_SELECT(var, key) { t = dict_find(iterator, key); if (t) (var) = atoi(t->value->cstring); }
  
  LOAD_COLOR(settings.PrimaryColor, MESSAGE_KEY_PrimaryColor);
  LOAD_COLOR(settings.SecondaryColor, MESSAGE_KEY_SecondaryColor);
  LOAD_COLOR(settings.TertiaryColor, MESSAGE_KEY_TertiaryColor);
  LOAD_COLOR(settings.BackgroundColor, MESSAGE_KEY_BackgroundColor);
  LOAD_BOOL(settings.ShowDate, MESSAGE_KEY_ShowDate);
  LOAD_BOOL(settings.ShowGizmos, MESSAGE_KEY_ShowGizmos);
  LOAD_BOOL(settings.NotchSquircle, MESSAGE_KEY_NotchSquircle);
  LOAD_INT(settings.MinuteHandLength, MESSAGE_KEY_MinuteHandLength);
  LOAD_INT(settings.HourHandLength, MESSAGE_KEY_HourHandLength);
  LOAD_INT(settings.RecurseScale, MESSAGE_KEY_RecurseScale);
  LOAD_INT(settings.NotchInset, MESSAGE_KEY_NotchInset);
  LOAD_INT(settings.PrimaryHandWidth, MESSAGE_KEY_PrimaryHandWidth);
  LOAD_SELECT(settings.Font, MESSAGE_KEY_Font);
  LOAD_SELECT(settings.HourMarkers, MESSAGE_KEY_HourMarkers);
  LOAD_BOOL(settings.DebugGrid, MESSAGE_KEY_DebugGrid);
  LOAD_BOOL(settings.DebugSpeed, MESSAGE_KEY_DebugSpeed);
  
  #undef LOAD_COLOR
  #undef LOAD_INT
  #undef LOAD_BOOL
  #undef LOAD_SELECT

  if (settings_loaded_callback) settings_loaded_callback();

  settings_save();
}
