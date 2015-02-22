#include <pebble.h>
  
  
  
  
//windows and texts
Window *w_window;
TextLayer *tl_message;
MenuLayer *m_time_list;

// time taken to fall asleep in seconds
const int FALL_ASLEEP_TIME = 840;
int wakeytimes[] = {0, 0, 0, 0, 0, 0 };

///
/// L O G I C    F O R
/// C A L C U L A T I N G   B E S T   W A K E   U P   T I M E
///


void wakeytimecalculate(void){
  const time_t now = time(NULL);
  
  for(int i = ARRAY_LENGTH(wakeytimes); i > 0; i--) {
    wakeytimes[ARRAY_LENGTH(wakeytimes)-i] = now + FALL_ASLEEP_TIME + (i * 90 * 60);
    printf("%d\n", wakeytimes[ARRAY_LENGTH(wakeytimes)-i]);
  }
  
}


///
/// M E N U 
/// C A L L B A C K S
///

void draw_row_callback(GContext *context, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {

  int index = cell_index->row;    
  char buffer[] = "0sadihfbasudbfi";
//   snprintf(buffer, sizeof(buffer), "%d", index);
//   char* epochtime = wakey(index);
  
  time_t c;
  int curtime = wakeytimes[index];    
  struct tm * timeinfo;
  
  time_t epoch_time_as_time_t = curtime;
  timeinfo = localtime(&epoch_time_as_time_t);

  //struct tm humantime = localtime(time(epochtime));
  
  
  strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
  menu_cell_basic_draw(context, cell_layer, buffer , NULL, NULL);
  printf("%s\n", buffer);  
    
  
}

uint16_t num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) { 
  return 6;
}

void select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  int which = cell_index->row;
  
  char buffer[] = "noth";
  snprintf(buffer, sizeof(buffer), "%d", which);
  text_layer_set_text(tl_message, buffer);
  
  uint32_t segments[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  
  for(int i = 0; i < which +1; i++) {
    segments[2*i] = 200;
    segments[(2*i)+1] = 100;
  }
  
  VibePattern pattern = {
    .durations = segments,
    .num_segments = 16
  };
  
  vibes_enqueue_custom_pattern(pattern);
}

///
/// W I N D O W 
/// H A N D L E R S
///


//handles window stuff
static void window_load(Window *window) {
  tl_message = text_layer_create(GRect(0, 0, 144, 40));
  text_layer_set_text(tl_message, "Going to sleep now? Wake up at:");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(tl_message));

  m_time_list = menu_layer_create(GRect(0,36,144, 120));
  menu_layer_set_click_config_onto_window(m_time_list, w_window);
  
  
  MenuLayerCallbacks callbacks = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
    .select_click = (MenuLayerSelectCallback) select_click_callback
  };
  
  menu_layer_set_callbacks(m_time_list, NULL, callbacks);
  
  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(m_time_list));
}


static void window_unload(Window *window) {
  menu_layer_destroy(m_time_list);
  
}

///
/// C L I C K
/// H A N D L E R S
///

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(tl_message, "PRESSED UP EH");
  uint32_t segments[] = {100, 200, 500, 1000, 500, 1000};
  
  VibePattern pattern = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments)
  };
  
  vibes_enqueue_custom_pattern(pattern);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(tl_message, "PRESSED SELECT");
  vibes_double_pulse();
}

// subscribers
void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

///
/// I N I T
/// D E I N I T
///

//initialization handler
void init(void) {
  w_window = window_create();
  wakeytimecalculate();
  printf("wakey times calculated");
  window_set_window_handlers(w_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  
  window_set_click_config_provider(w_window, click_config_provider);
  window_stack_push(w_window, true);
}

//deinitialization handler
void deinit(void) {
  text_layer_destroy(tl_message);
  window_destroy(w_window);
}



//main function
int main(void) {
  init();
  app_event_loop();
  deinit();
}
