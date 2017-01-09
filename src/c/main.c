#include <pebble.h>

static Window *s_main_window;

// you need this level of indirection for metamacros
// https://stackoverflow.com/a/41395465/34799
#define APPLY_MACRO(x, t) x(t)

#define MAIN_WINDOW_TEXT_LAYERS_METAMACRO(X, tr) \
  APPLY_MACRO(X,tr(hour_layer)) \
  APPLY_MACRO(X,tr(min_layer)) \
  APPLY_MACRO(X,tr(date_layer)) \
  APPLY_MACRO(X,tr(temperature_layer))

#define MAIN_WINDOW_BITMAP_LAYERS_METAMACRO(X, tr) \
  APPLY_MACRO(X,tr(degree_symbol_layer))

#define MAIN_WINDOW_LAYERS_METAMACRO(X, tr) \
  APPLY_MACRO(X,tr(colon_layer)) \
  APPLY_MACRO(X,tr(phone_batt_layer)) \
  APPLY_MACRO(X,tr(watch_batt_layer)) \
  APPLY_MACRO(X,tr(precip_chance_layer))

#define GBITMAPS_WITH_RESOURCE_IDS_METAMACRO(X, tr) \
  APPLY_MACRO(X,tr(watch_icon, ICON_WATCH_6X11)) \
  APPLY_MACRO(X,tr(watch_charging_icon, ICON_WATCH_CHARGING_6X11)) \
  APPLY_MACRO(X,tr(phone_icon, ICON_PHONE_6X11)) \
  APPLY_MACRO(X,tr(phone_charging_icon, ICON_PHONE_CHARGING_6X11)) \
  APPLY_MACRO(X,tr(phone_disconnected_icon, ICON_PHONE_DISCONNECTED_6X11)) \
  APPLY_MACRO(X,tr(degrees_c_bitmap, DEGREES_C_12X27)) \
  APPLY_MACRO(X,tr(degrees_f_bitmap, DEGREES_F_12X27))

#define GFONTS_WITH_RESOURCE_IDS_METAMACRO(X, tr) \
  APPLY_MACRO(X,tr(time_12h_font, FONT_DIGITS_UBUNTU_MONO_BOLD_80)) \
  APPLY_MACRO(X,tr(time_24h_font, FONT_DIGITS_UBUNTU_MONO_BOLD_60)) \
  APPLY_MACRO(X,tr(date_font, FONT_DATE_UBUNTU_MONO_BOLD_20)) \
  APPLY_MACRO(X,tr(temperature_font, FONT_DIGITS_UBUNTU_MONO_BOLD_40))

#define IDENTITY_MACRO(x) x
#define STATIC_PREFIX_MACRO(x) s_ ## x
#define STATIC_PREFIX_DISCARD_MACRO(x, _) s_ ## x
#define STATIC_PREFIX_RESOURCE_ID_PREFIX_MACRO(x, id) \
  s_ ## x, RESOURCE_ID_ ## id
#define STATIC_PREFIX_NO_PREFIX_MACRO(x, y) \
  s_ ## x, y

#define FOR_MAIN_WINDOW_STATIC_TEXT_LAYER_POINTERS(macro) \
  MAIN_WINDOW_TEXT_LAYERS_METAMACRO(macro, STATIC_PREFIX_MACRO)

#define FOR_MAIN_WINDOW_STATIC_BITMAP_LAYER_POINTERS(macro) \
  MAIN_WINDOW_BITMAP_LAYERS_METAMACRO(macro, STATIC_PREFIX_MACRO)

#define FOR_MAIN_WINDOW_STATIC_LAYER_POINTERS(macro) \
  MAIN_WINDOW_LAYERS_METAMACRO(macro, STATIC_PREFIX_MACRO)

#define FOR_MAIN_WINDOW_LAYER_NAMES(macro) \
  MAIN_WINDOW_LAYERS_METAMACRO(macro, IDENTITY_MACRO)

#define FOR_STATIC_GFONTS(macro) \
  GFONTS_WITH_RESOURCE_IDS_METAMACRO(macro, STATIC_PREFIX_DISCARD_MACRO)

