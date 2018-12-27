#include "blaze.h"
#include "./deps/SOIL/SOIL.h"
#include "./glad/include/glad/glad.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define calloc_one(s) calloc(1, s)
#define BUFFER_COUNT 3
#define HAS_FLAG(flag) (FLAGS & flag) == flag

#define return_success(result) \
	do                         \
	{                          \
		__lastError = NULL;    \
		return result;         \
	} while (0);
#define success() return_success(BLZ_TRUE)
#define fail(msg)          \
	do                     \
	{                      \
		__lastError = msg; \
		return BLZ_FALSE;  \
	} while (0);
#define fail_cmp(val, cmp, msg) \
	do                          \
	{                           \
		if (val == cmp)         \
		{                       \
			fail(msg);          \
		}                       \
	} while (0);
#define fail_if_null(val, msg) fail_cmp(val, NULL, msg)
#define fail_if_false(val, msg) fail_cmp(val, BLZ_FALSE, msg)
#define check_alloc(p) fail_if_null(p, "Could not allocate memory")
#define validate(expr)                                         \
	do                                                         \
	{                                                          \
		if (!(expr))                                           \
		{                                                      \
			fail("Invalid parameter value, should be " #expr); \
		}                                                      \
	} while (0);

struct Buffer
{
	GLuint vao, vbo;
};

struct BLZ_Texture
{
	GLuint id;
	int width, height;
};

struct BLZ_StaticBatch
{
	struct BLZ_Texture texture;
	struct Buffer buffer;
};

struct StreamBatch
{
	GLuint texture;
	int quad_count;
	struct BLZ_Vertex *vertices;
	struct Buffer buffer[BUFFER_COUNT];
};

static int MAX_TEXTURES;
static int MAX_SPRITES_PER_TEXTURE;
static enum BLZ_InitFlags FLAGS = 0;
static unsigned char BUFFER_INDEX = 0;
static struct StreamBatch *stream_batches = NULL;

static char *__lastError = NULL;

static GLfloat orthoMatrix[16] =
	{0, 0, 0, 0,
	 0, 0, 0, 0,
	 0, 0, 0, 0,
	 -1.0f, 1.0f, -1.0f, 1.0f};

static GLchar vertexSource[] =
	"#version 130\n"
	"uniform mat4 u_mvpMatrix;"
	"in vec4 in_Position;"
	"in vec4 in_Color;"
	"in vec2 in_Texcoord;"
	"out vec4 ex_Color;"
	"out vec2 ex_Texcoord;"
	"void main() {"
	"  ex_Color = in_Color;"
	"  ex_Texcoord = in_Texcoord;"
	"  gl_Position = u_mvpMatrix * in_Position;"
	"}";

static GLchar fragmentSource[] =
	"#version 130\n"
	"in vec4 ex_Color;"
	"in vec2 ex_Texcoord;"
	"out vec4 outColor;"
	"uniform sampler2D tex;"
	"void main() {"
	"  outColor = texture(tex, ex_Texcoord) * ex_Color;"
	"}";

static GLuint SHADER_DEFAULT;
static unsigned char FRAMESKIP = 0;

static struct Buffer create_buffer()
{
	struct Buffer result;
	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0); /* x|y|z|padding */
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0); /* r|g|b|a */
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); /* u|v */
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	result.vao = vao;
	result.vbo = vbo;
	return result;
}

static void free_buffer(struct Buffer buffer)
{
	glDeleteVertexArrays(1, &buffer.vao);
	glDeleteBuffers(1, &buffer.vbo);
}

/* Public API */
int BLZ_GetOptions(int *max_textures, int *max_sprites_per_tex,
				   enum BLZ_InitFlags *flags)
{
	if (MAX_TEXTURES <= 0 || MAX_SPRITES_PER_TEXTURE <= 0)
	{
		fail("Not initialized");
	}
	*max_textures = MAX_TEXTURES;
	*max_sprites_per_tex = MAX_SPRITES_PER_TEXTURE;
	*flags = FLAGS;
	success();
}

int BLZ_SetViewport(int w, int h)
{
	validate(w > 0);
	validate(h > 0);
	orthoMatrix[0] = 2.0f / (GLfloat)w;
	orthoMatrix[5] = -2.0f / (GLfloat)h;
	success();
}

