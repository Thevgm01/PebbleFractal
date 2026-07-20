#include <pebble.h>

#define SETTINGS_KEY 1

typedef struct ClaySettings {
  GColor PrimaryColor;
  GColor SecondaryColor;
  GColor BackgroundColor;
  bool ShowDate;
  int16_t MinuteHandLength;
  int16_t HourHandLength;
  int16_t RecurseScale;
  int16_t WidthScale;
  int16_t FirstHandScale;
  int16_t FontSize;
}

static ClaySettings settings;

static void settings_restore_default() {
  settings.PrimaryColor = GColorWhite;
  settings.SecondaryColor = GColorDarkGray;
  settings.BackgroundColor = GColorBlack;
  settings.ShowDate = true;
  settings.MinuteHandLength = 60;
  settings.HourHandLength = 40;
  settings.RecurseScale = 18;
  settings.WidthScale = 0;
  settings.FirstHandScale = 1;
  settings.FontSize = 14;
}

static void settings_save() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void settings_load() {
  settings_restore_default();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}