#define FOR_STATIC_GFONTS_WITH_RESOURCE_IDS(macro) \
  GFONTS_WITH_RESOURCE_IDS_METAMACRO(macro, \
    STATIC_PREFIX_RESOURCE_ID_PREFIX_MACRO)

#define FOR_STATIC_GBITMAP_POINTERS_WITH_RESOURCE_IDS(macro) \
  GBITMAPS_WITH_RESOURCE_IDS_METAMACRO(macro, \
    STATIC_PREFIX_RESOURCE_ID_PREFIX_MACRO)

#define FOR_STATIC_GBITMAP_POINTERS(macro) \
  GBITMAPS_WITH_RESOURCE_IDS_METAMACRO(macro, STATIC_PREFIX_DISCARD_MACRO)

#define X(name) static TextLayer *name;
FOR_MAIN_WINDOW_STATIC_TEXT_LAYER_POINTERS(X)
#undef X

#define X(name) static BitmapLayer *name;
FOR_MAIN_WINDOW_STATIC_BITMAP_LAYER_POINTERS(X)
#undef X

#define X(name) static Layer *name;
FOR_MAIN_WINDOW_STATIC_LAYER_POINTERS(X)
#undef X

#define X(name) static GFont name;
FOR_STATIC_GFONTS(X)
#undef X

#define X(name) static GBitmap *name;
FOR_STATIC_GBITMAP_POINTERS(X)
#undef X

static GPoint s_precip_points[112];
static GPoint* s_precip_60m_points = &s_precip_points[2];
static GPoint* s_precip_48h_points = &s_precip_points[62];
static const GPathInfo PRECIP_60M_PATHINFO = {
  .num_points = 60,
  .points = &s_precip_points[2]
};
static const GPathInfo PRECIP_60M_FILLED_PATHINFO = {
  .num_points = 62,
  .points = &s_precip_points[0]
};
static const GPathInfo PRECIP_48H_PATHINFO = {
  .num_points = 48,
  .points = &s_precip_points[62]
};
static const GPathInfo PRECIP_48H_FILLED_PATHINFO = {
  .num_points = 50,
  .points = &s_precip_points[62]
};
static const GPathInfo PRECIP_BOTH_PATHINFO = {
  .num_points = 108,
  .points = &s_precip_points[2]
};
static const GPathInfo PRECIP_BOTH_FILLED_PATHINFO = {
  .num_points = 110,
  .points = &s_precip_points[1]
};

#define GPATHS_WITH_INFO_METAMACRO(X, tr) \
  APPLY_MACRO(X,tr(precip_60m_path, PRECIP_60M_FILLED_PATHINFO)) \
  APPLY_MACRO(X,tr(precip_48h_path, PRECIP_48H_FILLED_PATHINFO))

#define FOR_STATIC_GPATH_POINTERS_WITH_INFO(macro) \
  GPATHS_WITH_INFO_METAMACRO(macro, STATIC_PREFIX_NO_PREFIX_MACRO)

#define FOR_STATIC_GPATH_POINTERS(macro) \
  GPATHS_WITH_INFO_METAMACRO(macro, STATIC_PREFIX_DISCARD_MACRO)

#define X(name) static GPath *name;
FOR_STATIC_GPATH_POINTERS(X)
#undef X

const int CLOCK_HEIGHT = 84;
const int CLOCK_TOP_SHIFT = 12;
const int COLON_WIDTH = 10;
const int COLON_12H_HEIGHT = 38;
const int COLON_24H_HEIGHT = 38;
const int COLON_DOT_HEIGHT = 13;
const int COLON_MARGIN = 4;
const int COLON_12H_SHIFT = -20;
const int COLON_TOP_SHIFT = 36;
const int WEATHER_HEIGHT = 40;
const int TEMPERATURE_TOP_SHIFT = -7;
const int TEMPERATURE_WIDTH = 45;
const int PRECIP_60M_WIDTH = 60;

static int s_time_is_pm = 2;
static int s_watch_batt_level = 0;
static int s_watch_batt_charging = 0;
static int s_phone_batt_level = 0;
static int s_phone_batt_charging = 0;
static int s_phone_connected = 0;
static int s_current_temperature = 0;
static int s_current_temperature_is_f = 0;

