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

void Feedback_Enable(int count);
void Feedback_Read(GLfloat* buffer, int count);
void Feedback_Begin();
void Feedback_End();

#endif
