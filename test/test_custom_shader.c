#include "common.h"
#include "unistd.h"

struct BLZ_Vector4 clearColor = {0, 0, 0, 0};
struct BLZ_Vector4 white = {1, 1, 1, 1};
struct BLZ_Vector2 position = { 156, 156 };

/* simple color negate shader */
/* u_mvpMatrix is a model-view-projection matrix which transforms
 * supplied pixel coordinates into NDC, calculated by BLZ_SetViewport(...)
 */
static GLchar vertexSource[] =
	"#version 130\n"
	"uniform mat4 u_mvpMatrix;"
	"in vec2 in_Position;"
	"in vec2 in_Texcoord;"
	"in vec4 in_Color;"
	"out vec4 ex_Color;"
	"out vec2 ex_Texcoord;"
	"void main() {"
	"  ex_Color = in_Color;"
	"  ex_Texcoord = in_Texcoord;"
	"  gl_Position = u_mvpMatrix * vec4(in_Position, 1, 1);"
	"}";

/* sample the texture at passed coordinates, multiply by color specified
 * in BLZ_Draw(...) and then negate the RGB components
 */
static GLchar fragmentSource[] =
	"#version 130\n"
	"in vec4 ex_Color;"
	"in vec2 ex_Texcoord;"
	"out vec4 outColor;"
	"uniform sampler2D tex;"
	"void main() {"
	"  vec4 color = texture(tex, ex_Texcoord) * ex_Color;"
	"  outColor = vec4(1 - color.x, 1 - color.y, 1 - color.z, color.w);"
	"}";

int main(int argc, char *argv[])
{
	int i;
	char cwd[255];
	struct BLZ_SpriteBatch *batch;
	struct BLZ_Shader *shader;
	struct BLZ_Texture *texture;
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
	batch = BLZ_CreateBatch(1, 1, DEFAULT);
	BLZ_SetViewport(WINDOW_WIDTH, WINDOW_HEIGHT);
	texture = BLZ_LoadTextureFromFile("test/jellybeans.png", AUTO, 0, NONE);
	if (texture == NULL)
	{
		BAIL_OUT("Could not load texture file!");
	}
	/* compile and bind our shader */
	shader = BLZ_CompileShader(vertexSource, fragmentSource);
	if (shader == NULL)
	{
		BAIL_OUT("Could not compile shader!");
	}
	if (!BLZ_UseShader(shader)) {
		BAIL_OUT("Could not use the specified shader!");
	}

	plan(2);
	ok(shader != BLZ_GetDefaultShader());
	/* draw the scene */
	BLZ_SetClearColor(clearColor);
	BLZ_SetBlendMode(BLEND_NORMAL);
	for (i = 0; i < 5; i++)
	{
		BLZ_Clear(COLOR_BUFFER);
		BLZ_Draw(batch, texture, position, NULL, 0.0f, NULL, NULL, white, NONE);
		BLZ_Present(batch);
		SDL_GL_SwapWindow(window);
	}
	/* create a screenshot and compare */
	ok(Validate_Output("test_custom_shader", 0.999f));

	BLZ_FreeShader(shader);
	BLZ_FreeTexture(texture);
	BLZ_FreeBatch(batch);
	Test_Shutdown();
	done_testing();
}