static uint8_t s_precip_48h[48];
static uint8_t s_precip_60m[60];

static const char* weekdays[] = {
  "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

static int colon_left_position(int winwidth) {
  int left = winwidth/2 - COLON_WIDTH/2;
  if (!clock_is_24h_style()) {
    left += COLON_12H_SHIFT;
  }
  return left;
}

static int clock_top_position(int winheight) {
  return winheight / 2 - CLOCK_HEIGHT + CLOCK_TOP_SHIFT;
}

static GRect hour_layer_frame(int winwidth, int winheight) {
  return GRect(
    0,
    clock_top_position(winheight),
    colon_left_position(winwidth) - COLON_MARGIN,
    CLOCK_HEIGHT);
}
static GRect min_layer_frame(int winwidth, int winheight) {
  int left = colon_left_position(winwidth) + COLON_WIDTH + COLON_MARGIN;
  return GRect(
    left,
    clock_top_position(winheight),
    winwidth - left,
    CLOCK_HEIGHT);
}
static GRect colon_layer_frame(int winwidth, int winheight) {
  return GRect(
    colon_left_position(winwidth),
    clock_top_position(winheight) + COLON_TOP_SHIFT,
    COLON_WIDTH,
    clock_is_24h_style() ? COLON_24H_HEIGHT : COLON_12H_HEIGHT);
}

static void update_clock_position() {
  GRect bounds = layer_get_bounds(window_get_root_layer(s_main_window));
  int winwidth = bounds.size.w;
  int winheight = bounds.size.h;
  layer_set_frame(s_colon_layer,
    colon_layer_frame(winwidth, winheight));
  layer_set_frame(text_layer_get_layer(s_hour_layer),
    hour_layer_frame(winwidth, winheight));
  layer_set_frame(text_layer_get_layer(s_min_layer),
    min_layer_frame(winwidth, winheight));
  text_layer_set_font(s_hour_layer,
    clock_is_24h_style() ? s_time_24h_font : s_time_12h_font);
  text_layer_set_font(s_min_layer,
    clock_is_24h_style() ? s_time_24h_font : s_time_12h_font);
}

static void update_weather() {
  static char s_temperature_buffer[5];

  // update precip chance point positions
  for (int i = 0; i < 60; i++) {
    s_precip_60m_points[i].y = WEATHER_HEIGHT -
      s_precip_60m[i] * WEATHER_HEIGHT / 100;
  }
  for (int i = 0; i < 48; i++) {
    s_precip_48h_points[i].y = WEATHER_HEIGHT -
      s_precip_48h[i] * WEATHER_HEIGHT / 100;
  }

  layer_mark_dirty(s_precip_chance_layer);

  snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "%i",
    s_current_temperature);
  text_layer_set_text(s_temperature_layer, s_temperature_buffer);
  bitmap_layer_set_bitmap(s_degree_symbol_layer,
    s_current_temperature_is_f ? s_degrees_f_bitmap : s_degrees_c_bitmap);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_hour_buffer[3];
  static char s_min_buffer[3];
  static char s_date_buffer[15];

  int hour = tick_time->tm_hour;

  if (!clock_is_24h_style()) {
    int is_pm = hour > 11;
    hour = hour % 12;
    if (!hour) hour = 12;
    if (s_time_is_pm != is_pm) {
      if (s_time_is_pm == 2) {
        update_clock_position();
      }
      s_time_is_pm = is_pm;
      layer_mark_dirty(s_colon_layer);
    }
  } else if (s_time_is_pm != 2) {
    update_clock_position();
    s_time_is_pm = 2;
    layer_mark_dirty(s_colon_layer);
  }

  if (clock_is_24h_style()) {
    snprintf(s_hour_buffer, sizeof(s_hour_buffer), "%02i", hour);
  } else {
    int mhour = tick_time->tm_hour % 12;
    snprintf(s_hour_buffer, sizeof(s_hour_buffer), "%c",
      mhour < 10 ? mhour ? '0' + mhour : 'D' : mhour == 10 ? 'X' : 'E');
  }
  strftime(s_min_buffer, sizeof(s_min_buffer), "%M", tick_time);
  text_layer_set_text(s_hour_layer, s_hour_buffer);
  text_layer_set_text(s_min_layer, s_min_buffer);

  snprintf(s_date_buffer, sizeof(s_date_buffer), "%s %04i-%02i-%02i",
    weekdays[tick_time->tm_wday],
    tick_time->tm_year + 1900, tick_time->tm_mon + 1, tick_time->tm_mday);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void watch_battery_state_callback(BatteryChargeState state) {
  // Record the new battery level
  s_watch_batt_level = state.charge_percent;
  s_watch_batt_charging = state.is_plugged;
  layer_mark_dirty(s_watch_batt_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read tuples for data
  Tuple *phone_batt_level_tuple = dict_find(
    iterator, MESSAGE_KEY_PHONE_BATT_LEVEL);
  Tuple *phone_batt_charging_tuple = dict_find(
    iterator, MESSAGE_KEY_PHONE_BATT_CHARGING);
  Tuple *current_temperature_tuple = dict_find(
    iterator, MESSAGE_KEY_CURRENT_TEMPERATURE);
  Tuple *current_temperature_is_f_tuple = dict_find(
    iterator, MESSAGE_KEY_CURRENT_TEMPERATURE_IS_FAHRENHEIT);
  Tuple *precip_chance_60m_tuple = dict_find(
    iterator, MESSAGE_KEY_PRECIP_PROBABILITY_NEXT_60_MINUTES);
  Tuple *precip_chance_48h_tuple = dict_find(
    iterator, MESSAGE_KEY_PRECIP_PROBABILITY_NEXT_48_HOURS);

  if (phone_batt_level_tuple) {
    s_phone_batt_level = phone_batt_level_tuple->value->int32;
  }
  if (phone_batt_charging_tuple) {
    s_phone_batt_charging = phone_batt_charging_tuple->value->int32;
  }
  if (phone_batt_charging_tuple || phone_batt_level_tuple) {
    layer_mark_dirty(s_phone_batt_layer);
  }

  if (current_temperature_tuple) {
    s_current_temperature = current_temperature_tuple->value->int32;
  }
  if (current_temperature_is_f_tuple) {
    s_current_temperature_is_f = current_temperature_is_f_tuple->value->int32;
  }
  if (precip_chance_60m_tuple) {
    memcpy(s_precip_60m, precip_chance_60m_tuple->value, sizeof(s_precip_60m));
  }
  if (precip_chance_48h_tuple) {
    memcpy(s_precip_48h, precip_chance_48h_tuple->value, sizeof(s_precip_48h));
  }
  if (current_temperature_tuple) { // I could &&-conditional the others but meh
    update_weather();
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed, code %i", reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void bluetooth_callback(bool connected) {
  s_phone_connected = connected;
  layer_mark_dirty(s_phone_batt_layer);
}

static void colon_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx,
    GRect(0, 0, COLON_WIDTH, COLON_DOT_HEIGHT),
    1, GCornersAll);
  graphics_fill_rect(ctx,
    GRect(0, bounds.size.h - COLON_DOT_HEIGHT, COLON_WIDTH, COLON_DOT_HEIGHT),
    1, GCornersAll);
  graphics_context_set_text_color(ctx, GColorBlack);
  if (s_time_is_pm == 0) {
    graphics_draw_text(ctx,"A",fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
      GRect(0, -3, 10, 12),
      GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
  } else if (s_time_is_pm == 1) {
    graphics_draw_text(ctx,"P",fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
      GRect(0, bounds.size.h - COLON_DOT_HEIGHT - 3, 10, 12),
      GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
  }
}

static void phone_batt_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int bar_width = s_phone_batt_level * (bounds.size.w - 8) / 100;
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_bitmap_in_rect(ctx,
    s_phone_connected
      ? s_phone_batt_charging ? s_phone_charging_icon : s_phone_icon
      : s_phone_disconnected_icon,
    GRect(0,0,6,11));
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, GRect(8,1,bounds.size.w-8,9), 0, GCornerNone);
  graphics_context_set_fill_color(ctx,
    s_phone_connected ? GColorGreen : GColorMagenta);
  graphics_fill_rect(ctx, GRect(8,1,bar_width,9), 0, GCornerNone);
}

static void watch_batt_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int bar_width = s_watch_batt_level * (bounds.size.w - 8) / 100;
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_bitmap_in_rect(ctx,
    s_watch_batt_charging ? s_watch_charging_icon : s_watch_icon,
    GRect(0,0,6,11));
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, GRect(8,1,bounds.size.w-8,9), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorCyan);
  graphics_fill_rect(ctx, GRect(8,1,bar_width,9), 0, GCornerNone);
}

