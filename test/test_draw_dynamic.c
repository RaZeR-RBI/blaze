#include "common.h"

struct BLZ_Vector4 clearColor = {0xC0, 0xFF, 0xEE, 0xFF};

int main()
{
	int i;
	struct BLZ_Texture *texture;
	struct BLZ_Vector4 white = {0xFF, 0xFF, 0xFF, 0xFF};
	struct BLZ_Vector2 position = {20, 20};
	if (Test_Init() != 0)
	{
		printf("Could not initialize test suite\n");
		return -1;
	}
	BLZ_Init(5, 100, DEFAULT);
	BLZ_SetViewport(WINDOW_WIDTH, WINDOW_HEIGHT);
	texture = BLZ_LoadTextureFromFile("test/test_texture.png", AUTO, 0, NONE);

	plan(1);
	/* draw the scene */
	BLZ_SetClearColor(clearColor);
	for (i = 0; i < 5; i++)
	{
		BLZ_Clear(ALL);
		BLZ_Draw(texture, position, NULL, 0.0f, NULL, NULL,
				 white, NONE, 0.0f);

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
