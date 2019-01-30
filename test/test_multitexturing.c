#include "common.h"
#include "unistd.h"

struct BLZ_Vector4 clearColor = {0, 0, 0, 0};
struct BLZ_Vector4 white = {1, 1, 1, 1};
struct BLZ_SpriteQuad quad = {{{20, 20, 0, 0, 1, 1, 1, 1},
							   {20, 480, 0, 1.5f, 1, 1, 1, 1},
							   {480, 20, 1.5f, 0, 1, 1, 1, 1},
							   {480, 480, 1.5f, 1.5f, 1, 1, 1, 1}}};

/* multitexturing effect shader */
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

/* "color burn" effect similar to Photoshop */
static GLchar fragmentSource[] =
	"#version 130\n"
	"in vec4 ex_Color;"
	"in vec2 ex_Texcoord;"
	"out vec4 outColor;"
	"uniform sampler2D tex;"
	"uniform sampler2D tex2;"
	"void main() {"
	"  vec4 color = texture(tex, ex_Texcoord) * ex_Color;"
	"  vec4 color2 = texture(tex2, ex_Texcoord);"
	"  outColor = (color + color2) - vec4(1, 1, 1, 1);"
	"}";

int main(int argc, char *argv[])
{
	int i;
	char cwd[255];
	struct BLZ_Shader *shader;
	struct BLZ_Texture *texture, *texture2, *tex_ignored;
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
	texture2 = BLZ_LoadTextureFromFile("test/pnggrad8rgb.png", AUTO, 0, NONE);
	tex_ignored = BLZ_LoadTextureFromFile("test/stripes_200px.png", AUTO, 0, NONE);
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
	if (!BLZ_UseShader(shader))
	{
		BAIL_OUT("Could not use the specified shader!");
	}

	plan(2);
	ok(BLZ_GetMaxTextureSlots());
	BLZ_SetTextureFiltering(texture, NEAREST, NEAREST);
	BLZ_SetTextureWrap(texture, CLAMP_TO_EDGE, CLAMP_TO_EDGE);
	/* draw the scene */
	BLZ_SetClearColor(clearColor);
	BLZ_SetBlendMode(BLEND_NORMAL);

	/* first slot (zero) is bound here to test texture overriding */
	/* when DrawImmediate is called, the texture passed to it will be replaced
	* with the one bound to slot 0, if it's unbound, the passed one is used
	*/
	BLZ_Uniform1i(BLZ_GetUniformLocation(shader, "tex"), 0);
	BLZ_Uniform1i(BLZ_GetUniformLocation(shader, "tex2"), 1);
	BLZ_BindTexture(texture, 0);
	BLZ_BindTexture(texture2, 1);
	for (i = 0; i < 5; i++)
	{
		BLZ_Clear(COLOR_BUFFER);
		BLZ_LowerDrawImmediate(tex_ignored->id, &quad);
		SDL_GL_SwapWindow(window);
	}
	/* create a screenshot and compare */
	ok(Validate_Output("test_multitexturing", 0.999f));
	BLZ_BindTexture(NULL, 0);
	BLZ_BindTexture(NULL, 1);
	BLZ_FreeShader(shader);
	BLZ_FreeTexture(texture);
	Test_Shutdown();
	done_testing();
}
