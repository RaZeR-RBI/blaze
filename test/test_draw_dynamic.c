#include "common.h"
#include "unistd.h"

/* #define ASSERT_FEEDBACK */

#ifdef ASSERT_FEEDBACK
#define INIT_FLAGS ENABLE_FEEDBACK
#define FEEDBACK_COUNT 4
#define FEEDBACK_SIZE FEEDBACK_COUNT * 4
#else
#define INIT_FLAGS DEFAULT
#endif

struct BLZ_Vector4 clearColor = {0, 0, 0, 0};

int main(int argc, char *argv[])
{
	int i;
	char cwd[255];
	struct BLZ_Texture *texture;
	struct BLZ_Vector4 white = {1, 1, 1, 1};
	struct BLZ_Vector2 position = {20, 20};
#ifdef ASSERT_FEEDBACK
	GLfloat feedback[FEEDBACK_SIZE];
	int j;
#endif
	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
		printf("Could not get current directory - getcwd fail\n");
		return -1;
	}
	printf("Current working dir: %s\n", cwd);
	if (Test_Init() != 0)
	{
		printf("Could not initialize test suite\n");
		return -1;
	}
	BLZ_Init(5, 100, INIT_FLAGS);
	BLZ_SetViewport(WINDOW_WIDTH, WINDOW_HEIGHT);
#ifdef ASSERT_FEEDBACK
	Feedback_Enable(FEEDBACK_SIZE);
#endif
	texture = BLZ_LoadTextureFromFile("test/test_texture.png", AUTO, 0, NONE);
	if (texture == NULL)
	{
		BAIL_OUT("Could not load texture file!");
	}

	plan(1);
	/* draw the scene */
	BLZ_SetClearColor(clearColor);
	for (i = 0; i < 5; i++)
	{
		BLZ_Clear(COLOR_BUFFER);
#ifdef ASSERT_FEEDBACK
		Feedback_Begin();
#endif
		BLZ_Draw(texture, position, NULL, 0.0f, NULL, NULL, white, NONE);
#ifdef ASSERT_FEEDBACK
		Feedback_End();
		Feedback_Read((GLfloat *)&feedback, FEEDBACK_SIZE);
		printf("Frame %d\n", i);
		for (j = 0; j < FEEDBACK_SIZE; j++)
		{
			printf("%d: %d\n", j, feedback[j]);
		}
#endif
		BLZ_Present();
		SDL_GL_SwapWindow(window);
	}
	/* create a screenshot and compare */
	/* TODO */
	BLZ_SaveScreenshot("test_draw_dynamic.bmp", BMP, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	ok(BLZ_FALSE);

	BLZ_FreeTexture(texture);
	BLZ_Shutdown();
	Test_Shutdown();
	done_testing();
}
