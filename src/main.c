#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;
static TextLayer *s_ampm_layer;
static Layer *bluetooth_icon_layer;
static GFont s_time_font;
static GFont s_date_font;
static GFont s_ampm_font;
static bool bluetoothConnected;
static bool bluetoothIconEnabled;

#define KEY_BGCOLOR_R 0
#define KEY_BGCOLOR_G 1
#define KEY_BGCOLOR_B 2
#define KEY_TIME_COLOR_R 3
#define KEY_TIME_COLOR_G 4
#define KEY_TIME_COLOR_B 5
#define KEY_BLUETOOTH_ICON_ENABLED 6

static void bt_update() {
  if (bluetoothIconEnabled) {
    layer_set_hidden(bluetooth_icon_layer, !bluetoothConnected);
  } else {
    layer_set_hidden(bluetooth_icon_layer, true);
  }
}
  
static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  #if PBL_SDK_3 
  int red, green, blue;
  Tuple *color_red_t, *color_green_t, *color_blue_t;
  // Background color?
  color_red_t = dict_find(iter, KEY_BGCOLOR_R);
  color_green_t = dict_find(iter, KEY_BGCOLOR_G);
  color_blue_t = dict_find(iter, KEY_BGCOLOR_B);
  if(color_red_t && color_green_t && color_blue_t) {
    // Apply the color if available
      red = color_red_t->value->int32;
      green = color_green_t->value->int32;
      blue = color_blue_t->value->int32;
  
      // Persist values
      persist_write_int(KEY_BGCOLOR_R, red);
      persist_write_int(KEY_BGCOLOR_G, green);
      persist_write_int(KEY_BGCOLOR_B, blue);
  
      GColor bg_color = GColorFromRGB(red, green, blue);
      window_set_background_color(s_main_window, bg_color);
  }
  // Time color?
  color_red_t = dict_find(iter, KEY_TIME_COLOR_R);
  color_green_t = dict_find(iter, KEY_TIME_COLOR_G);
  color_blue_t = dict_find(iter, KEY_TIME_COLOR_B);
  if(color_red_t && color_green_t && color_blue_t) {
    // Apply the color if available
      red = color_red_t->value->int32;
      green = color_green_t->value->int32;
      blue = color_blue_t->value->int32;
  
      // Persist values
      persist_write_int(KEY_TIME_COLOR_R, red);
      persist_write_int(KEY_TIME_COLOR_G, green);
      persist_write_int(KEY_TIME_COLOR_B, blue);
  
      GColor time_color = GColorFromRGB(red, green, blue);
      text_layer_set_text_color(s_time_layer, time_color); 
  }
  #endif
  // Get bluetooth enabled
  Tuple *bluetooth_icon_enabled_t;
  bluetooth_icon_enabled_t = dict_find(iter, KEY_BLUETOOTH_ICON_ENABLED);
  if (bluetooth_icon_enabled_t) {
    int enabled = bluetooth_icon_enabled_t->value->uint8;
    bluetoothIconEnabled = enabled != 0;
    bt_update();
    persist_write_int(KEY_BLUETOOTH_ICON_ENABLED, enabled);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void bt_handler(bool connected) {
  // Vibrate on disconnect
  if (bluetoothConnected == true && connected == false) {
    vibes_short_pulse();
  }
  bluetoothConnected = connected;
  bt_update();
}

static void bluetooth_icon_update_proc(Layer *this_layer, GContext *ctx) {
  // Draw things here using ctx
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  // X
  graphics_draw_line(ctx, GPoint(0, 10), GPoint(10, 20));
  graphics_draw_line(ctx, GPoint(0, 20), GPoint(10, 10));
  // |
  graphics_draw_line(ctx, GPoint(5, 5), GPoint(5, 25));
  // \ + /
  graphics_draw_line(ctx, GPoint(5, 5), GPoint(10, 10));
  graphics_draw_line(ctx, GPoint(5, 25), GPoint(10, 20));
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char dateBuffer[] = "Jan 01";
  static char dayBuffer[] = "Wednesday";
  static char ampmBuffer[] = "AM";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 24h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
      
    // Write day/evening into buffer
    strftime(ampmBuffer, sizeof("AM"), "%p", tick_time); 
    text_layer_set_text(s_ampm_layer, ampmBuffer);
  }
  
  // Write date into buffer
  strftime(dateBuffer, sizeof("Jan 01"), "%h %d", tick_time);

  // Write day into buffer
  strftime(dayBuffer, sizeof("Wednesday"), "%A", tick_time);  
  
  // Display text
  text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(s_date_layer, dateBuffer);
  text_layer_set_text(s_day_layer, dayBuffer);
}

static void main_window_load(Window *window) {
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 51, 144, 100));
  s_date_layer = text_layer_create(GRect(0, 101, 144, 80));
  s_day_layer = text_layer_create(GRect(4, 3, 144, 80));
  s_ampm_layer = text_layer_create(GRect(126, 85, 30, 30));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_background_color(s_ampm_layer, GColorClear);
  
  // Create bluetooth icon layer and register it
  bluetooth_icon_layer = layer_create(GRect(6, 137, 144, 168));
  layer_set_update_proc(bluetooth_icon_layer, bluetooth_icon_update_proc);
 
  // Get stored colors
  #ifdef PBL_COLOR
    int red, green, blue;
    if (persist_exists(KEY_BGCOLOR_R)) {
      red = persist_read_int(KEY_BGCOLOR_R);
      green = persist_read_int(KEY_BGCOLOR_G);
      blue = persist_read_int(KEY_BGCOLOR_B);
      GColor bg_color = GColorFromRGB(red, green, blue);
      window_set_background_color(s_main_window, bg_color); 
    } 
    else {
      window_set_background_color(s_main_window, GColorVividCerulean); 
    }
    if (persist_exists(KEY_TIME_COLOR_R)) {
      red = persist_read_int(KEY_TIME_COLOR_R);
      green = persist_read_int(KEY_TIME_COLOR_G);
      blue = persist_read_int(KEY_TIME_COLOR_B);  
      GColor time_color = GColorFromRGB(red, green, blue);
      text_layer_set_text_color(s_time_layer, time_color);  
    } 
    else {
      text_layer_set_text_color(s_time_layer, GColorFolly);  
    }
  #endif
  if (persist_exists(KEY_BLUETOOTH_ICON_ENABLED)) {
    int enabled = persist_read_int(KEY_BLUETOOTH_ICON_ENABLED);
    bluetoothIconEnabled = enabled != 0;
  }
  else {
    bluetoothIconEnabled = true;
  }
  // Set secondary colors
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_text_color(s_ampm_layer, GColorWhite);
  
  // Improve the layout to be more like a watchface
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentLeft);

  // Add child layers to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_ampm_layer));
  layer_add_child(window_get_root_layer(window), bluetooth_icon_layer);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Show current connection state
  bt_handler(bluetooth_connection_service_peek());
  
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TIME_46));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_20));
  s_ampm_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AMPM_10));
  
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_font(s_day_layer, s_date_font);
  text_layer_set_font(s_ampm_layer, s_ampm_font);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_ampm_layer);
  // Destroy GFonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_ampm_font);
  
  layer_destroy(bluetooth_icon_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
  
static void init() {

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());  
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Subscribe to TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Subscribe to Bluetooth service
  bluetooth_connection_service_subscribe(bt_handler);
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
