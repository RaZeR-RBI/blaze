#include "blaze.h"
#include "./deps/SOIL/SOIL.h"

#include <stdlib.h>
#include <GL/glcorearb.h>

#define calloc(s) calloc(1, s)
#define malloc(n, s) malloc(n *s)

#ifdef TEST
#define static
#endif

#define success()           \
	do                      \
	{                       \
		__lastError = NULL; \
		return BLZ_TRUE;    \
	} while (0);
#define fail(msg)          \
	do                     \
	{                      \
		__lastError = msg; \
		return BLZ_FALSE;  \
	} while (0);
#define fail_if_null(p, msg) \
	do                       \
	{                        \
		if (p == NULL)       \
		{                    \
			fail(msg);       \
		}                    \
	} while (0);
#define check_alloc(p) fail_if_null(p, "Could not allocate memory")

struct Vertex
{
	GLfloat x, y, z;
	GLfloat padding;
	GLfloat r, g, b, a;
	GLfloat u, v;
};

struct StaticBuffer
{
	GLuint vao, vbo;
};

struct BLZ_StaticBatch
{
	GLuint texture;
	struct StaticBuffer buffer;
};

struct DynamicBuffer
{
	GLuint vao;
	GLuint vbo[3];
};

struct StreamBatchList
{
	GLuint texture;
	struct StreamBatchList *next;
	int quad_count;
	struct Vertex *vertices;
	struct DynamicBuffer buffer;
};

static int MAX_TEXTURES;
static int MAX_SPRITES_PER_TEXTURE;
static struct StreamBatchList *stream_batches = NULL;

static char *__lastError = NULL;

static struct StreamBatchList *alloc_stream_batch(int max_sprites_per_tex)
{
	struct StreamBatchList *result = calloc(sizeof(struct StreamBatchList));
	if (result == NULL)
	{
		return NULL;
	}
	result->vertices = malloc(max_sprites_per_tex * 4, sizeof(struct Vertex));
	if (result->vertices == NULL)
	{
		return NULL;
	}
	return result;
}

/* Public API */
int BLZ_Shutdown()
{
	if (stream_batches != NULL)
	{
		struct StreamBatchList *cur = stream_batches;
		struct StreamBatchList *next = NULL;
		do
		{
			next = cur->next;
			free(cur->vertices);
			free(cur);
			cur = next;
		} while (next != NULL);
	}
	success();
}

int BLZ_Init(int max_textures, int max_sprites_per_tex)
{
	int i;
	struct StreamBatchList *cur;
	MAX_SPRITES_PER_TEXTURE = max_sprites_per_tex;
	MAX_TEXTURES = max_textures;
	if (stream_batches != NULL)
	{
		BLZ_Shutdown();
	}
	stream_batches = alloc_stream_batch(max_sprites_per_tex);
	check_alloc(stream_batches);
	cur = stream_batches;
	for (i = 1; i < max_textures; i++)
	{
		cur->next = alloc_stream_batch(max_sprites_per_tex);
		check_alloc(cur->next);
		cur = cur->next;
	}
	success();
}

int BLZ_Begin()
{
	fail("Not implemented");
}

int BLZ_Flush()
{
	fail("Not implemented");
}

int BLZ_End()
{
	BLZ_Flush();
	fail("Not implemented");
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
		flags
	);
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
	unsigned char* data = SOIL_load_image_from_memory(
		buffer, buffer_length,
		&width, &height, &channels,
		force_channels
	);
	fail_if_null(data, "Could not load the image")
	result = SOIL_create_OGL_texture(
		data, width, height, channels, texture_id, flags);
	return result;
}

int BLZ_SaveScreenshot(
	const char *filename,
	enum BLZ_SaveImageFormat format,
	int x, int y,
	int width, int height
)
{
	return SOIL_save_screenshot(
		filename,
		format, x, y, width, height
	);
}
