#include "common.h"

struct BLZ_Vector4 clearColor = {0, 0, 0, 0};
/* TODO: Remove this debug data */
struct BLZ_SpriteQuad fs_quad = {.vertices[0] = {-1, 1, 1.1, 1, 1, 1, 1, 1, 0, 0},
								 .vertices[1] = {-1, -1, 1.1, 1, 1, 1, 1, 1, 0, 0},
								 .vertices[2] = {1, -1, 1.1, 1, 1, 1, 1, 1, 0, 0},
								 .vertices[3] = {1, 1, 1.1, 1, 1, 1, 1, 1, 0, 0}};

int main()
{
	int i;
	struct BLZ_Texture *texture;
	struct BLZ_Vector4 white = {1, 1, 1, 0};
	struct BLZ_Vector2 position = {20, 20};
	if (Test_Init() != 0)
	{
		printf("Could not initialize test suite\n");
		return -1;
	}
	BLZ_Init(5, 100, DEFAULT);
	BLZ_SetViewport(WINDOW_WIDTH, WINDOW_HEIGHT);
	texture = BLZ_LoadTextureFromFile("test/test_texture.png", AUTO, 0, NONE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	plan(1);
	/* draw the scene */
	BLZ_SetClearColor(clearColor);
	for (i = 0; i < 5; i++)
	{
		BLZ_Clear(COLOR_BUFFER | DEPTH_BUFFER);
		// BLZ_Draw(texture, position, NULL, 0.0f, NULL, NULL,
		//  white, NONE, 0.1f);
		/* TODO: Remove this debug call */
		BLZ_LowerDraw(texture->id, &fs_quad);

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
