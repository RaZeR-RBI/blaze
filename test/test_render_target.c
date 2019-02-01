#include "common.h"
#include "unistd.h"

struct BLZ_Vector4 clearColor = {0, 0, 0, 0};
struct BLZ_Vector4 white = {1, 1, 1, 1};
struct BLZ_Vector4 red = {1, 0, 0, 1};
struct BLZ_Vector2 position = {156, 156};
struct BLZ_Vector2 topLeft = {0, 0};

int main(int argc, char *argv[])
{
	int i;
	char cwd[255];
	struct BLZ_Texture *texture;
	struct BLZ_RenderTarget *target;
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
	texture = BLZ_LoadTextureFromFile("test/jellybeans.png", AUTO, 0, NONE);
	if (texture == NULL)
	{
		BAIL_OUT("Could not load texture file!");
	}
	target = BLZ_CreateRenderTarget(WINDOW_WIDTH, WINDOW_HEIGHT);

	plan(2);
	ok(target != NULL);
	/* draw the scene into the render target */
	BLZ_BindRenderTarget(target);
	BLZ_SetClearColor(clearColor);
	BLZ_SetBlendMode(BLEND_NORMAL);
	BLZ_Clear();
	BLZ_DrawImmediate(texture, position, NULL, 0.0f, NULL, NULL, white, NONE);

	BLZ_BindRenderTarget(NULL);
	for (i = 0; i < 5; i++)
	{
		BLZ_Clear();
		BLZ_DrawImmediate(&target->texture, topLeft, NULL, 0.0f, NULL, NULL,
						  red, NONE);
		SDL_GL_SwapWindow(window);
	}
	/* create a screenshot and compare */
	ok(Validate_Output("test_render_target", 0.999f));

	BLZ_FreeRenderTarget(target);
	BLZ_FreeTexture(texture);
	Test_Shutdown();
	done_testing();
}
