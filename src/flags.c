#include "flags.h"
#include "xpride.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Check if a character is a valid hexadecimal digit
static bool is_hex(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
	       (c >= 'A' && c <= 'F');
}

// Convert a hex digit as a ASCII char to hex
static int from_hex(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

// Check if a NULL-terminated string is only made of alpha characters
static bool is_str_alpha(char const* const str)
{
	// For each character c in the string
	for (char const* c = str; *c; ++c)
		if (!isalpha(*c)) return false;
	// If no characters were not alpha, the whole string is alpha
	return true;
}

resolved_flag_t* resolve_flag_for_width_height(flag_t* const flag,
                                               uint16_t      width,
                                               uint16_t      height)
{
	// Allocate space for all the list of computed stripes
	colored_rectangle_t* computed_stripes =
	    malloc(sizeof(colored_rectangle_t) * flag->stripe_count);

	// Compute the size of a single stripe
	uint16_t stripe_height = (height / flag->stripe_count);

	// For each stripe in the original flag
	for (size_t i = 0; i < flag->stripe_count; ++i) {
		// Copy the color of the stripe
		computed_stripes[i].color = flag->color_stripes[i];

		// Those values are the same for all stripes of the flags
		computed_stripes[i].shape.x      = 0;
		computed_stripes[i].shape.width  = width;
		computed_stripes[i].shape.height = stripe_height;

		// Set the stripe's y based on its index
		computed_stripes[i].shape.y = (stripe_height * i);
	}

	// Allocate & return the resolved_flag
	resolved_flag_t* final_flag = malloc(sizeof(resolved_flag_t));

	final_flag->stripe_count     = flag->stripe_count;
	final_flag->stripes_as_rects = computed_stripes;

	return final_flag;
}

flag_t* read_flag_from_file(FILE* file)
{
	assert(file || "Trying to to parse a flag from a non-existant file");

	// Allocate memory for the color of the stripes
	uint32_t* colors      = malloc(MAX_STRIPE_COUNT * sizeof(uint32_t));
	size_t    color_index = 0;

	// The current character being read
	char current_char = 0;

	// Initialize the first color stripe to 0 (the others will be initialized as
	// they are needed
	colors[0] = 0;

	// Loop over each character in the file, while there is data to be read
	while (fread(&current_char, 1, 1, file)) {

		// If we encounter a new line, it's the end of the current color
		if (current_char == '\n') {

			// We move to the next color
			++color_index;

			// If we are over the stripe count limit
			if (color_index >= MAX_STRIPE_COUNT) {
				// Bail out nicely before things go very wrong
				fprintf(stderr,
				        "Reached maximum stripe count (%d), stopping",
				        MAX_STRIPE_COUNT);
				break;
			}

			// Be nice and initialize the color :)
			colors[color_index] = 0;

			// Go back at the start of the loop and read the next character
			continue;
		}

		// If the character isn't hex, something is wrong...
		if (!is_hex(current_char)) {
			// Warn the user about it
			fprintf(
			    stderr,
			    "Unexpected character: '%c' (0x%x), expected valid hexadecimal "
			    "character.\n",
			    current_char,
			    current_char);

			// Ignore the wrong character and continue parsing
			continue;
			// TODO: Is continuing a good solution..? Should we just bail out?
		}

		// We have a hex digit to handle here!

		// Get the current digit as an int to do math on it
		int digit_value = from_hex(current_char);
		if (digit_value == -1)
			assert(false ||
			       "current_char is not hex, this should never happen");

		// Append the hex digit to the current color
		colors[color_index] <<= 4;
		colors[color_index] |= digit_value;
	}

	// Now we create the flag struct
	flag_t* flag        = malloc(sizeof(flag_t));
	flag->color_stripes = colors;
	flag->stripe_count  = color_index;

	return flag;
}

// Try to open a flag with the given name in the given folder
// This also adds `FLAG_POSTFIX` to the end of `flag_name`
// NOTE: `folder` has to end with a '/'
static FILE* flag_file_from_name_and_dir(char const* const folder,
                                         char const* const flag_name)
{
	// Find the size of the full path
	size_t full_path_size = sizeof(char) * (strlen(folder) + strlen(flag_name) +
	                                        sizeof(FLAG_POSTFIX)) +
	                        1; // +1 for the NULL terminator
	// Allocate the full path string based on this size
	char* full_path = malloc(full_path_size);
	// 0 out the string
	memset(full_path, 0, full_path_size);

	// Append the folder path to the string
	strncpy(full_path, folder, full_path_size);
	// Concat the flag name
	strncat(full_path, flag_name, full_path_size);
	// Concat the flag postif
	strncat(full_path, FLAG_POSTFIX, full_path_size);

	// Open the file
	FILE* flag_file = fopen(full_path, "r");

	// Free up the memory allocated to the string
	free(full_path);

	// Give back the file
	return flag_file;
}

FILE* flag_file_from_name(char const* const flag_name)
{
	// Try to open the flag name as a path
	FILE* file = fopen(flag_name, "r");
	// If it works, return here
	if (file) return file;

	// We'll now try to find a matching file in a common location

	// We make sure there are no weird characters in the flag name to avoid path
	// traversals and other bad stuff
	if (!is_str_alpha(flag_name)) return 0;

	// Try to load a flag in "/usr/share/flags/"
	file = flag_file_from_name_and_dir("/usr/share/flags/", flag_name);
	// If it works, return the flag file
	if (file) return file;

	// If nothing worked, return null
	return 0;
}
