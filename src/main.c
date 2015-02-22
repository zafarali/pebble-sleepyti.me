#include <pebble.h>
  
#define WAKEUP_REASON 0
#define PERSIST_WAKEUP_ID_KEY 15
  
  
//windows and texts
static Window *w_window;
static Window *w_confirm;
static TextLayer *tl_message;
static TextLayer *tl_confirm_message;
static TextLayer *tl_confirm_time;
static MenuLayer *m_time_list;
static WakeupId s_wakeup_id;

// time taken to fall asleep in seconds
const int FALL_ASLEEP_TIME = 840;
int wakeytimes[] = {0, 0, 0, 0, 0, 0 };
int selectedwakeup = 0;
///
/// L O G I C    F O R
/// C A L C U L A T I N G   B E S T   W A K E   U P   T I M E
///


void wakeytimecalculate(void){
  const time_t now = time(NULL);
  
  for(int i = ARRAY_LENGTH(wakeytimes); i > 0; i--) {
    wakeytimes[ARRAY_LENGTH(wakeytimes)-i] = now + FALL_ASLEEP_TIME + (i * 90 * 60);
//     printf("%d\n", wakeytimes[ARRAY_LENGTH(wakeytimes)-i]);
  }
  
}

struct tm * parseEpoch(int epoch_time) {
    static struct tm * timeinfo;
    time_t epoch_time_as_time_t = epoch_time;
    timeinfo = localtime(&epoch_time_as_time_t);
    return timeinfo;
}
  
  char* parseWakeyIndex(int index) {
  static char buffer[] = "00:00AA";
  
  struct tm* timeinfo = parseEpoch(wakeytimes[index]);    
  
  if (clock_is_24h_style() == true) {
    strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
  } else {
    strftime(buffer, sizeof(buffer), "%I:%M%p", timeinfo);
  }
  
  return buffer;
}

char* parse_time_t(time_t *timestamp){
    static struct tm* timeinfo;
    timeinfo = localtime(timestamp);
  
    static char timebuffer[]= "123456879";
    if (clock_is_24h_style() == true) {
      strftime(timebuffer, sizeof(timebuffer), "%H:%M", timeinfo);
    } else {
      strftime(timebuffer, sizeof(timebuffer), "%I:%M%p", timeinfo);
    }
  
    return timebuffer;
}


///
/// W A K E  U P 
/// H A N D L E R
///

static void wakeup_handler(WakeupId id, int32_t reason) {
  //wake up event occured must do something....
  text_layer_set_text(tl_message, "Wakey wakey!");
  
  persist_delete(PERSIST_WAKEUP_ID_KEY);
}

///
/// C L I C K
/// H A N D L E R S
///

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(tl_confirm_message, "PRESSED UP EH");
  uint32_t segments[] = {100, 200, 500, 1000, 500, 1000};
  
  VibePattern pattern = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments)
  };
  
  vibes_enqueue_custom_pattern(pattern);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(tl_confirm_message, "PRESSED SELECT");
  vibes_double_pulse();
}

//subscribers
void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}



///
///  C O N F I R M 
///  W I N D O W   S T U F F
///


static void window_confirm_load(Window *window) {
  tl_confirm_message = text_layer_create(GRect(0, 0, 144, 90));
  text_layer_set_text_color(tl_confirm_message, GColorBlack);
  
  tl_confirm_time = text_layer_create(GRect(0,30,144,200));
  text_layer_set_font(tl_confirm_time, FONT_KEY_GOTHIC_28_BOLD);

  char* buffer2 = parseWakeyIndex(selectedwakeup);
  
  // for some reason this text doesn't print on the screen!
  text_layer_set_text(tl_confirm_message, "Are you sure you want to wake up at:");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(tl_confirm_message));
  printf("text layer tl_confirm_message was pushed");
  text_layer_set_text(tl_confirm_time,buffer2);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(tl_confirm_time));
    printf("text layer tl_confirm_time was pushed");

}

static void window_confirm_unload(Window *window){
  text_layer_destroy(tl_confirm_message);
  text_layer_destroy(tl_confirm_time);
}


///
/// M E N U 
/// C A L L B A C K S
///

void draw_row_callback(GContext *context, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {

  int index = cell_index->row;    

  char* buffer;
  buffer = parseWakeyIndex(index);
  char buffer2[] = "0 cycles";
  int cycle_number = ARRAY_LENGTH(wakeytimes)-index;
  snprintf(buffer2, 12, "%d cycle (s)", cycle_number);
  menu_cell_basic_draw(context, cell_layer, buffer , buffer2, NULL);
//   printf("%s\n", buffer);  
    
  
}

uint16_t num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) { 
  return 6;
}

void select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  int which = cell_index->row;
  selectedwakeup = which;
  
  
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
  
//   w_confirm = window_create();
//   window_set_window_handlers(w_confirm, (WindowHandlers) {
//     .load = window_confirm_load,
//     .unload = window_confirm_unload
//   });
  
  
//   window_set_click_config_provider(w_confirm, click_config_provider);
//   window_stack_push(w_confirm, true);

  if(!wakeup_query(s_wakeup_id, NULL)){
    
    int epochtime = wakeytimes[which];
    time_t when = epochtime;
    
    wakeup_query(s_wakeup_id, &when);
    
    
    static char buffer2[32];
    char* timebuffer = parse_time_t(&when);
    
    snprintf(buffer2, 32, "You will be woken up at %s", timebuffer);
    
    text_layer_set_text(tl_message, buffer2);  
  }
  
}

///
/// W I N D O W 
/// H A N D L E R S
///


//handles window stuff
static void window_load(Window *window) {
  tl_message = text_layer_create(GRect(0, 0, 144, 40));
  
  s_wakeup_id = persist_read_int(PERSIST_WAKEUP_ID_KEY );
  
  if( s_wakeup_id > 0 ) {
    time_t timestamp = 0;
    wakeup_query(s_wakeup_id, &timestamp);
    

    static struct tm* timeinfo;
    timeinfo = localtime(&timestamp);
  
    char timebuffer[]= "123456879";
    if (clock_is_24h_style() == true) {
      strftime(timebuffer, sizeof(timebuffer), "%H:%M", timeinfo);
    } else {
      strftime(timebuffer, sizeof(timebuffer), "%I:%M%p", timeinfo);
    }

    static char s_buffer[64];
    
    snprintf(s_buffer, sizeof(s_buffer), "You are scheduled to wake up at %s, want to change that?", timebuffer);
    text_layer_set_text(tl_message, s_buffer);
  } else {
    text_layer_set_text(tl_message, "Going to sleep now? Wake up at:");
  }
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
    text_layer_destroy(tl_message);

}



///
/// I N I T
/// D E I N I T
///

//initialization handler
void init(void) {
  w_window = window_create();
  wakeytimecalculate();
//   printf("wakey times calculated");
  window_set_window_handlers(w_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  
  window_stack_push(w_window, true);
  
  wakeup_service_subscribe(wakeup_handler);
  if(launch_reason() == APP_LAUNCH_WAKEUP) {
    WakeupId id = 0;
    int32_t reason = 0;
    
    //handle the wake up
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
  }
}

//deinitialization handler
void deinit(void) {

  window_destroy(w_window);
  window_destroy(w_confirm);

}



//main function
int main(void) {
  init();
  app_event_loop();
  deinit();
}