static GLuint compile_shader(GLenum type, char *src)
{
	int compiled;
	int log_length;
	char *log_string;
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar **)&src, 0);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
		log_string = malloc(log_length);
		glGetShaderInfoLog(shader, log_length, &log_length, log_string);
		printf("Error compiling shader: %s\n", log_string);
		free(log_string);
		return 0;
	}
	return shader;
}

GLuint BLZ_CompileShader(char *vert, char *frag)
{
	GLuint program, vertex_shader, fragment_shader;
	int is_linked, log_length;
	char *log_string;
	vertex_shader = compile_shader(GL_VERTEX_SHADER, vert);
	fail_if_false(vertex_shader, "Could not compile vertex shader");
	fragment_shader = compile_shader(GL_FRAGMENT_SHADER, frag);
	fail_if_false(fragment_shader, "Could not compile fragment shader");
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glBindAttribLocation(program, 0, "in_Position");
	glBindAttribLocation(program, 1, "in_Color");
	glBindAttribLocation(program, 2, "in_Texcoord");
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
	if (!is_linked)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		log_string = malloc(log_length);
		glGetProgramInfoLog(program, log_length, &log_length, log_string);
		printf("Error linking shader: %s\n", log_string);
		free(log_string);
		return 0;
	}
	return program;
}

int BLZ_UseShader(GLuint program)
{
	GLenum result;
	glUseProgram(program);
	result = glGetError();
	if (result == GL_NO_ERROR)
	{
		success();
	}
	printf("glUseProgram: error %d\n", result);
	fail("Could not use shader program");
}

int BLZ_DeleteShader(GLuint program)
{
	glDeleteShader(program);
	success();
}

GLuint BLZ_GetDefaultShader()
{
	return SHADER_DEFAULT;
}

int BLZ_Load(glGetProcAddress loader)
{
	int result = gladLoadGLLoader((GLADloadproc)loader);
	fail_if_false(result, "Could not load the OpenGL library");
	SHADER_DEFAULT = BLZ_CompileShader(vertexSource, fragmentSource);
	fail_if_false(SHADER_DEFAULT, "Could not compile default shader");
	fail_if_false(BLZ_UseShader(SHADER_DEFAULT), "Could not use default shader");
	success();
}

int BLZ_Shutdown()
{
	int i;
	struct StreamBatch cur;
	if (stream_batches != NULL)
	{
		for (i = 0; i < MAX_TEXTURES; i++)
		{
			cur = *(stream_batches + i);
			free_buffer(cur.buffer[0]);
			if (!HAS_FLAG(NO_TRIPLEBUFFER))
			{
				free_buffer(cur.buffer[1]);
				free_buffer(cur.buffer[2]);
			}
		}
		free(stream_batches);
	}
	MAX_TEXTURES = MAX_SPRITES_PER_TEXTURE = 0;
	success();
}

int BLZ_Init(int max_textures, int max_sprites_per_tex, enum BLZ_InitFlags flags)
{
	int i;
	const int VERT_COUNT = MAX_SPRITES_PER_TEXTURE * 4;
	struct BLZ_Vertex *vertices;
	struct StreamBatch *cur;
	validate(max_textures > 0);
	validate(max_sprites_per_tex > 0);
	if (stream_batches != NULL)
	{
		BLZ_Shutdown();
	}
	MAX_SPRITES_PER_TEXTURE = max_sprites_per_tex;
	MAX_TEXTURES = max_textures;
	FLAGS = flags;
	BUFFER_INDEX = 0;
	if (!HAS_FLAG(NO_TRIPLEBUFFER))
	{
		FRAMESKIP = 1;
	}
	stream_batches = calloc(MAX_TEXTURES, sizeof(struct StreamBatch));
	check_alloc(stream_batches);
	for (i = 0; i < MAX_TEXTURES; i++)
	{
		cur = (stream_batches + i);
		vertices = calloc(VERT_COUNT, sizeof(struct BLZ_Vertex));
		check_alloc(vertices);
		cur->vertices = vertices;
		cur->buffer[0] = create_buffer();
		cur->buffer[1] = create_buffer();
		cur->buffer[2] = create_buffer();
	}
	success();
}

