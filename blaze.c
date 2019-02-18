#include "blaze.h"
#include "./deps/SOIL/SOIL.h"
#include "./glad/include/glad/glad.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define calloc_one(s) calloc(1, s)
#define BUFFER_COUNT 2
#define HAS_FLAG(batch, flag) ((batch->flags & flag) == flag)

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
#define null_if_false(val, msg) \
	do                          \
	{                           \
		if (val == BLZ_FALSE)   \
		{                       \
			__lastError = msg;  \
			return NULL;        \
		}                       \
	} while (0);
#define check_alloc(p) fail_if_null(p, "Could not allocate memory")
#define validate(expr)                                         \
	do                                                         \
	{                                                          \
		if (!(expr))                                           \
		{                                                      \
			fail("Invalid parameter value, should be " #expr); \
		}                                                      \
	} while (0);
#define null_if_invalid(expr)                                          \
	do                                                                 \
	{                                                                  \
		if (!(expr))                                                   \
		{                                                              \
			__lastError = "Invalid parameter value, should be " #expr; \
			return NULL;                                               \
		}                                                              \
	} while (0);

/* Public constants */
const struct BLZ_BlendFunc BLEND_NORMAL = {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};
const struct BLZ_BlendFunc BLEND_ADDITIVE = {GL_ONE, GL_ONE};
const struct BLZ_BlendFunc BLEND_MULTIPLY = {GL_DST_COLOR, GL_ZERO};

/* Internal values */
struct Buffer
{
	GLuint vao, vbo, ebo;
};

struct BLZ_StaticBatch
{
	int sprite_count;
	int max_sprite_count;
	unsigned char is_uploaded;
	struct BLZ_Vertex *vertices;
	struct Buffer buffer;
	const struct BLZ_Texture *texture;
};

struct BLZ_SpriteBatch
{
	int max_buckets;
	int max_sprites_per_bucket;
	unsigned char buffer_index;
	unsigned char frameskip;
	enum BLZ_InitFlags flags;
	struct SpriteBucket *sprite_buckets;
};

struct BLZ_Shader
{
	GLuint program;
	GLint mvp_param;
};

struct SpriteBucket
{
	GLuint texture;
	int sprite_count;
	struct BLZ_Vertex *vertices;
	struct Buffer buffer[BUFFER_COUNT];
};

static char *__lastError = NULL;

#define Z_NEAR 0.9f
#define Z_FAR 2.1f
#define LAYER_DEPTH(depth) 1.0f + depth

static GLfloat orthoMatrix[16] =
	{0, 0, 0, 0,
	 0, 0, 0, 0,
	 0, 0, 1, 0,
	 -1, 1, 0, 1};

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

static GLchar fragmentSource[] =
	"#version 130\n"
	"in vec4 ex_Color;"
	"in vec2 ex_Texcoord;"
	"out vec4 outColor;"
	"uniform sampler2D tex;"
	"void main() {"
	"  outColor = texture(tex, ex_Texcoord) * ex_Color;"
	"}";

static BLZ_Shader *SHADER_DEFAULT;
static BLZ_Shader *SHADER_CURRENT;
static struct Buffer immediateBuf;
static GLuint tex0_override = 0;

static const int VERT_SIZE = sizeof(struct BLZ_Vertex);

/* TODO: Optimization: Reuse same VAO for all batches to minimize state changes */
static struct Buffer create_buffer(int max_sprites, GLenum usage)
{
	int INDICES_SIZE = max_sprites * 6 * sizeof(GLushort);
	int i;
	struct Buffer result;
	GLushort *indices = malloc(INDICES_SIZE);
	GLuint vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, VERT_SIZE * 4 * max_sprites,
				 NULL, usage);
	/* x|y */
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, VERT_SIZE, (void *)0);
	/* u|v */
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VERT_SIZE, (void *)8);
	/* r|g|b|a */
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VERT_SIZE, (void *)16);
	/* indices */
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	for (i = 0; i < max_sprites; i++)
	{
		*(indices + (i * 6)) = (GLushort)(i * 4);
		*(indices + (i * 6) + 1) = (GLushort)(i * 4 + 1);
		*(indices + (i * 6) + 2) = (GLushort)(i * 4 + 2);
		*(indices + (i * 6) + 3) = (GLushort)(i * 4 + 2);
		*(indices + (i * 6) + 4) = (GLushort)(i * 4 + 1);
		*(indices + (i * 6) + 5) = (GLushort)(i * 4 + 3);
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDICES_SIZE, indices, GL_STATIC_DRAW);
	free(indices);
	glBindVertexArray(0);
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
char* BLZ_GetLastError()
{
	return __lastError;
}

