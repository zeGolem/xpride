#include "xpride.h"
#include "flags.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

// Create the main window
// As it has a bunch of properties, we use a separate function to avoid
// pulluting main
static xcb_window_t create_main_window(xcb_connection_t* x_connection,
                                       xcb_screen_t*     x_screen)
{
	// NOTE: This is mostly copied from
	// https://xcb.freedesktop.org/tutorial/basicwindowsanddrawing/
	// I don't understand how everything works :/

	// 1. Generate an ID for the window
	xcb_window_t window = xcb_generate_id(x_connection);

	// 2. Prepare the values for the window
	uint32_t value_mask   = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t value_list[] = {x_screen->white_pixel, XCB_EVENT_MASK_EXPOSURE};

	// 2. Create a window with this ID
	// clang-format off
	// To condense the function call a bit, in ways that make sense
	xcb_create_window(x_connection,
	                  XCB_COPY_FROM_PARENT,   // depth
	                  window, x_screen->root, // window, parent
	                  0, 0, 750, 450,         // x, y, width, height
	                  0,                      // border width
	                  XCB_WINDOW_CLASS_INPUT_OUTPUT,
	                  x_screen->root_visual,
	                  value_mask, value_list);
	// clang-format on

	// Set the window title
	// (This is taken from xcb_change_property(3), I don't really understand how
	// it all works)
	xcb_change_property(x_connection,
	                    XCB_PROP_MODE_REPLACE,
	                    window,
	                    XCB_ATOM_WM_NAME,
	                    XCB_ATOM_STRING,
	                    8,
	                    WINDOW_TITLE_LEN,
	                    WINDOW_TITLE);

	return window;
}

// Create a blank GC (graphics context/xcb_gcontext_t), with no color
static xcb_gcontext_t create_blank_gc(xcb_connection_t* x_connection,
                                      xcb_screen_t*     x_screen)
{
	// Disable exposure event for this context.
	uint32_t value_mask   = XCB_GC_GRAPHICS_EXPOSURES;
	uint32_t value_list[] = {false};

	xcb_gcontext_t graphics_context = xcb_generate_id(x_connection);
	xcb_create_gc(
	    x_connection, graphics_context, x_screen->root, value_mask, value_list);

	return graphics_context;
}

// Change the foreground color of a GC (graphics context/xcb_gcontext_t)
static void set_gc_foreground(xcb_connection_t* x_connection,
                              xcb_gcontext_t    gc,
                              uint32_t          color)
{
	uint32_t value_mask = XCB_GC_FOREGROUND;
	xcb_change_gc(x_connection, gc, value_mask, &color);
}

// Draw a colored rectangle to a xcb window
// SIDE EFFECTS: gc's foreground will be set to the color of the rectangle
static void fill_colored_reclangle(xcb_connection_t*   x_connection,
                                   xcb_gcontext_t      gc,
                                   xcb_window_t        window,
                                   colored_rectangle_t rectangle)
{
	set_gc_foreground(x_connection, gc, rectangle.color);
	xcb_poly_fill_rectangle(x_connection, window, gc, 1, &rectangle.shape);
}

static void draw_flag(xcb_connection_t* x_connection,
                      xcb_gcontext_t    gc,
                      xcb_window_t      window,
                      xcb_rectangle_t   window_rect,
                      flag_t*           flag)
{
	resolved_flag_t* final_flag = resolve_flag_for_width_height(
	    flag, window_rect.width, window_rect.height);

	// For each stripe in the final flag
	for (size_t i = 0; i < final_flag->stripe_count; ++i)
		// Draw the stripe
		fill_colored_reclangle(
		    x_connection, gc, window, final_flag->stripes_as_rects[i]);

	// Free the resolved flag
	free(final_flag->stripes_as_rects);
	final_flag->stripes_as_rects = 0;
	free(final_flag);
}

int main(int, char**)
{
	flag_t* flag = read_flag_from_file("res/pride.flag");

	// Connect to the default X server
	xcb_connection_t* x_connection = xcb_connect(NULL, NULL);

	// Get information about the current X setup
	const xcb_setup_t* x_setup = xcb_get_setup(x_connection);

	// Get the first screen
	xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(x_setup);
	xcb_screen_t*         x_screen        = screen_iterator.data;

	// Create the main window
	xcb_window_t main_window = create_main_window(x_connection, x_screen);

	// Map the window on the screen
	xcb_map_window(x_connection, main_window);

	// Flush everything so that the window gets shown for sure
	xcb_flush(x_connection);

	// Create a simple graphics context for drawing things
	xcb_gcontext_t graphics_context = create_blank_gc(x_connection, x_screen);

	// Main even loop, while there are events available, recieve them, and store
	// them in `event`
	xcb_generic_event_t* event;
	while ((event = xcb_wait_for_event(x_connection))) {
		switch (event->response_type & ~0x80) {

			// On expose (draw) event
			case XCB_EXPOSE: {
				// Convert the event to an expose event, to get access to more
				// properties
				xcb_expose_event_t* expose_event = (xcb_expose_event_t*)event;

				// Get the window rect from the event
				xcb_rectangle_t window_rect = {
				    .x      = expose_event->x,
				    .y      = expose_event->y,
				    .width  = expose_event->width,
				    .height = expose_event->height,
				};

				// Draw the currently selcted flag
				draw_flag(x_connection,
				          graphics_context,
				          main_window,
				          window_rect,
				          flag);

				// Flush all events to make sure it gets rendered
				xcb_flush(x_connection);
			} break;

			default: break;
		}

		free(event);
	}

	// Once we are here, the window has been closed

	// Free the flag
	free(flag->color_stripes);
	flag->color_stripes = 0;
	free(flag);
	flag = 0;

	// Disconnect from the X server
	xcb_disconnect(x_connection);
}
