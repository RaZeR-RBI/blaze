#include "blaze.h"
#include <stdlib.h>
#include <GL/glcorearb.h>

#define calloc(s) calloc(1, s)
#define malloc(n, s) malloc(n *s)

#define success()           \
	do                      \
	{                       \
		__lastError = NULL; \
		return 1;           \
	} while (0);
#define fail(msg)          \
	do                     \
	{                      \
		__lastError = msg; \
		return -1;         \
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

typedef struct
{
	GLfloat x, y, z;
	GLfloat padding;
	GLfloat r, g, b, a;
	GLfloat u, v;
} Vertex;

typedef struct
{
	GLuint vao, vbo;
} StaticBuffer;

struct BLZ_StaticBatch
{
	GLuint texture;
	StaticBuffer buffer;
};

typedef struct
{
	GLuint vao;
	GLuint vbo[3];
} DynamicBuffer;

typedef struct _StreamBatchList
{
	GLuint texture;
	struct _StreamBatchList *next;
	int quad_count;
	Vertex *vertices;
	DynamicBuffer buffer;
} StreamBatchList;

static int MAX_TEXTURES;
static int MAX_SPRITES_PER_TEXTURE;
static StreamBatchList *stream_batches = NULL;

static char *__lastError = NULL;

static StreamBatchList *alloc_stream_batch(int max_sprites_per_tex)
{
	StreamBatchList *result = calloc(sizeof(StreamBatchList));
	if (result == NULL)
	{
		return NULL;
	}
	result->vertices = malloc(max_sprites_per_tex * 4, sizeof(Vertex));
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
		StreamBatchList *cur = stream_batches;
		StreamBatchList *next = NULL;
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
	StreamBatchList *cur;
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
