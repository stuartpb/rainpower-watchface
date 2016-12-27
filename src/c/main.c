#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static GFont s_time_font;
static GFont s_date_font;

static const char* weekdays[] = {
  "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_time_buffer[6];
  static char s_date_buffer[15];

  if (clock_is_24h_style()) {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  } else {
    int mhour = tick_time->tm_hour % 12;
    snprintf(s_time_buffer, sizeof(s_time_buffer), "%c%c:%02i",
      tick_time->tm_hour < 12 ? 'A' : 'P',
      mhour < 10 ? '0' + mhour : mhour == 10 ? 'X' : 'E',
      tick_time->tm_min);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);

  snprintf(s_date_buffer, sizeof(s_date_buffer), "%s %02i/%02i.%04i",
    weekdays[tick_time->tm_wday],
    tick_time->tm_mon, tick_time->tm_mday,
    tick_time->tm_year + 1900);

  // Display this time on the TextLayer
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init_watch_text_layer(TextLayer *text_layer) {
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create TextLayers with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  s_date_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(110, 105), bounds.size.w, 25));

  // Create GFonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NOVA_MONO_48));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NOVA_MONO_20));

  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_font(s_date_layer, s_date_font);

  init_watch_text_layer(s_time_layer);
  init_watch_text_layer(s_date_layer);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
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
