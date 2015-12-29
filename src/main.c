//////////////////////////////////////////////////////////////////// 
//
// PEBBLE binary circuit color clock V0.1             12-2015
//
// based on my old CIRCUIT BINARY watchface for the pebble smartwatch
//
// bachground.png IDENTIFIKATOR: IMAGE_BACKGROUND_
//
////////////////////////////////////////////////////////////////////

//TODO: BT check every secound

//Seting
#define SETTINGS_KEY 99
  
#include "pebble.h"

Window *window;

static GBitmap *background_image;
static BitmapLayer *background_layer;

static AppSync sync;
static uint8_t sync_buffer[256]; //128

static int valueRead, valueWritten;

typedef struct persist {
  //setting
	int mode;			// Farbe des hintergrunds
	int  color;		//Farbe der LEDs
  bool numbers;	//zeige nummer an
	bool vibe_h;
	bool vibe_bt;
	bool show_bat;
	bool show_dat;
	
	//remenber the old state to detect changes
	bool day;
	bool number_old;
  bool bt_old;
} __attribute__((__packed__)) persist;

persist settings = {
  .mode 			= 2,
	.color 			= 0,
	.numbers 		= true,
	.vibe_h			= false,
	.vibe_bt		= false,
	.show_bat		= true, //false,
	.show_dat		= true,
	.day 				= true,
	.number_old	= true,
	.bt_old			= true
};

enum {
  KEY_MODE      = 0,
	KEY_COLOR 		= 1,
	KEY_NUMBER		= 2,
	KEY_VIBE_H		= 3,
	KEY_VIBE_BT		= 4,
	KEY_SHOW_BAT	= 5
};

//black or white
bool day  = false; //false, true


Layer *led_layer;

//----------------------------------------- config

void set_background() {
    //change the background
    //clear old
    gbitmap_destroy(background_image);

    //set new background	
		if(settings.numbers){
			//with numbers
			switch(settings.mode){
				case 2: //green
					background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_GN);
					break;
				case 1: //white
					background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_WN);
					break;
				case 0:
				default : //black
					background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_BN);
					break;
			}
		}else{
			//without numbers
			switch(settings.mode){
				case 2: //green
					background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_GB);
					break;
				case 1: //white
					background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_WB);
					break;
				case 0:
				default : //black
					background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_BB);
					break;
			}
		}
  
		//write new background
    bitmap_layer_set_bitmap(background_layer, background_image);
    layer_mark_dirty(bitmap_layer_get_layer(background_layer)); 
}

//----------------------------------------- save - load settings

static void loadPersistentSettings() {  
  valueRead = persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void savePersistentSettings() {
  valueWritten = persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

//------------------------------------------------------- Sync callback

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY: %d", (int)key);
	
	switch (key) {
    case KEY_MODE:
			settings.mode = (int)new_tuple->value->int32;
      break;
    case KEY_COLOR:
      settings.color = (int)new_tuple->value->int32;
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Day_start Time: %d", (int)settings.Day_start);
    	break;
		case KEY_NUMBER:
			if(strcmp(new_tuple->value->cstring, "show") == 0){settings.numbers = true;}
			if(strcmp(new_tuple->value->cstring, "hide") == 0){settings.numbers = false;}
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "show numbers: %d", settings.numbers);
			break;
		case KEY_VIBE_H:
			settings.vibe_h = (((int)new_tuple->value->int32) == 0 ? false : true);
			break;
		case KEY_VIBE_BT:
			settings.vibe_bt = (((int)new_tuple->value->int32) == 0 ? false : true);
			break;
		case KEY_SHOW_BAT:
			settings.show_bat = (((int)new_tuple->value->int32) == 0 ? false : true);
			break;
  }

  set_background();
	savePersistentSettings();
}

//----------------------------------------- Clock

//draw the LEDs
GRect getRect(int row, int led) {
  if (row == 0)
    return GRect(19 + led * 31, 21, 13, 13);//GRect(4 + led * 40, 21, 18, 18);
  else
    return GRect(14 + led * 20, 135, 10, 10);
}

//Change the LED color
void color_led(GContext* ctx, int row, int led, bool on) {
	
	//set default colors
	GColor8 color_led_on  = GColorWhite;	
	GColor8 color_led_off = GColorBlack;	

	switch(settings.color){
		case 5: //orange
			color_led_on = GColorFromRGB(255, 85, 0);
			break;
		case 4: //blue
			color_led_on = GColorFromRGB(0, 0, 255);
			break;
		case 3: //green
			color_led_on = GColorFromRGB(0, 255, 0);
			break;
		case 2: //yellow
			color_led_on = GColorFromRGB(255, 255, 0);
			break;
		case 1: //red
			color_led_on = GColorFromRGB(255, 0, 0);
			break;
		case 0:	//white
		default : 
			color_led_on = GColorWhite;
		break;
	}

	graphics_context_set_fill_color(ctx, on ? color_led_on : color_led_off); //on

  graphics_fill_rect(ctx, getRect(row, led), 1, 0);
}

