#pragma once
#include <pebble.h>

#define min(temp_a, temp_b) (temp_a < temp_b ? temp_a : temp_b)
#define max(temp_a, temp_b) (temp_a > temp_b ? temp_a : temp_b)

#define gpoint_shift(temp_point, temp_x, temp_y) (GPoint(temp_point.x + temp_x, temp_point.y + temp_y))

#define is_even(temp_a) (temp_a % 2 == 0)
#define is_odd(temp_a) (temp_a % 2 == 1)

#define center_in_rect(temp_size, temp_rect) GPoint( \
  temp_rect.origin.x + (temp_rect.size.w - temp_size.w) / 2, \
  temp_rect.origin.y + (temp_rect.size.h - temp_size.h) / 2)

#define APP_LOG_GRECT(temp_log_level, temp_string, temp_rect) APP_LOG( \
  temp_log_level, \
  "%sGRect(x:%d, y:%d, w:%d, h:%d)", \
  temp_string, \
  temp_rect.origin.x, \
  temp_rect.origin.y, \
  temp_rect.size.w, \
  temp_rect.size.h)

#define APP_LOG_BINARY(temp_log_level, temp_string, temp_int) APP_LOG( \
  temp_log_level, \
  "%s%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", \
  temp_string, \
  temp_int & 0x8000 ? '1' : '0', \
  temp_int & 0x4000 ? '1' : '0', \
  temp_int & 0x2000 ? '1' : '0', \
  temp_int & 0x1000 ? '1' : '0', \
  temp_int & 0x0800 ? '1' : '0', \
  temp_int & 0x0400 ? '1' : '0', \
  temp_int & 0x0200 ? '1' : '0', \
  temp_int & 0x0100 ? '1' : '0', \
  temp_int & 0x0080 ? '1' : '0', \
  temp_int & 0x0040 ? '1' : '0', \
  temp_int & 0x0020 ? '1' : '0', \
  temp_int & 0x0010 ? '1' : '0', \
  temp_int & 0x0008 ? '1' : '0', \
  temp_int & 0x0004 ? '1' : '0', \
  temp_int & 0x0002 ? '1' : '0', \
  temp_int & 0x0001 ? '1' : '0')