int BLZ_Load(glGetProcAddress loader)
{
	int result = gladLoadGLLoader((GLADloadproc)loader);
	fail_if_false(result, "Could not load the OpenGL library");
	SHADER_DEFAULT = BLZ_CompileShader(vertexSource, fragmentSource);
	fail_if_false(SHADER_DEFAULT, "Could not compile default shader");
	fail_if_false(BLZ_UseShader(SHADER_DEFAULT), "Could not use default shader");
	immediateBuf = create_buffer(1, GL_STREAM_DRAW);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	success();
}

int BLZ_GetMaxTextureSlots()
{
	GLint result;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &result);
	return result;
}

int BLZ_BindTexture(struct BLZ_Texture *texture, int slot)
{
	GLuint id = texture == NULL ? 0 : texture->id;
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, id);
	if (slot == 0)
	{
		tex0_override = id;
	}
	success();
}

int BLZ_SetTextureFiltering(
	struct BLZ_Texture *texture,
	enum BLZ_TextureFilter minification,
	enum BLZ_TextureFilter magnification)
{
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minification);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnification);
	glBindTexture(GL_TEXTURE_2D, 0);
	success();
}

int BLZ_SetTextureWrap(
	struct BLZ_Texture *texture,
	enum BLZ_TextureWrap x,
	enum BLZ_TextureWrap y)
{
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, x);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, y);
	glBindTexture(GL_TEXTURE_2D, 0);
	success();
}

static void bind_tex0(GLuint tex)
{
	if (tex0_override == 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
	}
}

int BLZ_GetOptions(const struct BLZ_SpriteBatch *batch,
				   int *max_buckets, int *max_sprites_per_bucket,
				   enum BLZ_InitFlags *flags)
{
	validate(batch != NULL);
	if (batch->max_buckets <= 0 || batch->max_sprites_per_bucket <= 0)
	{
		fail("Not initialized");
	}
	*max_buckets = batch->max_buckets;
	*max_sprites_per_bucket = batch->max_sprites_per_bucket;
	*flags = batch->flags;
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

void BLZ_SetClearColor(struct BLZ_Vector4 color)
{
	glClearColor(color.x, color.y, color.z, color.w);
}

void BLZ_SetBlendMode(const struct BLZ_BlendFunc func)
{
	glBlendFunc(func.source, func.destination);
}

void BLZ_Clear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

static GLuint compile_shader(GLenum type, const char *src)
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

GLint BLZ_GetUniformLocation(const struct BLZ_Shader *shader, const char *name)
{
	if (shader == NULL || name == NULL)
	{
		return -1;
	}
	return glGetUniformLocation(shader->program, (const GLchar *)name);
}

BLZ_Shader *BLZ_CompileShader(const char *vert, const char *frag)
{
	struct BLZ_Shader *shader;
	GLuint program, vertex_shader, fragment_shader;
	int is_linked, log_length;
	char *log_string;
	validate(vert != NULL);
	validate(frag != NULL);
	shader = malloc(sizeof(struct BLZ_Shader));
	vertex_shader = compile_shader(GL_VERTEX_SHADER, vert);
	if (!vertex_shader)
	{
		return NULL;
	}
	fragment_shader = compile_shader(GL_FRAGMENT_SHADER, frag);
	if (!fragment_shader)
	{
		return NULL;
	}
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glBindAttribLocation(program, 0, "in_Position");
	glBindAttribLocation(program, 1, "in_Texcoord");
	glBindAttribLocation(program, 2, "in_Color");
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
	if (!is_linked)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		log_string = malloc(log_length);
		glGetProgramInfoLog(program, log_length, &log_length, log_string);
		printf("Error linking shader: %s\n", log_string);
		free(log_string);
		return NULL;
	}
	shader->program = program;
	shader->mvp_param = BLZ_GetUniformLocation(shader, "u_mvpMatrix");
	return shader;
}

int BLZ_UseShader(struct BLZ_Shader *program)
{
	GLenum result;
	validate(program != NULL);
	/* clear previous errors to make sure we're reading the actual one */
	while (glGetError() != GL_NO_ERROR)
	{
	};
	glUseProgram(program->program);
	result = glGetError();
	if (result == GL_NO_ERROR)
	{
		SHADER_CURRENT = program;
		success();
	}
	printf("glUseProgram: error %d\n", result);
	fail("Could not use shader program");
}

int BLZ_FreeShader(BLZ_Shader *program)
{
	validate(program != NULL);
	glDeleteShader(program->program);
	free(program);
	success();
}

BLZ_Shader *BLZ_GetDefaultShader()
{
	return SHADER_DEFAULT;
}

