#include <pebble.h>
  
#define KEY_ICON 0
  
static Window *s_main_window;

 
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap;
const int animation_frames[3][2] = { 
  { // Ecstatic -> 0
    RESOURCE_ID_ECSTATIC_0,
    RESOURCE_ID_ECSTATIC_1
  },
  { // Happy -> 1
    RESOURCE_ID_FRAME_1,
    RESOURCE_ID_FRAME_10 
  },
  { // Annoyed -> 2
    RESOURCE_ID_FRAME_20,
    RESOURCE_ID_FRAME_30 
  }

};

int frame_no;
int selected_image = 0;
bool image_changed = 0;

static void timer_handler_lol(void *context) {
    if (s_icon_bitmap != NULL) {
      gbitmap_destroy(s_icon_bitmap);
      s_icon_bitmap = NULL;
    }
    if (image_changed){
      frame_no = 0;
      image_changed = 0;
    }
    
    s_icon_bitmap = gbitmap_create_with_resource(animation_frames[selected_image][frame_no]);
    
    bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(s_icon_layer));

    frame_no++;
    frame_no %= ARRAY_LENGTH(animation_frames[selected_image]);

    app_timer_register(800, timer_handler_lol, NULL); 
    
};

static void load_sequence() {
  frame_no = 0;
  app_timer_register(1, timer_handler_lol, NULL);
}



enum MessageKeys {
  AVATAR_ICON_KEY = 0x0,         // TUPLE_INT
};
 
static void main_window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window load!");
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_icon_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));
  
  load_sequence();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Finished window load!");
}
 
static void main_window_unload(Window *window) {
  gbitmap_destroy(s_icon_bitmap);
  bitmap_layer_destroy(s_icon_layer);
}
 
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  //load_sequence();
  // Get weather update every 7 seconds
  if(tick_time->tm_sec % 7 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
 
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
 
    // Send the message!
    app_message_outbox_send();
  }
}
 
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read tuples for data
  Tuple *icon_tuple = dict_find(iterator, KEY_ICON);

  // If all data is available, use it
  if(icon_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing image %lu!", (unsigned long)icon_tuple->value->uint8);
    selected_image = icon_tuple->value->int32;
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
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
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