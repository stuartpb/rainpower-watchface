#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_hour_layer;
static TextLayer *s_min_layer;
static TextLayer *s_date_layer;
static Layer *s_colon_layer;
static GFont s_time_font;
static GFont s_date_font;
static int s_time_is_pm = 2;

static const char* weekdays[] = {
  "SU", "MO", "TU", "WE", "TH", "FR", "SA"};

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_hour_buffer[3];
  static char s_min_buffer[3];
  static char s_date_buffer[15];

  strftime(s_hour_buffer, sizeof(s_hour_buffer),
    clock_is_24h_style() ? "%H" : "%I", tick_time);
  strftime(s_min_buffer, sizeof(s_min_buffer), "%M", tick_time);
  text_layer_set_text(s_hour_layer, s_hour_buffer);
  text_layer_set_text(s_min_layer, s_min_buffer);

  if (!clock_is_24h_style()) {
    int is_pm = tick_time->tm_hour > 11;
    if (s_time_is_pm != is_pm) {
      s_time_is_pm = is_pm;
      layer_mark_dirty(s_colon_layer);
    }
  } else if (s_time_is_pm != 2) {
    s_time_is_pm = 2;
    layer_mark_dirty(s_colon_layer);
  }

  snprintf(s_date_buffer, sizeof(s_date_buffer), "%s %02i/%02i.%04i",
    weekdays[tick_time->tm_wday],
    tick_time->tm_mon + 1, tick_time->tm_mday,
    tick_time->tm_year + 1900);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init_watch_text_layer(TextLayer *text_layer) {
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_color(text_layer, GColorWhite);
}

static void colon_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0,0,8,10), 1, GCornersAll);
  graphics_fill_rect(ctx, GRect(0,bounds.size.h-10,8,10), 1, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorBlack);
  if (s_time_is_pm == 0) {
    graphics_draw_text(ctx,"A",fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
      GRect(1,1,6,8),
      GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
  } else if (s_time_is_pm == 1) {
    graphics_draw_text(ctx,"P",fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
      GRect(1,bounds.size.h-9,6,8),
      GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
  }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create TextLayers with specific bounds
  s_hour_layer = text_layer_create(
    GRect(0, bounds.size.h/2-30,
      bounds.size.w/2-8, 50));
  s_min_layer = text_layer_create(
    GRect(bounds.size.w/2+16, bounds.size.h/2-30,
      bounds.size.w/2-8, 50));
  s_colon_layer = layer_create(
    GRect(bounds.size.w/2-4, bounds.size.h/2-30, 8, 50));
  layer_set_update_proc(s_colon_layer, colon_update_proc);
  s_date_layer = text_layer_create(
    GRect(0, bounds.size.h/2+30, bounds.size.w, 25));

  // Create GFonts
  s_time_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_GIDOLE_48));
  s_date_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_GIDOLE_20));

  // Apply to TextLayer
  text_layer_set_font(s_hour_layer, s_time_font);
  text_layer_set_font(s_min_layer, s_time_font);
  text_layer_set_font(s_date_layer, s_date_font);

  init_watch_text_layer(s_hour_layer);
  init_watch_text_layer(s_min_layer);
  init_watch_text_layer(s_date_layer);
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_min_layer, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_min_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, s_colon_layer);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_min_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_colon_layer);
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

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
