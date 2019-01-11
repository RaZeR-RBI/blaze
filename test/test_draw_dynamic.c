#include "common.h"
#include "unistd.h"

struct BLZ_Vector4 clearColor = {0, 0, 0, 0};
struct BLZ_Vector4 colors[12] = {
	{1, 0, 0, 1},
	{0, 1, 0, 1},
	{0, 0, 1, 1},
	{1, 1, 0, 1},
	{0, 1, 1, 1},
	{1, 0, 1, 1},
	{1, 0, 0, 0.5f},
	{0, 1, 0, 0.5f},
	{0, 0, 1, 0.5f},
	{1, 1, 0, 0.5f},
	{0, 1, 1, 0.5f},
	{1, 0, 1, 0.5f},
};

#define DEGREES(x) ((x)*3.14159265f / 180.0f)
#define MoveRight()       \
	do                    \
	{                     \
		position.x += 40; \
	} while (0);
#define NextLine()        \
	do                    \
	{                     \
		position.x = 20;  \
		position.y += 40; \
	} while (0);

int main(int argc, char *argv[])
{
	int i, j;
	char cwd[255];
	struct BLZ_Texture *texture;
	struct BLZ_Vector4 white = {1, 1, 1, 1};
	struct BLZ_Vector2 position = {20, 20};
	struct BLZ_Vector2 center = {8, 8}; /* texture center */
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
	BLZ_Init(5, 100, DEFAULT);
	BLZ_SetViewport(WINDOW_WIDTH, WINDOW_HEIGHT);
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
		/* Different rotation angles */
		for (j = 0; j < 12; j++)
		{
			BLZ_Draw(texture, position, NULL, DEGREES(30.0f * j), NULL, NULL, white, NONE);
			MoveRight();
		}
		NextLine();
		/* Different colors */
		for (j = 0; j < 12; j++)
		{
			BLZ_Draw(texture, position, NULL, 0, NULL, NULL, colors[j], NONE);
			MoveRight();
		}
		NextLine();
		/* Rotate around specified origin */
		for (j = 0; j < 12; j++)
		{
			BLZ_Draw(texture, position, NULL, DEGREES(30.0f * j), &center, NULL, white, NONE);
			MoveRight();
		}
		NextLine();
		BLZ_Present();
		SDL_GL_SwapWindow(window);
		position.y = 20;
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