int BLZ_FreeBatch(struct BLZ_SpriteBatch *batch)
{
	int i;
	struct SpriteBucket cur;
	if (batch->sprite_buckets != NULL)
	{
		for (i = 0; i < batch->max_buckets; i++)
		{
			cur = *(batch->sprite_buckets + i);
			free_buffer(cur.buffer[0]);
			if (!HAS_FLAG(batch, NO_BUFFERING))
			{
				free_buffer(cur.buffer[1]);
				free_buffer(cur.buffer[2]);
			}
		}
		free(batch->sprite_buckets);
	}
	free(batch);
	success();
}

struct BLZ_SpriteBatch *BLZ_CreateBatch(
	int max_buckets, int max_sprites_per_bucket, enum BLZ_InitFlags flags)
{
	int i;
	struct BLZ_Vertex *vertices;
	struct SpriteBucket *cur;
	struct BLZ_SpriteBatch *batch = malloc(sizeof(BLZ_SpriteBatch));
	null_if_invalid(max_buckets > 0);
	null_if_invalid(max_sprites_per_bucket > 0);
	batch->max_sprites_per_bucket = max_sprites_per_bucket;
	batch->max_buckets = max_buckets;
	batch->flags = flags;
	batch->buffer_index = 0;
	if (!HAS_FLAG(batch, NO_BUFFERING))
	{
		batch->frameskip = 1;
	}
	batch->sprite_buckets = calloc(batch->max_buckets, sizeof(struct SpriteBucket));
	check_alloc(batch->sprite_buckets);
	for (i = 0; i < batch->max_buckets; i++)
	{
		cur = (batch->sprite_buckets + i);
		vertices = malloc(batch->max_sprites_per_bucket * 4 * sizeof(struct BLZ_Vertex));
		check_alloc(vertices);
		cur->vertices = vertices;
		cur->buffer[0] = create_buffer(max_sprites_per_bucket, GL_STREAM_DRAW);
		cur->buffer[1] = create_buffer(max_sprites_per_bucket, GL_STREAM_DRAW);
	}
	return batch;
}

static inline void set_mvp_matrix(const GLfloat *matrix)
{
	if (SHADER_CURRENT->mvp_param > -1)
	{
		BLZ_UniformMatrix4fv(SHADER_CURRENT->mvp_param, 1, GL_FALSE, matrix);
	}
}