//change bt status
void bt_status(GContext* ctx, bool on) {
  //graphics_context_set_fill_color(ctx, on ? GColorWhite : GColorBlack); //on
	graphics_context_set_fill_color(ctx, settings.mode != 1 ? GColorWhite : GColorBlack); //on
  if(on){ 	//BTon
		//graphics_fill_rect(ctx, GRect(1 , 1, 10, 10), 1, 0);	
		graphics_fill_circle(ctx, GPoint(82, 94), 3);
	}else{    //BTOff
		graphics_fill_circle(ctx, GPoint(82, 94), 3);
		graphics_context_set_fill_color(ctx, settings.mode == 1 ? GColorWhite : GColorBlack);
		graphics_fill_circle(ctx, GPoint(82, 94), 2);
  }
	
	//diconnect vibe
	if(settings.vibe_bt && settings.bt_old && !on){vibes_long_pulse();}
	settings.bt_old = on;//save the old status
}

void bat_status(GContext* ctx, BatteryChargeState charge_state) {
	if(settings.show_bat){  //settings.show_bat //true
		//change the color
		graphics_context_set_fill_color(ctx, settings.mode == 0 ? GColorWhite : GColorBlack); //on
	  //draw the line
		
		float var1 = (144*charge_state.charge_percent)/100;
		int var = (int)var1;
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery Level is %d", charge_state.charge_percent);
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery Bar hav %d", var);
		
		graphics_fill_rect(ctx, GRect(0, 166, var, 10), 1, 0);
	}
}

//binary calc minute
void led_layer_update_callback(Layer *me, GContext* ctx) {
  
  set_background();  //just do it in the hour trigger to save energy

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  unsigned short hour = t->tm_hour % 12;
  hour = hour ? hour : 12; // 0 as 12
  unsigned short min = t->tm_min;

	//hours
  color_led(ctx, 0, 0, hour & 8);
  color_led(ctx, 0, 1, hour & 4);
  color_led(ctx, 0, 2, hour & 2);
  color_led(ctx, 0, 3, hour & 1);

	//minutes
  color_led(ctx, 1, 0, min & 32);
  color_led(ctx, 1, 1, min & 16);
  color_led(ctx, 1, 2, min & 8);
  color_led(ctx, 1, 3, min & 4);
  color_led(ctx, 1, 4, min & 2);
  color_led(ctx, 1, 5, min & 1);
	
	//bt connection On/Off
	bt_status(ctx, bluetooth_connection_service_peek());
	//show the battery status
	bat_status(ctx, battery_state_service_peek());
	
	
	//hourly vibes
	if(min == 0 &&  settings.vibe_h){vibes_short_pulse();} 
}

//minute trigger
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(led_layer);
}

//hourtrigger
void handle_hour_tick(struct tm *tick_time, TimeUnits units_changed) {
  //set_background();
}

//--------------------------------------------- init

static void do_init(void) {

  /*
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Start in Mode: %d", settings.Mode);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Start in Day: %d", settings.day);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Start number_old: %d", settings.number_old);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "end day: %d", settings.Day_end);
  */
  
  const int inbound_size = 256;//128;
  const int outbound_size = 256;//128;  
  app_message_open(inbound_size, outbound_size); 
  
  //load the persist config setting  
  loadPersistentSettings(); 
	
	/*
	settings.mode = 0;
	settings.color = 0;
	*/
	
	/*
  APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD Mode: %d", settings.Mode);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD Day: %d", settings.day);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD number_old: %d", settings.number_old);
  */
	
  window = window_create();
  
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);
	
  GRect frame = layer_get_frame(window_layer);
  
  //load the background

  
  //set default background
	background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_WB);
  background_layer = bitmap_layer_create(layer_get_frame(window_layer));
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));		
	
  set_background();
  
  // Init the layer for the display
  led_layer = layer_create(frame);
  layer_set_update_proc(led_layer, &led_layer_update_callback);
  layer_add_child(window_layer, led_layer);

	//eingangsvariablen -> nicht mehr vergessen
  Tuplet initial_values[] = {
    TupletInteger(KEY_MODE, settings.mode),
    TupletInteger(KEY_COLOR, settings.color),
		TupletInteger(KEY_NUMBER, settings.numbers),
		TupletInteger(KEY_VIBE_H, settings.vibe_h),
		TupletInteger(KEY_VIBE_BT, settings.vibe_bt),
		TupletInteger(KEY_SHOW_BAT, settings.show_bat)
  };
  
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values), sync_tuple_changed_callback, NULL, NULL);
  
  tick_timer_service_subscribe(HOUR_UNIT, &handle_hour_tick);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  
}

static void do_deinit(void) {
  
  savePersistentSettings();
  app_sync_deinit(&sync);
  
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);
  layer_destroy(led_layer);
  window_destroy(window);	 
}

int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}