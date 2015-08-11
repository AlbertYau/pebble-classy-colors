#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;
static TextLayer *s_ampm_layer;
static GFont s_time_font;
static GFont s_date_font;
static GFont s_ampm_font;

#define KEY_BGCOLOR_R 0
#define KEY_BGCOLOR_G 1
#define KEY_BGCOLOR_B 2
#define KEY_TIME_COLOR_R 3
#define KEY_TIME_COLOR_G 4
#define KEY_TIME_COLOR_B 5

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  // Background color?
  Tuple *color_red_t = dict_find(iter, KEY_BGCOLOR_R);
  Tuple *color_green_t = dict_find(iter, KEY_BGCOLOR_G);
  Tuple *color_blue_t = dict_find(iter, KEY_BGCOLOR_B);
  if(color_red_t && color_green_t && color_blue_t) {
    // Apply the color if available
  #if PBL_SDK_3 
      int red = color_red_t->value->int32;
      int green = color_green_t->value->int32;
      int blue = color_blue_t->value->int32;
  
      // Persist values
      persist_write_int(KEY_BGCOLOR_R, red);
      persist_write_int(KEY_BGCOLOR_G, green);
      persist_write_int(KEY_BGCOLOR_B, blue);
  
      GColor bg_color = GColorFromRGB(red, green, blue);
      window_set_background_color(s_main_window, bg_color);
  #endif
  }
  // Time color?
  color_red_t = dict_find(iter, KEY_TIME_COLOR_R);
  color_green_t = dict_find(iter, KEY_TIME_COLOR_G);
  color_blue_t = dict_find(iter, KEY_TIME_COLOR_B);
  if(color_red_t && color_green_t && color_blue_t) {
    // Apply the color if available
  #if PBL_SDK_3 
      int red = color_red_t->value->int32;
      int green = color_green_t->value->int32;
      int blue = color_blue_t->value->int32;
  
      // Persist values
      persist_write_int(KEY_TIME_COLOR_R, red);
      persist_write_int(KEY_TIME_COLOR_G, green);
      persist_write_int(KEY_TIME_COLOR_B, blue);
  
      GColor time_color = GColorFromRGB(red, green, blue);
      text_layer_set_text_color(s_time_layer, time_color); 
  #endif
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
 
  #ifdef PBL_COLOR
    int red, green, blue;
    if (persist_exists(KEY_BGCOLOR_R)) {
      // Use background color setting
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
  
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_text_color(s_ampm_layer, GColorWhite);
  //text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  //text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  //text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  //text_layer_set_font(s_ampm_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  
  // Improve the layout to be more like a watchface
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentLeft);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_ampm_layer));
    
  // Make sure the time is displayed from the start
  update_time();
  
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
  
  // Set background color
  #ifdef PBL_COLOR
    window_set_background_color(s_main_window, GColorVividCerulean);
  #else
    window_set_background_color(s_main_window, GColorBlack);
  #endif
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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
