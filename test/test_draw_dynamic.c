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

struct BLZ_Texture *textures[2];
struct BLZ_Vector4 white = {1, 1, 1, 1};
struct BLZ_Vector2 startPosition = {20, 20};
struct BLZ_Vector2 position;
struct BLZ_Vector2 center = {8, 8}; /* texture center */
struct BLZ_Rectangle texPart = {4, 4, 8, 8};
struct BLZ_Vector2 scale = {1, 1};

void draw(struct BLZ_Texture *texture)
{
	int j;
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
	/* Draw only specified part using different scales */
	for (j = 0; j < 12; j++)
	{
		scale.x = j / 6.0f;
		scale.y = j / 6.0f;
		BLZ_Draw(texture, position, &texPart, 0.0f, NULL, &scale, white, NONE);
		MoveRight();
	}
	NextLine();
	/* Do various flips */
	for (j = 0; j < 4; j++)
	{
		BLZ_Draw(texture, position, NULL, 0.0f, NULL, NULL, white,
				 (enum BLZ_SpriteEffects)(j % 4));
		MoveRight();
	}
	NextLine();
	NextLine();
}

int main(int argc, char *argv[])
{
	int i, tex;
	char cwd[255];
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
	textures[0] = BLZ_LoadTextureFromFile("test/test_texture.png", AUTO, 0, NONE);
	textures[1] = BLZ_LoadTextureFromFile("test/test_texture2.png", AUTO, 0, NONE);
	if (textures[0] == NULL || textures[1] == NULL)
	{
		BAIL_OUT("Could not load texture file!");
	}

	plan(1);
	/* draw the scene */
	BLZ_SetClearColor(clearColor);
	for (i = 0; i < 5; i++)
	{
		position = startPosition;
		BLZ_Clear(COLOR_BUFFER);
		draw(textures[0]);
		draw(textures[1]);
		BLZ_Present();
		SDL_GL_SwapWindow(window);
	}
	/* create a screenshot and compare */
	ok(Validate_Output("test_draw_dynamic", 0.999f));

	BLZ_FreeTexture(textures[0]);
	BLZ_FreeTexture(textures[1]);
	BLZ_Shutdown();
	Test_Shutdown();
	done_testing();
}
