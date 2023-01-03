#pragma once

#include <xcb/xproto.h>

// The main window's title
#define WINDOW_TITLE     "xpride"
#define WINDOW_TITLE_LEN (sizeof(WINDOW_TITLE) - 1)

// XCB rectangle with extra color information
typedef struct colored_rectangle {
	xcb_rectangle_t shape;
	// GBRA color, as a hex number (0xbbggrraa)
	uint32_t color;
} colored_rectangle_t;
