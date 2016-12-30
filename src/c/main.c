#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_hour_layer;
static TextLayer *s_min_layer;
static TextLayer *s_date_layer;
static Layer *s_colon_layer;
static Layer *s_phone_batt_layer;
static Layer *s_watch_batt_layer;
static GFont s_time_font;
static GFont s_date_font;
static GBitmap *s_watch_icon;
static GBitmap *s_watch_charging_icon;
static GBitmap *s_phone_icon;
static GBitmap *s_phone_charging_icon;
static int s_time_is_pm = 2;
static int s_watch_batt_level = 0;
static int s_watch_batt_charging = 0;
static int s_phone_batt_level = 0;
static int s_phone_batt_charging = 0;

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

  if (phone_batt_level_tuple) {
    s_phone_batt_level = phone_batt_level_tuple->value->int32;
  }
  if (phone_batt_charging_tuple) {
    s_phone_batt_charging = phone_batt_charging_tuple->value->int32;
  }
  if (phone_batt_charging_tuple || phone_batt_level_tuple) {
    layer_mark_dirty(s_phone_batt_layer);
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
  graphics_fill_rect(ctx, GRect(0,0,8,11), 1, GCornersAll);
  graphics_fill_rect(ctx, GRect(0,bounds.size.h-11,8,11), 1, GCornersAll);
  graphics_context_set_text_color(ctx, GColorBlack);
  if (s_time_is_pm == 0) {
    graphics_draw_text(ctx,"A",fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
      GRect(-1,-4,10,12),
      GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
  } else if (s_time_is_pm == 1) {
    graphics_draw_text(ctx,"P",fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
      GRect(-1,bounds.size.h-15,10,12),
      GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
  }
}

static void phone_batt_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int bar_width = s_phone_batt_level * (bounds.size.w - 8) / 100;
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_bitmap_in_rect(ctx,
    s_phone_batt_charging ? s_phone_charging_icon : s_phone_icon,
    GRect(0,0,6,11));
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, GRect(8,1,bounds.size.w-8,9), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorGreen);
  graphics_fill_rect(ctx, GRect(8,1,bar_width,9), 0, GCornerNone);
}

static void watch_batt_update_proc(Layer *layer, GContext *ctx) {
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

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create layers
  s_phone_batt_layer = layer_create(
    GRect(0, 21, bounds.size.w, 11));
  layer_set_update_proc(s_phone_batt_layer, phone_batt_update_proc);
  s_watch_batt_layer = layer_create(
    GRect(0, 8, bounds.size.w, 11));
  layer_set_update_proc(s_watch_batt_layer, watch_batt_update_proc);
  s_hour_layer = text_layer_create(
    GRect(0, bounds.size.h/2-50,
      bounds.size.w/2-8, 50));
  s_min_layer = text_layer_create(
    GRect(bounds.size.w/2+8, bounds.size.h/2-50,
      bounds.size.w/2-8, 50));
  s_colon_layer = layer_create(
    GRect(bounds.size.w/2-4, bounds.size.h/2-36, 8, 34));
  layer_set_update_proc(s_colon_layer, colon_update_proc);
  s_date_layer = text_layer_create(
    GRect(0, bounds.size.h/2+5, bounds.size.w, 25));

  // Create GFonts
  s_time_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_ARVO_BOLD_48));
  s_date_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_ARVO_BOLD_20));

  // Create GBitmaps
  s_watch_icon =
    gbitmap_create_with_resource(RESOURCE_ID_ICON_WATCH_6X11);
  s_watch_charging_icon =
    gbitmap_create_with_resource(RESOURCE_ID_ICON_WATCH_CHARGING_6X11);
  s_phone_icon =
    gbitmap_create_with_resource(RESOURCE_ID_ICON_PHONE_6X11);
  s_phone_charging_icon =
    gbitmap_create_with_resource(RESOURCE_ID_ICON_PHONE_CHARGING_6X11);

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
  layer_add_child(window_layer, s_watch_batt_layer);
  layer_add_child(window_layer, s_phone_batt_layer);
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_min_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, s_colon_layer);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  layer_destroy(s_watch_batt_layer);
  layer_destroy(s_phone_batt_layer);
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

  // Register for watch battery level updates
  battery_state_service_subscribe(watch_battery_state_callback);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Ensure watch battery level is displayed from the start
  watch_battery_state_callback(battery_state_service_peek());

  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  // Unload GFonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  // Destroy GBitmaps
  gbitmap_destroy(s_watch_icon);
  gbitmap_destroy(s_watch_charging_icon);
  gbitmap_destroy(s_phone_icon);
  gbitmap_destroy(s_phone_charging_icon);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
