#include "common.h"
#include "unistd.h"

struct BLZ_Vector4 clearColor = {0.5f, 0.5f, 0.5f, 0};
struct BLZ_Vector4 colors[3] = {
	{1, 0, 0, 1.0f},
	{0, 1, 0, 1.0f},
	{0, 0, 1, 1.0f},
};

struct BLZ_Texture *texture;

void draw(
	struct BLZ_SpriteBatch *batch,
	struct BLZ_Texture *texture, int x, int y, const struct BLZ_BlendFunc blend)
{
	struct BLZ_Vector2 position = {x, y};
	BLZ_SetBlendMode(blend);
	BLZ_Draw(batch, texture, position, NULL, 0.0f, NULL, NULL, colors[0], NONE);
	position.x += 50;
	BLZ_Draw(batch, texture, position, NULL, 0.0f, NULL, NULL, colors[1], NONE);
	position.x -= 25;
	position.y += 25;
	BLZ_Draw(batch, texture, position, NULL, 0.0f, NULL, NULL, colors[2], NONE);
	BLZ_Present(batch); /* draw to screen before changing blend mode */
}

int main(int argc, char *argv[])
{
	int i, tex;
	char cwd[255];
	struct BLZ_SpriteBatch *batches[3];
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
	/* setting low limits to hit more code branches */
	/* in realistic use-cases numbers should be 10 or 100 times greater */
	for (i = 0; i < 3; i++)
	{
		batches[i] = BLZ_CreateBatch(2, 100, DEFAULT);
	}
	BLZ_SetViewport(WINDOW_WIDTH, WINDOW_HEIGHT);
	texture = BLZ_LoadTextureFromFile("test/circle_100px.png", AUTO, 0, NONE);
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
		draw(batches[0], texture, 50, 50, BLEND_NORMAL);
		draw(batches[1], texture, 300, 50, BLEND_ADDITIVE);
		draw(batches[2], texture, 175, 250, BLEND_MULTIPLY);
		SDL_GL_SwapWindow(window);
	}
	/* create a screenshot and compare */
	ok(Validate_Output("test_blend_modes", 0.999f));

	BLZ_FreeTexture(texture);
	for (i = 0; i < 3; i++)
	{
		BLZ_FreeBatch(batches[i]);
	}
	Test_Shutdown();
	done_testing();
}
