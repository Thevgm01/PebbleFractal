#pragma once
#include <pebble.h>

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#define is_even(a) (a % 2 == 0)
#define is_odd(a) (a % 2 == 1)

#define APP_LOG_GRECT(log_level, string, rect) APP_LOG( \
  log_level, \
  "%sGRect(x:%d, y:%d, w:%d, h:%d)", \
  string, \
  rect.origin.x, \
  rect.origin.y, \
  rect.size.w, \
  rect.size.h)

#define APP_LOG_BINARY(log_level, string, int) APP_LOG( \
  log_level, \
  "%s%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", \
  string, \
  int & 0x8000 ? '1' : '0', \
  int & 0x4000 ? '1' : '0', \
  int & 0x2000 ? '1' : '0', \
  int & 0x1000 ? '1' : '0', \
  int & 0x0800 ? '1' : '0', \
  int & 0x0400 ? '1' : '0', \
  int & 0x0200 ? '1' : '0', \
  int & 0x0100 ? '1' : '0', \
  int & 0x0080 ? '1' : '0', \
  int & 0x0040 ? '1' : '0', \
  int & 0x0020 ? '1' : '0', \
  int & 0x0010 ? '1' : '0', \
  int & 0x0008 ? '1' : '0', \
  int & 0x0004 ? '1' : '0', \
  int & 0x0002 ? '1' : '0', \
  int & 0x0001 ? '1' : '0')
