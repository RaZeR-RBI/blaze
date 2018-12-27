#include "blaze.h"
#include "./deps/SOIL/SOIL.h"
#include "./glad/include/glad/glad.h"

#include <stdio.h>
#include <stdlib.h>
#include <GL/glcorearb.h>

#define calloc_one(s) calloc(1, s)
#define BUFFER_COUNT 3
#define HAS_FLAG(flag) (FLAGS & flag) == flag

#ifdef TEST
#define static
#endif

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

struct Vertex
{
	GLfloat x, y, z;
	GLfloat padding;
	GLfloat r, g, b, a;
	GLfloat u, v;
};

struct Buffer
{
	GLuint vao, vbo;
};

struct BLZ_StaticBatch
{
	GLuint texture;
	struct Buffer buffer;
};

struct StreamBatch
{
	GLuint texture;
	int quad_count;
	struct Vertex *vertices;
	struct Buffer buffer[BUFFER_COUNT];
};

static int MAX_TEXTURES;
static int MAX_SPRITES_PER_TEXTURE;
static enum BLZ_InitFlags FLAGS = 0;
static unsigned char BUFFER_INDEX = 0;
static struct StreamBatch *stream_batches = NULL;

static char *__lastError = NULL;

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

int BLZ_Load(glGetProcAddress loader)
{
	int result = gladLoadGLLoader((GLADloadproc)loader);
	fail_if_false(result, "Could not load the OpenGL library");
	success();
}

int BLZ_Shutdown()
{
	if (stream_batches != NULL)
	{
		/* TODO: Free OpenGL objects here */
		free(stream_batches);
	}
	MAX_TEXTURES = MAX_SPRITES_PER_TEXTURE = 0;
	success();
}

int BLZ_Init(int max_textures, int max_sprites_per_tex, enum BLZ_InitFlags flags)
{
	int i;
	const int VERT_SIZE = MAX_SPRITES_PER_TEXTURE * sizeof(struct Vertex) * 4;
	struct Vertex *vertices;
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
	stream_batches = calloc(MAX_TEXTURES, sizeof(struct StreamBatch));
	check_alloc(stream_batches);
	for (i = 0; i < MAX_TEXTURES; i++)
	{
		vertices = malloc(VERT_SIZE);
		check_alloc(vertices);
		(stream_batches + i)->vertices = vertices;
		/* TODO: Allocate OpenGL objects */
	}
	success();
}

int BLZ_Flush()
{
	fail("Not implemented");
}

int BLZ_Present()
{
	fail_if_false(BLZ_Flush(), "Could not flush the sprite queue");
	if (!HAS_FLAG(NO_TRIPLEBUFFER))
	{
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
	fail("Not implemented");
}

int BLZ_LoadTextureFromFile(
	const char *filename,
	enum BLZ_ImageChannels channels,
	unsigned int texture_id,
	enum BLZ_ImageFlags flags)
{
	return SOIL_load_OGL_texture(
		filename,
		channels,
		texture_id,
		flags);
}

int BLZ_LoadTextureFromMemory(
	const unsigned char *const buffer,
	int buffer_length,
	enum BLZ_ImageChannels force_channels,
	unsigned int texture_id,
	enum BLZ_ImageFlags flags)
{
	int width, height, channels;
	unsigned int result;
	unsigned char *data = SOIL_load_image_from_memory(
		buffer, buffer_length,
		&width, &height, &channels,
		force_channels);
	const char *last_result = SOIL_last_result();
	if (data == NULL)
	{
		printf("Error: %s\n", last_result);
	}
	fail_if_null(data, "Could not load image");
	result = SOIL_create_OGL_texture(
		data, width, height, channels, texture_id, flags);
	return_success(result);
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