static void precip_chance_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint midway_48h = GPoint(
    PRECIP_60M_WIDTH + ((bounds.size.w - PRECIP_60M_WIDTH) / 2),
    (s_precip_48h_points[23].y + s_precip_48h_points[24].y) / 2
  );

  // Draw backdrop
  graphics_context_set_fill_color(ctx, GColorOxfordBlue);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw graph
  graphics_context_set_fill_color(ctx, GColorDukeBlue);
  gpath_draw_filled(ctx, s_precip_60m_path);
  graphics_context_set_fill_color(ctx, GColorBlue);
  gpath_draw_filled(ctx, s_precip_48h_path);

  // Draw interval lines
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, GColorDukeBlue);
  graphics_draw_line(ctx, GPoint(PRECIP_60M_WIDTH, 0),
    GPoint(PRECIP_60M_WIDTH, s_precip_48h_points[0].y - 1));
  graphics_draw_line(ctx, GPoint(midway_48h.x, 0),
    GPoint(midway_48h.x, midway_48h.y - 1));
  graphics_context_set_stroke_color(ctx, GColorBlueMoon);
  graphics_draw_line(ctx, GPoint(PRECIP_60M_WIDTH, s_precip_48h_points[0].y),
    GPoint(PRECIP_60M_WIDTH, WEATHER_HEIGHT));
  graphics_draw_line(ctx, midway_48h, GPoint(midway_48h.x, WEATHER_HEIGHT));
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  int winwidth = bounds.size.w;
  int winheight = bounds.size.h;

  // HACK: This makes it so colon position doesn't have to be re-written
  // when updating the time the first time.
  // A better "was 24h at last check" bool would make this unnecessary:
  // tracking @ https://github.com/stuartpb/rainpower-watchface/issues/6
  if (!clock_is_24h_style()) s_time_is_pm = 3;

  // Create layers
  s_phone_batt_layer = layer_create(GRect(0, 21, bounds.size.w, 11));
  s_watch_batt_layer = layer_create(GRect(0, 8, bounds.size.w, 11));
  s_precip_chance_layer = layer_create(GRect(0, bounds.size.h - WEATHER_HEIGHT,
    bounds.size.w, WEATHER_HEIGHT));

  s_colon_layer = layer_create(colon_layer_frame(winwidth, winheight));
  s_hour_layer = text_layer_create(hour_layer_frame(winwidth, winheight));
  s_min_layer = text_layer_create(min_layer_frame(winwidth, winheight));

  s_date_layer = text_layer_create(
    GRect(0, bounds.size.h/2+10, bounds.size.w, 25));

  s_temperature_layer = text_layer_create(GRect(
    0,
    bounds.size.h - WEATHER_HEIGHT + TEMPERATURE_TOP_SHIFT,
    TEMPERATURE_WIDTH,
    WEATHER_HEIGHT - TEMPERATURE_TOP_SHIFT));

  s_degree_symbol_layer = bitmap_layer_create(
    GRect(TEMPERATURE_WIDTH, bounds.size.h - WEATHER_HEIGHT,
      12, WEATHER_HEIGHT));
  bitmap_layer_set_compositing_mode(s_degree_symbol_layer, GCompOpSet);

  // Set layer update functions
  #define X(name) layer_set_update_proc(s_ ## name, name ## _update_proc);
  FOR_MAIN_WINDOW_LAYER_NAMES(X)
  #undef X

  // Create GFonts
  #define X(name, id) name = fonts_load_custom_font(resource_get_handle(id));
  FOR_STATIC_GFONTS_WITH_RESOURCE_IDS(X)
  #undef X

  // Create GBitmaps
  #define X(name, id) name = gbitmap_create_with_resource(id);
  FOR_STATIC_GBITMAP_POINTERS_WITH_RESOURCE_IDS(X)
  #undef X

  // Create GPaths
  #define X(name, info) name = gpath_create(&info);
  FOR_STATIC_GPATH_POINTERS_WITH_INFO(X)
  #undef X

  // Set colors for text layers
  #define X(text_layer) \
    text_layer_set_background_color(text_layer, GColorClear); \
    text_layer_set_text_color(text_layer, GColorWhite);
  FOR_MAIN_WINDOW_STATIC_TEXT_LAYER_POINTERS(X);
  #undef X

  // Set fonts and alignments for text layers
  text_layer_set_font(s_hour_layer,
    clock_is_24h_style() ? s_time_24h_font : s_time_12h_font);
  text_layer_set_font(s_min_layer,
    clock_is_24h_style() ? s_time_24h_font : s_time_12h_font);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_font(s_temperature_layer, s_temperature_font);
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_min_layer, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentRight);

  // Initialize fixed precipitation graph coordinates
  s_precip_points[0].y = s_precip_points[1].y =
    s_precip_points[110].y = s_precip_points[111].y = WEATHER_HEIGHT;
  s_precip_points[0].x = PRECIP_60M_WIDTH - 1;
  s_precip_points[1].x = 0;
  s_precip_points[110].x = bounds.size.w - 1;
  s_precip_points[111].x = PRECIP_60M_WIDTH;

  int precip_48h_width = (bounds.size.w - PRECIP_60M_WIDTH);

  // Initialize Y and (fixed) X positions
  for (int i = 0; i < 60; i++) {
    s_precip_60m_points[i].x = (PRECIP_60M_WIDTH / 60.0) * i;
    s_precip_60m_points[i].y = WEATHER_HEIGHT;
  }
  for (int i = 0; i < 48; i++) {
    s_precip_48h_points[i].x =
      PRECIP_60M_WIDTH + ((precip_48h_width - 1) / 47.0) * i;
    s_precip_48h_points[i].y = WEATHER_HEIGHT;
  }

  // Add children to main window layer
  #define ADD_MAIN_WINDOW_CHILD(layer) layer_add_child(window_layer, layer);
  #define X ADD_MAIN_WINDOW_CHILD
  FOR_MAIN_WINDOW_STATIC_LAYER_POINTERS(X)
  #undef X
  #define X(layer) ADD_MAIN_WINDOW_CHILD(bitmap_layer_get_layer(layer))
  FOR_MAIN_WINDOW_STATIC_BITMAP_LAYER_POINTERS(X)
  #undef X
  #define X(layer) ADD_MAIN_WINDOW_CHILD(text_layer_get_layer(layer))
  FOR_MAIN_WINDOW_STATIC_TEXT_LAYER_POINTERS(X)
  #undef X
  #undef ADD_MAIN_WINDOW_CHILD
}

