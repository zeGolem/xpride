#pragma once

#include "xpride.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// TODO: Be smart and remove this
// Max numbers of strips in a flag
#define MAX_STRIPE_COUNT 8

// Postfix/file format specifier/"type" added at the end of a flag file's name
#define FLAG_POSTFIX ".flag"

typedef struct flag {
	size_t    stripe_count;
	uint32_t* color_stripes;
} flag_t;

// Flag with stripes as colored rectangles adapted for a specific window size
typedef struct resolved_flag {
	size_t               stripe_count;
	colored_rectangle_t* stripes_as_rects;
} resolved_flag_t;

// Parse a flag file into a flag_t
flag_t* read_flag_from_file(FILE* file);

// Resolve (compute the size of the stripes) for the given with and height
resolved_flag_t* resolve_flag_for_width_height(flag_t* const flag,
                                               uint16_t      width,
                                               uint16_t      height);

// Find flag file from name
// This will look up multiple search paths and find the first matching one
// If no match is found, nullptr is returned
FILE* flag_file_from_name(char const* const flag_name);