static struct BLZ_SpriteBatch *__lastBatch;
static struct SpriteBucket *__lastBucket;
static GLuint __lastTexture;
static int flush(struct BLZ_SpriteBatch *batch)
{
	unsigned char to_draw, to_fill;
	struct SpriteBucket bucket;
	struct SpriteBucket *bucket_ptr;
	int i, buf_size;
	set_mvp_matrix((const GLfloat *)&orthoMatrix);
	if (HAS_FLAG(batch, NO_BUFFERING) || batch->frameskip)
	{
		to_draw = to_fill = 0;
	}
	else
	{
		to_draw = batch->buffer_index,
		to_fill = batch->buffer_index + 1;
		if (to_fill >= BUFFER_COUNT)
		{
			to_fill -= BUFFER_COUNT;
		}
	}
	for (i = 0; i < batch->max_buckets; i++)
	{
		bucket_ptr = (batch->sprite_buckets + i);
		bucket = *bucket_ptr;
		buf_size = bucket.sprite_count * 4 * sizeof(struct BLZ_Vertex);
		if (buf_size == 0 || bucket.texture == 0)
		{
			/* we've reached the end of the batch */
			break;
		}
		/* fill the buffer */
		glBindBuffer(GL_ARRAY_BUFFER, bucket.buffer[to_fill].vbo);
		glBufferData(GL_ARRAY_BUFFER, buf_size, bucket.vertices, GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		/* bind our texture and the VAO and draw it */
		bind_tex0(bucket.texture);
		glBindVertexArray(bucket.buffer[to_draw].vao);
		glDrawElements(GL_TRIANGLES, bucket.sprite_count * 6, GL_UNSIGNED_SHORT, (void *)0);
		bucket_ptr->sprite_count = 0;
		bucket_ptr->texture = 0;
	}
	__lastBatch = NULL;
	__lastBucket = NULL;
	__lastTexture = 0;
	success();
}

int BLZ_Present(struct BLZ_SpriteBatch *batch)
{
	fail_if_false(flush(batch), "Could not flush the sprite batch");
	if (!HAS_FLAG(batch, NO_BUFFERING) && batch->frameskip == 0)
	{
		batch->buffer_index++;
		if (batch->buffer_index >= BUFFER_COUNT)
		{
			batch->buffer_index -= BUFFER_COUNT;
		}
	}
	if (batch->frameskip > 0)
	{
		batch->frameskip--;
	}
	success();
}

#define set_vertex(index, field, val)     \
	do                                    \
	{                                     \
		quad.vertices[index].field = val; \
	} while (0);

#define set_all_vertices(field, val) \
	do                               \
	{                                \
		set_vertex(0, field, val);   \
		set_vertex(1, field, val);   \
		set_vertex(2, field, val);   \
		set_vertex(3, field, val);   \
	} while (0);

#define swap(tmp, one, two) \
	do                      \
	{                       \
		tmp = one;          \
		one = two;          \
		two = tmp;          \
	} while (0);

static struct BLZ_SpriteQuad transform_position_fastpath(
	const struct BLZ_Texture *texture,
	const struct BLZ_Vector2 position,
	const struct BLZ_Vector4 color)
{
	GLfloat x = position.x;
	GLfloat y = position.y;
	GLfloat w = (GLfloat)texture->width;
	GLfloat h = (GLfloat)texture->height;
	struct BLZ_SpriteQuad quad;

	/* set the vertex values */
	set_vertex(0, x, x);
	set_vertex(0, y, y);
	set_vertex(0, u, 0);
	set_vertex(0, v, 0);
	set_vertex(2, x, x + w);
	set_vertex(2, y, y);
	set_vertex(2, u, 1);
	set_vertex(2, v, 0);
	set_vertex(3, x, x + w);
	set_vertex(3, y, y + h);
	set_vertex(3, u, 1);
	set_vertex(3, v, 1);
	set_vertex(1, x, x);
	set_vertex(1, y, y + h);
	set_vertex(1, u, 0);
	set_vertex(1, v, 1);

	set_all_vertices(r, color.x);
	set_all_vertices(g, color.y);
	set_all_vertices(b, color.z);
	set_all_vertices(a, color.w);
	return quad;
}

static struct BLZ_SpriteQuad transform_full(
	const struct BLZ_Texture *texture,
	const struct BLZ_Vector2 position,
	const struct BLZ_Rectangle *srcRectangle,
	float rotation,
	const struct BLZ_Vector2 *origin,
	const struct BLZ_Vector2 *scale,
	const struct BLZ_Vector4 color,
	enum BLZ_SpriteFlip effects)
{
	/* position: top-left, top-right, bottom-left, bottom-right */
	struct BLZ_Vector2 p_tl, p_tr, p_bl, p_br;
	/* same for texture coordinates */
	struct BLZ_Vector2 t_tl, t_tr, t_bl, t_br;
	struct BLZ_SpriteQuad quad;
	GLfloat dx, dy;
	GLfloat x = position.x;
	GLfloat y = position.y;
	GLfloat tw = (GLfloat)texture->width;
	GLfloat th = (GLfloat)texture->height;
	int w = srcRectangle == NULL ? tw : srcRectangle->w;
	int h = srcRectangle == NULL ? th : srcRectangle->h;
	GLfloat _sin = rotation == 0.0f ? 0.0f : sin(rotation);
	GLfloat _cos = rotation == 0.0f ? 1.0f : cos(rotation);
	GLfloat u1 = srcRectangle == NULL ? 0 : srcRectangle->x / tw;
	GLfloat v1 = srcRectangle == NULL ? 0 : srcRectangle->y / th;
	GLfloat u2 = srcRectangle == NULL ? 1 : u1 + (srcRectangle->w / tw);
	GLfloat v2 = srcRectangle == NULL ? 1 : v1 + (srcRectangle->h / th);
	GLfloat tmp;
	if (scale != NULL)
	{
		w *= scale->x;
		h *= scale->y;
	}
	if (origin == NULL)
	{
		dx = 0;
		dy = 0;
	}
	else
	{
		dx = -origin->x;
		dy = -origin->y;
	}
	p_tl.x = x + dx * _cos - dy * _sin;
	p_tl.y = y + dx * _sin + dy * _cos;
	p_tr.x = x + (dx + w) * _cos - dy * _sin;
	p_tr.y = y + (dx + w) * _sin + dy * _cos;
	p_bl.x = x + dx * _cos - (dy + h) * _sin;
	p_bl.y = y + dx * _sin + (dy + h) * _cos;
	p_br.x = x + (dx + w) * _cos - (dy + h) * _sin;
	p_br.y = y + (dx + w) * _sin + (dy + h) * _cos;

	/* calculate texture coordinates */
	switch (effects)
	{
	case FLIP_H:
		swap(tmp, u1, u2);
		break;
	case FLIP_V:
		swap(tmp, v1, v2);
		break;
	case BOTH:
		swap(tmp, u1, u2);
		swap(tmp, v1, v2);
		break;
	default:
		break;
	}
	t_tl.x = u1;
	t_tl.y = v1;
	t_tr.x = u2;
	t_tr.y = v1;
	t_bl.x = u1;
	t_bl.y = v2;
	t_br.x = u2;
	t_br.y = v2;
	/* set the vertex values */
	set_vertex(0, x, p_tl.x);
	set_vertex(0, y, p_tl.y);
	set_vertex(0, u, t_tl.x);
	set_vertex(0, v, t_tl.y);
	set_vertex(2, x, p_tr.x);
	set_vertex(2, y, p_tr.y);
	set_vertex(2, u, t_tr.x);
	set_vertex(2, v, t_tr.y);
	set_vertex(3, x, p_br.x);
	set_vertex(3, y, p_br.y);
	set_vertex(3, u, t_br.x);
	set_vertex(3, v, t_br.y);
	set_vertex(1, x, p_bl.x);
	set_vertex(1, y, p_bl.y);
	set_vertex(1, u, t_bl.x);
	set_vertex(1, v, t_bl.y);

	set_all_vertices(r, color.x);
	set_all_vertices(g, color.y);
	set_all_vertices(b, color.z);
	set_all_vertices(a, color.w);
	return quad;
}

inline static struct BLZ_SpriteQuad transform(
	const struct BLZ_Texture *texture,
	const struct BLZ_Vector2 position,
	const struct BLZ_Rectangle *srcRectangle,
	float rotation,
	const struct BLZ_Vector2 *origin,
	const struct BLZ_Vector2 *scale,
	const struct BLZ_Vector4 color,
	enum BLZ_SpriteFlip effects)
{
	if (srcRectangle == NULL && rotation == 0.0f && origin == NULL &&
		scale == NULL && effects == NONE)
	{
		return transform_position_fastpath(texture, position, color);
	}
	else
	{
		return transform_full(texture, position, srcRectangle, rotation,
							  origin, scale, color, effects);
	}
}

int BLZ_Draw(
	struct BLZ_SpriteBatch *batch,
	const struct BLZ_Texture *texture,
	const struct BLZ_Vector2 position,
	const struct BLZ_Rectangle *srcRectangle,
	float rotation,
	const struct BLZ_Vector2 *origin,
	const struct BLZ_Vector2 *scale,
	const struct BLZ_Vector4 color,
	enum BLZ_SpriteFlip effects)
{
	struct BLZ_SpriteQuad quad = transform(
		texture,
		position,
		srcRectangle,
		rotation,
		origin,
		scale,
		color,
		effects);
	return BLZ_LowerDraw(batch, texture->id, &quad);
}

int BLZ_LowerDraw(
	struct BLZ_SpriteBatch *batch,
	GLuint texture, const struct BLZ_SpriteQuad *quad)
{
	struct SpriteBucket *bucket = NULL;
	int i = 0;
	size_t offset;
	if (__lastBatch != batch)
	{
		__lastBatch = NULL;
		__lastBucket = NULL;
		__lastTexture = 0;
	}
	if (__lastTexture > 0 && texture == __lastTexture)
	{
		if (__lastBucket != NULL && __lastBucket->sprite_count < batch->max_sprites_per_bucket)
		{
			bucket = __lastBucket;
		}
	}
	if (bucket == NULL)
	{
		for (i = 0; i < batch->max_buckets; i++)
		{
			bucket = (batch->sprite_buckets + i);
			if (bucket->texture == texture &&
				bucket->sprite_count >= batch->max_sprites_per_bucket)
			{
				/* this bucket is already filled, skip it */
				continue;
			}
			if (bucket->texture == texture || bucket->texture == 0)
			{
				/* we found existing not-full bucket or an empty one */
				break;
			}
		}
	}
	if (bucket->sprite_count >= batch->max_sprites_per_bucket ||
		(bucket->texture != 0 && bucket->texture != texture))
	{
		/* we ran out of limits */
		fail("Sprite limit reached - increase limits in BLZ_CreateBatch(...)");
	}
	offset = bucket->sprite_count * 4;
	/* set the vertex data */
	memcpy((bucket->vertices + offset), quad, sizeof(struct BLZ_SpriteQuad));
	bucket->sprite_count++;
	bucket->texture = texture;
	__lastBatch = batch;
	__lastBucket = bucket;
	__lastTexture = texture;
	success();
}

/* Static drawing */
static void upload_static_vertices(struct BLZ_StaticBatch *batch)
{
	glBindBuffer(GL_ARRAY_BUFFER, batch->buffer.vbo);
	glBufferData(GL_ARRAY_BUFFER,
				 batch->sprite_count * 4 * sizeof(struct BLZ_Vertex),
				 batch->vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	batch->is_uploaded = BLZ_TRUE;
}

struct BLZ_StaticBatch *BLZ_CreateStatic(
	const struct BLZ_Texture *texture, int max_sprite_count)
{
	struct BLZ_StaticBatch *result = malloc(sizeof(struct BLZ_StaticBatch));
	result->texture = texture;
	result->buffer = create_buffer(max_sprite_count, GL_STATIC_DRAW);
	result->is_uploaded = BLZ_FALSE;
	result->sprite_count = 0;
	result->max_sprite_count = max_sprite_count;
	result->vertices = malloc(max_sprite_count * 4 * sizeof(struct BLZ_Vertex));
	return result;
}

int BLZ_GetOptionsStatic(
	const struct BLZ_StaticBatch *batch,
	int *max_sprite_count)
{
	validate(batch != NULL);
	*max_sprite_count = batch->max_sprite_count;
	success();
}

int BLZ_FreeBatchStatic(
	struct BLZ_StaticBatch *batch)
{
	if (batch == NULL)
	{
		success();
	}
	free(batch->vertices);
	free_buffer(batch->buffer);
	free(batch);
	success();
}

int BLZ_DrawStatic(
	struct BLZ_StaticBatch *batch,
	const struct BLZ_Vector2 position,
	const struct BLZ_Rectangle *srcRectangle,
	float rotation,
	const struct BLZ_Vector2 *origin,
	const struct BLZ_Vector2 *scale,
	const struct BLZ_Vector4 color,
	enum BLZ_SpriteFlip effects)
{
	struct BLZ_SpriteQuad quad = transform(
		batch->texture,
		position,
		srcRectangle,
		rotation,
		origin,
		scale,
		color,
		effects);
	return BLZ_LowerDrawStatic(batch, &quad);
}

int BLZ_LowerDrawStatic(
	struct BLZ_StaticBatch *batch,
	const struct BLZ_SpriteQuad *quad)
{
	if (batch->is_uploaded)
	{
		fail("Can't push more sprites to already uploaded static batch");
	}
	if (batch->sprite_count >= batch->max_sprite_count)
	{
		fail("Sprite limit reached - increase limits in BLZ_CreateStatic(...)");
	}
	/* set the vertex data */
	memcpy((batch->vertices + batch->sprite_count * 4), quad, sizeof(struct BLZ_SpriteQuad));
	batch->sprite_count++;
	success();
}

static GLfloat identityMatrix[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1};

#define O(y, x) (y + (x << 2))
static void mult_4x4_matrix(const GLfloat *restrict src1, const GLfloat *restrict src2, GLfloat *restrict dest)
{
	*(dest + O(0, 0)) = (*(src1 + O(0, 0)) * *(src2 + O(0, 0))) + (*(src1 + O(0, 1)) * *(src2 + O(1, 0))) + (*(src1 + O(0, 2)) * *(src2 + O(2, 0))) + (*(src1 + O(0, 3)) * *(src2 + O(3, 0)));
	*(dest + O(0, 1)) = (*(src1 + O(0, 0)) * *(src2 + O(0, 1))) + (*(src1 + O(0, 1)) * *(src2 + O(1, 1))) + (*(src1 + O(0, 2)) * *(src2 + O(2, 1))) + (*(src1 + O(0, 3)) * *(src2 + O(3, 1)));
	*(dest + O(0, 2)) = (*(src1 + O(0, 0)) * *(src2 + O(0, 2))) + (*(src1 + O(0, 1)) * *(src2 + O(1, 2))) + (*(src1 + O(0, 2)) * *(src2 + O(2, 2))) + (*(src1 + O(0, 3)) * *(src2 + O(3, 2)));
	*(dest + O(0, 3)) = (*(src1 + O(0, 0)) * *(src2 + O(0, 3))) + (*(src1 + O(0, 1)) * *(src2 + O(1, 3))) + (*(src1 + O(0, 2)) * *(src2 + O(2, 3))) + (*(src1 + O(0, 3)) * *(src2 + O(3, 3)));
	*(dest + O(1, 0)) = (*(src1 + O(1, 0)) * *(src2 + O(0, 0))) + (*(src1 + O(1, 1)) * *(src2 + O(1, 0))) + (*(src1 + O(1, 2)) * *(src2 + O(2, 0))) + (*(src1 + O(1, 3)) * *(src2 + O(3, 0)));
	*(dest + O(1, 1)) = (*(src1 + O(1, 0)) * *(src2 + O(0, 1))) + (*(src1 + O(1, 1)) * *(src2 + O(1, 1))) + (*(src1 + O(1, 2)) * *(src2 + O(2, 1))) + (*(src1 + O(1, 3)) * *(src2 + O(3, 1)));
	*(dest + O(1, 2)) = (*(src1 + O(1, 0)) * *(src2 + O(0, 2))) + (*(src1 + O(1, 1)) * *(src2 + O(1, 2))) + (*(src1 + O(1, 2)) * *(src2 + O(2, 2))) + (*(src1 + O(1, 3)) * *(src2 + O(3, 2)));
	*(dest + O(1, 3)) = (*(src1 + O(1, 0)) * *(src2 + O(0, 3))) + (*(src1 + O(1, 1)) * *(src2 + O(1, 3))) + (*(src1 + O(1, 2)) * *(src2 + O(2, 3))) + (*(src1 + O(1, 3)) * *(src2 + O(3, 3)));
	*(dest + O(2, 0)) = (*(src1 + O(2, 0)) * *(src2 + O(0, 0))) + (*(src1 + O(2, 1)) * *(src2 + O(1, 0))) + (*(src1 + O(2, 2)) * *(src2 + O(2, 0))) + (*(src1 + O(2, 3)) * *(src2 + O(3, 0)));
	*(dest + O(2, 1)) = (*(src1 + O(2, 0)) * *(src2 + O(0, 1))) + (*(src1 + O(2, 1)) * *(src2 + O(1, 1))) + (*(src1 + O(2, 2)) * *(src2 + O(2, 1))) + (*(src1 + O(2, 3)) * *(src2 + O(3, 1)));
	*(dest + O(2, 2)) = (*(src1 + O(2, 0)) * *(src2 + O(0, 2))) + (*(src1 + O(2, 1)) * *(src2 + O(1, 2))) + (*(src1 + O(2, 2)) * *(src2 + O(2, 2))) + (*(src1 + O(2, 3)) * *(src2 + O(3, 2)));
	*(dest + O(2, 3)) = (*(src1 + O(2, 0)) * *(src2 + O(0, 3))) + (*(src1 + O(2, 1)) * *(src2 + O(1, 3))) + (*(src1 + O(2, 2)) * *(src2 + O(2, 3))) + (*(src1 + O(2, 3)) * *(src2 + O(3, 3)));
	*(dest + O(3, 0)) = (*(src1 + O(3, 0)) * *(src2 + O(0, 0))) + (*(src1 + O(3, 1)) * *(src2 + O(1, 0))) + (*(src1 + O(3, 2)) * *(src2 + O(2, 0))) + (*(src1 + O(3, 3)) * *(src2 + O(3, 0)));
	*(dest + O(3, 1)) = (*(src1 + O(3, 0)) * *(src2 + O(0, 1))) + (*(src1 + O(3, 1)) * *(src2 + O(1, 1))) + (*(src1 + O(3, 2)) * *(src2 + O(2, 1))) + (*(src1 + O(3, 3)) * *(src2 + O(3, 1)));
	*(dest + O(3, 2)) = (*(src1 + O(3, 0)) * *(src2 + O(0, 2))) + (*(src1 + O(3, 1)) * *(src2 + O(1, 2))) + (*(src1 + O(3, 2)) * *(src2 + O(2, 2))) + (*(src1 + O(3, 3)) * *(src2 + O(3, 2)));
	*(dest + O(3, 3)) = (*(src1 + O(3, 0)) * *(src2 + O(0, 3))) + (*(src1 + O(3, 1)) * *(src2 + O(1, 3))) + (*(src1 + O(3, 2)) * *(src2 + O(2, 3))) + (*(src1 + O(3, 3)) * *(src2 + O(3, 3)));
}

int BLZ_PresentStatic(
	struct BLZ_StaticBatch *batch,
	const GLfloat *transformMatrix4x4)
{
	const GLfloat *transform = transformMatrix4x4 != NULL ? transformMatrix4x4 : (GLfloat *)&identityMatrix;

	GLfloat mvpMatrix[16];
	if (!batch->is_uploaded)
	{
		upload_static_vertices(batch);
	}
	if (SHADER_CURRENT->mvp_param > -1)
	{
		mult_4x4_matrix(transform, (GLfloat *)&orthoMatrix, (GLfloat *)&mvpMatrix);
		set_mvp_matrix((const GLfloat *)&mvpMatrix);
	}
	bind_tex0(batch->texture->id);
	glBindVertexArray(batch->buffer.vao);
	glDrawElements(GL_TRIANGLES, batch->sprite_count * 6, GL_UNSIGNED_SHORT, (void *)0);
	success();
}

/* Immediate drawing */
int BLZ_DrawImmediate(
	const struct BLZ_Texture *texture,
	const struct BLZ_Vector2 position,
	const struct BLZ_Rectangle *srcRectangle,
	float rotation,
	const struct BLZ_Vector2 *origin,
	const struct BLZ_Vector2 *scale,
	const struct BLZ_Vector4 color,
	enum BLZ_SpriteFlip effects)
{
	struct BLZ_SpriteQuad quad = transform(
		texture,
		position,
		srcRectangle,
		rotation,
		origin,
		scale,
		color,
		effects);
	return BLZ_LowerDrawImmediate(texture->id, &quad);
}

const int SIZE_OF_ONE_QUAD = sizeof(struct BLZ_SpriteQuad);
int BLZ_LowerDrawImmediate(
	GLuint texture,
	const struct BLZ_SpriteQuad *quad)
{
	glBindBuffer(GL_ARRAY_BUFFER, immediateBuf.vbo);
	glBufferData(GL_ARRAY_BUFFER, SIZE_OF_ONE_QUAD, quad, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(immediateBuf.vao);
	set_mvp_matrix((const GLfloat *)&orthoMatrix);
	bind_tex0(texture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)0);
	success();
}

/* Textures */
static void fill_texture_info(struct BLZ_Texture *texture)
{
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texture->width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texture->height);
	glBindTexture(GL_TEXTURE_2D, 0);
}

struct BLZ_Texture *BLZ_LoadTextureFromFile(
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
	const char *last_result = SOIL_last_result();
	if (!id)
	{
		printf("Error: %s\n", last_result);
		return NULL;
	}
	texture = malloc(sizeof(struct BLZ_Texture));
	texture->id = id;
	fill_texture_info(texture);
	return texture;
}

struct BLZ_Texture *BLZ_LoadTextureFromMemory(
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
	texture = malloc(sizeof(struct BLZ_Texture));
	texture->id = SOIL_create_OGL_texture(
		data, width, height, channels, texture_id, flags);
	fill_texture_info(texture);
	return texture;
}

int BLZ_FreeTexture(struct BLZ_Texture *texture)
{
	if (texture == NULL)
	{
		success();
	}
	glDeleteTextures(1, &texture->id);
	free(texture);
	success();
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

/* Render targets */
static const GLenum DRAW_BUFFERS[1] = {GL_COLOR_ATTACHMENT0};
struct BLZ_RenderTarget *BLZ_CreateRenderTarget(int width, int height)
{
	GLuint framebuffer, texture;
	struct BLZ_RenderTarget *result;
	glGenFramebuffers(1, &framebuffer);
	glGenTextures(1, &texture);
	null_if_false(framebuffer, "Could not create framebuffer");
	null_if_false(texture, "Could not create texture for framebuffer");
	result = malloc(sizeof(struct BLZ_RenderTarget));
	result->id = framebuffer;
	result->texture.id = texture;
	result->texture.width = width;
	result->texture.height = height;
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, NULL);
	BLZ_SetTextureFiltering(&result->texture, NEAREST, NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						   texture, 0);
	glDrawBuffers(1, DRAW_BUFFERS);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		free(result);
		fail("The specified framebuffer is not complete");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return result;
}

int BLZ_BindRenderTarget(struct BLZ_RenderTarget *target)
{
	glBindFramebuffer(GL_FRAMEBUFFER, target == NULL ? 0 : target->id);
	success();
}

int BLZ_FreeRenderTarget(struct BLZ_RenderTarget *target)
{
	if (target == NULL) {
		success();
	}
	glDeleteTextures(1, &target->texture.id);
	glDeleteFramebuffers(1, &target->id);
	free(target);
	success();
}

/* glUniform shims */
/* LCOV_EXCL_START */
#define PARAM1(type) type v0
#define PARAM2(type) PARAM1(type), type v1
#define PARAM3(type) PARAM2(type), type v2
#define PARAM4(type) PARAM3(type), type v3
#define PASS_PARAM1 v0
#define PASS_PARAM2 PASS_PARAM1, v1
#define PASS_PARAM3 PASS_PARAM2, v2
#define PASS_PARAM4 PASS_PARAM3, v3

#define UNIFORM_VEC(postfix, type, n)                            \
	void BLZ_Uniform##n##postfix(GLint location, PARAM##n(type)) \
	{                                                            \
		glUniform##n##postfix(location, PASS_PARAM##n);          \
	}

#define UNIFORM_MAT(size)                                             \
	void BLZ_UniformMatrix##size##fv(                                 \
		GLint location,                                               \
		GLsizei count,                                                \
		GLboolean transpose,                                          \
		const GLfloat *value)                                         \
	{                                                                 \
		glUniformMatrix##size##fv(location, count, transpose, value); \
	}

UNIFORM_VEC(f, GLfloat, 1)
UNIFORM_VEC(f, GLfloat, 2)
UNIFORM_VEC(f, GLfloat, 3)
UNIFORM_VEC(f, GLfloat, 4)
UNIFORM_VEC(i, GLint, 1)
UNIFORM_VEC(i, GLint, 2)
UNIFORM_VEC(i, GLint, 3)
UNIFORM_VEC(i, GLint, 4)
UNIFORM_VEC(ui, GLuint, 1)
UNIFORM_VEC(ui, GLuint, 2)
UNIFORM_VEC(ui, GLuint, 3)
UNIFORM_VEC(ui, GLuint, 4)
UNIFORM_MAT(2)
UNIFORM_MAT(3)
UNIFORM_MAT(4)
UNIFORM_MAT(2x3)
UNIFORM_MAT(3x2)
UNIFORM_MAT(2x4)
UNIFORM_MAT(4x2)
UNIFORM_MAT(3x4)
UNIFORM_MAT(4x3)
/* LCOV_EXCL_STOP */