static void main_window_unload(Window *window) {
  // Destroy layers
  #define X(layer) layer_destroy(layer);
  FOR_MAIN_WINDOW_STATIC_LAYER_POINTERS(X)
  #undef X
  #define X(layer) bitmap_layer_destroy(layer);
  FOR_MAIN_WINDOW_STATIC_BITMAP_LAYER_POINTERS(X)
  #undef X
  #define X(layer) text_layer_destroy(layer);
  FOR_MAIN_WINDOW_STATIC_TEXT_LAYER_POINTERS(X)
  #undef X

  // Unload fonts, bitmaps, and paths
  #define X(name) fonts_unload_custom_font(name);
  FOR_STATIC_GFONTS(X)
  #undef X
  #define X(name) gbitmap_destroy(name);
  FOR_STATIC_GBITMAP_POINTERS(X)
  #undef X
  #define X(name) gpath_destroy(name);
  FOR_STATIC_GPATH_POINTERS(X)
  #undef X
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set it black
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  const int inbox_size = 256;
  const int outbox_size = 64;
  app_message_open(inbox_size, outbox_size);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());

  // Register for watch battery level updates
  battery_state_service_subscribe(watch_battery_state_callback);

  // Ensure watch battery level is displayed from the start
  watch_battery_state_callback(battery_state_service_peek());

  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