int BLZ_Flush()
{
	unsigned char to_draw, to_fill;
	struct StreamBatch batch;
	int i, buf_size;
	if (HAS_FLAG(NO_TRIPLEBUFFER) || FRAMESKIP)
	{
		to_draw = to_fill = 0;
	}
	else
	{
		to_draw = BUFFER_INDEX,
		to_fill = BUFFER_INDEX + 1;
		if (to_fill >= BUFFER_COUNT)
		{
			to_fill -= BUFFER_COUNT;
		}
	}
	for (i = 0; i < MAX_TEXTURES; i++)
	{
		batch = *(stream_batches + i);
		buf_size = batch.quad_count * 4 * sizeof(struct BLZ_Vertex);
		if (buf_size == 0 || batch.texture == 0)
		{
			/* we've reached the end of the queue */
			break;
		}
		/* fill the buffer */
		glBindBuffer(GL_ARRAY_BUFFER, batch.buffer[to_fill].vbo);
		glBufferData(GL_ARRAY_BUFFER, buf_size, batch.vertices, GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		/* bind the VAO and draw it */
		glBindVertexArray(batch.buffer[to_draw].vao);
		glDrawArrays(GL_TRIANGLES, 0, batch.quad_count * 4);
	}
	success();
}

int BLZ_Present()
{
	fail_if_false(BLZ_Flush(), "Could not flush the sprite queue");
	if (!HAS_FLAG(NO_TRIPLEBUFFER))
	{
		FRAMESKIP = 0;
		BUFFER_INDEX++;
		if (BUFFER_INDEX >= BUFFER_COUNT)
		{
			BUFFER_INDEX -= BUFFER_COUNT;
		}
	}
	success();
}

int BLZ_Draw(
	GLuint texture,
	struct BLZ_Vector2 *position,
	struct BLZ_Rectangle *srcRectangle,
	float rotation,
	struct BLZ_Vector2 *origin,
	struct BLZ_Vector2 *scale,
	enum BLZ_SpriteEffects effects,
	float layerDepth)
{
	/* TODO: Implement vertex calculation */
	fail("Not implemented");
}

int BLZ_LowerDraw(GLuint texture, struct BLZ_SpriteQuad *quad)
{
	struct StreamBatch *batch;
	int i;
	size_t offset;
	for (i = 0; i < MAX_TEXTURES; i++)
	{
		batch = (stream_batches + i);
		if (batch->texture == texture &&
			batch->quad_count >= MAX_SPRITES_PER_TEXTURE)
		{
			/* this batch is already filled, skip it */
			continue;
		}
		if (batch->texture == texture || batch->texture == 0)
		{
			/* we found existing not-full batch or an empty one */
			break;
		}
	}
	if (i >= MAX_TEXTURES && batch->texture > 0 && batch->texture != texture)
	{
		/* we ran out of limits and must flush and repeat */
		BLZ_Flush();
		BLZ_LowerDraw(texture, quad);
	}
	offset = batch->quad_count * 4;
	/* set the vertex data */
	memcpy((batch->vertices + offset), quad, sizeof(struct BLZ_SpriteQuad));
	batch->quad_count++;
	success();
}

static void fill_texture_info(struct BLZ_Texture *texture)
{
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texture->width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texture->height);
	glBindTexture(GL_TEXTURE_2D, 0);
}

BLZ_Texture *BLZ_LoadTextureFromFile(
	const char *filename,
	enum BLZ_ImageChannels channels,
	unsigned int texture_id,
	enum BLZ_ImageFlags flags)
{
	struct BLZ_Texture *texture;
	unsigned int id = SOIL_load_OGL_texture(
		filename,
		channels,
		texture_id,
		flags);
	if (!id)
	{
		return NULL;
	}
	texture = malloc(sizeof(struct BLZ_Texture));
	texture->id = id;
	fill_texture_info(texture);
	return texture;
}

BLZ_Texture *BLZ_LoadTextureFromMemory(
	const unsigned char *const buffer,
	int buffer_length,
	enum BLZ_ImageChannels force_channels,
	unsigned int texture_id,
	enum BLZ_ImageFlags flags)
{
	struct BLZ_Texture *texture;
	int width, height, channels;
	unsigned char *data = SOIL_load_image_from_memory(
		buffer, buffer_length,
		&width, &height, &channels,
		force_channels);
	const char *last_result = SOIL_last_result();
	if (data == NULL)
	{
		printf("Error: %s\n", last_result);
		return NULL;
	}
	texture = malloc(sizeof(BLZ_Texture));
	texture->id = SOIL_create_OGL_texture(
		data, width, height, channels, texture_id, flags);
	fill_texture_info(texture);
	return texture;
}

int BLZ_SaveScreenshot(
	const char *filename,
	enum BLZ_SaveImageFormat format,
	int x, int y,
	int width, int height)
{
	return SOIL_save_screenshot(
		filename,
		format, x, y, width, height);
}
