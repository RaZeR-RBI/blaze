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
struct BLZ_Vector2 center = {8, 8}; /* texture center */
struct BLZ_Rectangle texPart = {4, 4, 8, 8};
struct BLZ_Vector2 scale = {1, 1};

/* move 200 pixels down */
/* matrix is in NDC, so division by window height and multiply by 2 is needed */
/* the value is also negated because the Y axis goes down instead of up in OpenGL */
/* matrix is stored in column-major order, following the OpenGL matrix convention */
/* when transformed it looks like this:
*  1 0 0 0
*  0 1 0 (200/-2*h)
*  0 0 1 0
*  0 0 0 1
* so it's basically identity matrix with Y translation
*/
GLfloat moveDownTransform[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 200.0f / -WINDOW_WIDTH * 2.0f, 0, 1
};

void draw(struct BLZ_StaticBatch *batch)
{
	int j;
	struct BLZ_Vector2 position = startPosition;
	/* Different rotation angles */
	for (j = 0; j < 12; j++)
	{
		BLZ_DrawStatic(batch, position, NULL, DEGREES(30.0f * j), NULL, NULL, white, NONE);
		MoveRight();
	}
	NextLine();
	/* Different colors */
	for (j = 0; j < 12; j++)
	{
		BLZ_DrawStatic(batch, position, NULL, 0, NULL, NULL, colors[j], NONE);
		MoveRight();
	}
	NextLine();
	/* Rotate around specified origin */
	for (j = 0; j < 12; j++)
	{
		BLZ_DrawStatic(batch, position, NULL, DEGREES(30.0f * j), &center, NULL, white, NONE);
		MoveRight();
	}
	NextLine();
	/* Draw only specified part using different scales */
	for (j = 0; j < 12; j++)
	{
		scale.x = j / 6.0f;
		scale.y = j / 6.0f;
		BLZ_DrawStatic(batch, position, &texPart, 0.0f, NULL, &scale, white, NONE);
		MoveRight();
	}
	NextLine();
	/* Do various flips */
	for (j = 0; j < 4; j++)
	{
		BLZ_DrawStatic(batch, position, NULL, 0.0f, NULL, NULL, white,
				 (enum BLZ_SpriteEffects)(j % 4));
		MoveRight();
	}
}

int main(int argc, char *argv[])
{
	int i, tex;
	char cwd[255];
	struct BLZ_StaticBatch* batches[2];
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
	BLZ_SetViewport(WINDOW_WIDTH, WINDOW_HEIGHT);
	textures[0] = BLZ_LoadTextureFromFile("test/test_texture.png", AUTO, 0, NONE);
	textures[1] = BLZ_LoadTextureFromFile("test/test_texture2.png", AUTO, 0, NONE);
	if (textures[0] == NULL || textures[1] == NULL)
	{
		BAIL_OUT("Could not load texture file!");
	}
	batches[0] = BLZ_CreateStatic(textures[0], 52);
	batches[1] = BLZ_CreateStatic(textures[1], 52);

	plan(1);
	/* draw the scene once into static buffer */
	/* it will be 'baked' into memory on first PresentStatic call */
	draw(batches[0]);
	draw(batches[1]);
	BLZ_SetClearColor(clearColor);
	BLZ_SetBlendMode(BLEND_NORMAL);
	for (i = 0; i < 5; i++)
	{
		BLZ_Clear(COLOR_BUFFER);
		BLZ_PresentStatic(batches[0], NULL);
		BLZ_PresentStatic(batches[1], (GLfloat*)&moveDownTransform);
		SDL_GL_SwapWindow(window);
	}
	/* create a screenshot and compare */
	ok(Validate_Output("test_draw_static", 0.999f));

	BLZ_FreeBatchStatic(batches[0]);
	BLZ_FreeBatchStatic(batches[1]);
	BLZ_FreeTexture(textures[0]);
	BLZ_FreeTexture(textures[1]);
	Test_Shutdown();
	done_testing();
}
