#include "pebble.h"
#include <pebble-universal-fb/pebble-universal-fb.h>

// Pointer to main window and layer
static Window *s_main_window;
static Layer *s_main_window_layer;

static Layer *s_canvas_layer;

// Pointer to mockup image and layer
static GBitmap *s_image;

static int row;

static void draw_canvas_layer(Layer *layer, GContext *ctx){
  
    GRect bounds = layer_get_bounds(layer);
  
    // Capture the frame buffer
    GBitmap *fb = graphics_capture_frame_buffer(ctx);
    
    for (int x=0; x<bounds.size.w+1; x++){
      // Get the image data
      GBitmapDataRowInfo image_info = gbitmap_get_data_row_info(s_image, row);
      GColor c1 = universal_fb_get_pixel_color(image_info, bounds, GPoint(x, row));
  
      // loop through the framebuffer
      for (int y=row; y>-1; y--){
        GBitmapDataRowInfo fb_info = gbitmap_get_data_row_info(fb, y);
        
        universal_fb_set_pixel_color(fb_info, bounds, GPoint(x, y), c1);
      }
    }
  
    // Release the frame buffer so it can be drawn
    graphics_release_frame_buffer(ctx, fb);
  
}


static void draw_timer_handler(void *context){
  
  // force the layer to redraw
  layer_mark_dirty(s_canvas_layer);
  
  // if we are not at top of the screen then register another call back
  if (row > 0){ 
    row--;
    app_timer_register(25, draw_timer_handler, NULL);
  }
  
}


static void start_flood_draw(){
 
  // get screen size
  GRect bounds = layer_get_bounds(s_main_window_layer);
  
  // set row to bottom row of screen
  row = bounds.size.h;
  
  // run the first callback
  app_timer_register(1, draw_timer_handler, NULL);
  
}


// WINDOW LOAD
static void main_window_load(Window *window) {

  // Get the root window layer
  s_main_window_layer = window_get_root_layer(s_main_window);

  // Get the size of the main window - as size now different for Rect or Round
  GRect s_main_window_layer_frame = layer_get_frame(s_main_window_layer);

  s_canvas_layer = layer_create(s_main_window_layer_frame);
  
  layer_add_child(s_main_window_layer, s_canvas_layer);
  
  s_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WIN98);
  
  layer_set_update_proc(s_canvas_layer, draw_canvas_layer);
  
  start_flood_draw();
  
}


// WINDOW UNLOAD
static void main_window_unload(Window *window) {

  //Destroy the mockup layer and image
  gbitmap_destroy(s_image);
  s_image = NULL;
    
}


// INIT
static void init(void) {

  // Create main Window
  s_main_window = window_create();
  
  window_set_background_color(s_main_window, GColorRed);
  
  // Set handlers to manage the loading and unloading of elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

}


// DEINIT
static void deinit(void) {
  
  // Destroy the main window
  window_destroy(s_main_window);
  
}


// MAIN PROGRAM LOOP
int main(void) {
  init();
  app_event_loop();
  deinit();
}