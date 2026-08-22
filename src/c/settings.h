#include <pebble.h>

#define SETTINGS_KEY 1

typedef struct {
  GColor PrimaryColor;
  GColor SecondaryColor;
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
  settings.SecondaryColor = GColorDarkGray;
  settings.BackgroundColor = GColorBlack;
  settings.ShowDate = true;
  settings.MinuteHandLength = 60;
  settings.HourHandLength = 40;
  settings.RecurseScale = 80;
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
  
  #define LOAD_COLOR(var, key) (var) = (GColorFromHEX(dict_find(iterator, key)->value->int32))
  #define LOAD_INT(var, key) (var) = (dict_find(iterator, key)->value->int32)
  #define LOAD_BOOL(var, key) (var) = (dict_find(iterator, key)->value->int32 == 1)
  LOAD_COLOR(settings.PrimaryColor, MESSAGE_KEY_PrimaryColor);
  LOAD_COLOR(settings.SecondaryColor, MESSAGE_KEY_SecondaryColor);
  LOAD_COLOR(settings.BackgroundColor, MESSAGE_KEY_BackgroundColor);
  LOAD_BOOL(settings.ShowDate, MESSAGE_KEY_ShowDate);
  LOAD_INT(settings.MinuteHandLength, MESSAGE_KEY_MinuteHandLength);
  LOAD_INT(settings.HourHandLength, MESSAGE_KEY_HourHandLength);
  LOAD_INT(settings.RecurseScale, MESSAGE_KEY_RecurseScale);
  LOAD_INT(settings.WidthScale, MESSAGE_KEY_WidthScale);
  LOAD_INT(settings.FirstHandScale, MESSAGE_KEY_FirstHandScale);
  LOAD_BOOL(settings.DebugGrid, MESSAGE_KEY_DebugGrid);
  LOAD_BOOL(settings.DebugSpeed, MESSAGE_KEY_DebugSpeed);
  #undef LOAD_COLOR
  #undef LOAD_INT
  #undef LOAD_BOOL
  settings.Font = atoi(dict_find(iterator, MESSAGE_KEY_Font)->value->cstring);

  settings_save();
  settings_loaded_callback();
}
