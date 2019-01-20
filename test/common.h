#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#define die(msg)             \
	do                       \
	{                        \
		printf("%s\n", msg); \
		return -1;           \
	} while (0);

#define die_on_error(val, msg) \
	do                         \
	{                          \
		if (val == 0)          \
		{                      \
			die(msg);          \
		}                      \
	} while (0);

#define TEST
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#define OPENGL_MAJOR 3
#define OPENGL_MINOR 2

#ifdef __WIN32__
#define SDL_MAIN_HANDLED
#endif

#include <SDL2/SDL.h>
#include "../blaze.h"
#include "../deps/tap.c/tap.h"

SDL_Window *window;
SDL_GLContext context;

int Test_Init();
void Test_Shutdown();

/*
*	Compares render output with the reference file located in test/refs.
*	"Likeness" ranges from 0.0 to 1.0, which means how many pixels should be
*	equal. For example, if likeness is 0.9, then 90% of the pixels must be same
*	in order to pass. Using 1.0 is not recommended due to possible differences
*	in renderer implementations.
*/
int Validate_Output(const char *test_name, float likeness);

#endif
