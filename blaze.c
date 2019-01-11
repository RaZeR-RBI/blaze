#include "blaze.h"
#include "./deps/SOIL/SOIL.h"
#include "./glad/include/glad/glad.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define calloc_one(s) calloc(1, s)
#define BUFFER_COUNT 3
#define HAS_FLAG(flag) ((FLAGS & flag) == flag)

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
	GLuint vao, vbo, ebo;
};

struct BLZ_StaticBatch
{
	struct BLZ_Texture texture;
	struct Buffer buffer;
};

struct BLZ_Shader
{
	GLuint program;
	GLint mvp_param;
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
	/*"	gl_Position = in_Position;"*/
	"}";

static GLchar fragmentSource[] =
	"#version 130\n"
	"in vec4 ex_Color;"
	"in vec2 ex_Texcoord;"
	"out vec4 outColor;"
	"uniform sampler2D tex;"
	"void main() {"
	"  outColor = texture(tex, ex_Texcoord) * ex_Color;"
	/* "  outColor = vec4(0.2, 0.3, 0.4, 1.0);" */
	"}";
static const GLchar *varyings[] = {"gl_Position"};

static BLZ_Shader *SHADER_DEFAULT;
static BLZ_Shader *SHADER_CURRENT;
static unsigned char FRAMESKIP = 0;

static const int VERT_SIZE = sizeof(struct BLZ_Vertex);

/* TODO: Reuse same VAO for all batches to minimize state changes */
static struct Buffer create_buffer()
{
	int INDICES_SIZE = MAX_SPRITES_PER_TEXTURE * 6 * sizeof(GLushort);
	int i;
	struct Buffer result;
	GLushort *indices = malloc(INDICES_SIZE);
	GLuint vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, VERT_SIZE * 4 * MAX_SPRITES_PER_TEXTURE, NULL, GL_STREAM_DRAW);
	/* x|y| */
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
	for (i = 0; i < MAX_SPRITES_PER_TEXTURE; i++)
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
	if (SHADER_CURRENT->mvp_param != -1)
	{
		BLZ_UniformMatrix4fv(SHADER_CURRENT->mvp_param, 1, GL_FALSE,
							 (const GLfloat *)&orthoMatrix);
	}
	success();
}

void BLZ_SetClearColor(struct BLZ_Vector4 color)
{
	glClearColor(color.x, color.y, color.z, color.w);
}

void BLZ_Clear(enum BLZ_ClearOptions options)
{
	glClear(options);
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

GLint BLZ_GetUniformLocation(struct BLZ_Shader *shader, const char *name)
{
	if (shader == NULL || name == NULL)
	{
		return -1;
	}
	return glGetUniformLocation(shader->program, (const GLchar *)name);
}

BLZ_Shader *BLZ_CompileShader(char *vert, char *frag)
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
	if (HAS_FLAG(ENABLE_FEEDBACK))
	{
		glTransformFeedbackVaryings(program, 1, varyings, GL_INTERLEAVED_ATTRIBS);
	}
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

int BLZ_UseShader(BLZ_Shader *program)
{
	GLenum result;
	validate(program != NULL);
	glUseProgram(program->program);
	result = glGetError();
	if (result == GL_NO_ERROR)
	{
		if (program->mvp_param != -1)
		{
			BLZ_UniformMatrix4fv(program->mvp_param, 1, GL_FALSE,
								 (const GLfloat *)&orthoMatrix);
		}
		SHADER_CURRENT = program;
		success();
	}
	printf("glUseProgram: error %d\n", result);
	fail("Could not use shader program");
}

int BLZ_DeleteShader(BLZ_Shader *program)
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

int BLZ_Load(glGetProcAddress loader)
{
	int result = gladLoadGLLoader((GLADloadproc)loader);
	validate(loader != NULL);
	fail_if_false(result, "Could not load the OpenGL library");
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
	if (SHADER_DEFAULT != NULL)
	{
		free(SHADER_DEFAULT);
	}
	MAX_TEXTURES = MAX_SPRITES_PER_TEXTURE = 0;
	success();
}

int BLZ_Init(int max_textures, int max_sprites_per_tex, enum BLZ_InitFlags flags)
{
	int i;
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
		vertices = calloc(MAX_SPRITES_PER_TEXTURE * 4, sizeof(struct BLZ_Vertex));
		check_alloc(vertices);
		cur->vertices = vertices;
		cur->buffer[0] = create_buffer();
		cur->buffer[1] = create_buffer();
		cur->buffer[2] = create_buffer();
	}
	SHADER_DEFAULT = BLZ_CompileShader(vertexSource, fragmentSource);
	fail_if_false(SHADER_DEFAULT, "Could not compile default shader");
	fail_if_false(BLZ_UseShader(SHADER_DEFAULT), "Could not use default shader");
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
	success();
}

int BLZ_Flush()
{
	unsigned char to_draw, to_fill;
	struct StreamBatch batch;
	struct StreamBatch *batch_ptr;
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
		batch_ptr = (stream_batches + i);
		batch = *batch_ptr;
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
		/* bind our texture and the VAO and draw it */
		glBindTexture(GL_TEXTURE_2D, batch.texture);
		glBindVertexArray(batch.buffer[to_draw].vao);
		glDrawElements(GL_TRIANGLES, batch.quad_count * 6, GL_UNSIGNED_SHORT, (void *)0);
		batch_ptr->quad_count = 0;
		batch_ptr->texture = 0;
	}
	success();
}

int BLZ_Present()
{
	fail_if_false(BLZ_Flush(), "Could not flush the sprite queue");
	if (!HAS_FLAG(NO_TRIPLEBUFFER) && FRAMESKIP == 0)
	{
		BUFFER_INDEX++;
		if (BUFFER_INDEX >= BUFFER_COUNT)
		{
			BUFFER_INDEX -= BUFFER_COUNT;
		}
	}
	if (FRAMESKIP > 0)
	{
		FRAMESKIP--;
	}
	success();
}

#define vec_add(to, one, two)     \
	do                            \
	{                             \
		to.x = (one).x + (two).x; \
		to.y = (one).y + (two).y; \
	} while (0);
#define vec_sub(to, one, two)     \
	do                            \
	{                             \
		to.x = (one).x - (two).x; \
		to.y = (one).y - (two).y; \
	} while (0);
#define vec_rotate(to, length, angle)            \
	do                                           \
	{                                            \
		if (angle != 0.0f)                       \
		{                                        \
			to.x = length * (float)cos(-angle);  \
			to.y = length * -(float)sin(-angle); \
		}                                        \
		else                                     \
		{                                        \
			to.x = length;                       \
			to.y = 0;                            \
		}                                        \
	} while (0);
#define vec_set(to, xval, yval) \
	do                          \
	{                           \
		to.x = xval;            \
		to.y = yval;            \
	} while (0);

#define PI 3.14159265f
#define HALF_PI PI / 2

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

int BLZ_Draw(
	struct BLZ_Texture *texture,
	struct BLZ_Vector2 position,
	struct BLZ_Rectangle *srcRectangle,
	float rotation,
	struct BLZ_Vector2 *origin,
	struct BLZ_Vector2 *scale,
	struct BLZ_Vector4 color,
	enum BLZ_SpriteEffects effects)
{
	/* position: top-left, top-right, bottom-left, bottom-right */
	struct BLZ_Vector2 p_tl, p_tr, p_bl, p_br;
	/* same for texture coordinates */
	struct BLZ_Vector2 t_tl, t_tr, t_bl, t_br;
	struct BLZ_Vector2 tmp;
	struct BLZ_SpriteQuad quad;
	int width = srcRectangle == NULL ? texture->width : srcRectangle->w;
	int height = srcRectangle == NULL ? texture->height : srcRectangle->h;
	if (scale != NULL)
	{
		width *= scale->x;
		height *= scale->y;
	}

	/* calculate corner positions in clockwise order starting from top left */
	if (origin == NULL)
	{
		p_tl = position;
	}
	else
	{
		vec_sub(p_tl, position, *origin);
	}
	vec_rotate(tmp, width, rotation);
	vec_add(p_tr, p_tl, tmp);
	vec_rotate(tmp, height, rotation - HALF_PI);
	vec_add(p_br, p_tr, tmp);
	vec_rotate(tmp, -width, rotation);
	vec_add(p_bl, p_br, tmp);

	/* calculate texture coordinates */
	/* TODO: Implement SpriteEffects flipping */
	if (srcRectangle == NULL)
	{
		vec_set(t_tl, 0, 0);
		vec_set(t_tr, 1, 0);
		vec_set(t_br, 1, 1);
		vec_set(t_bl, 0, 1);
	}
	else
	{
		vec_set(t_tl,
				srcRectangle->x / texture->width,
				srcRectangle->y / texture->height);
		t_tr.x = t_tl.x + srcRectangle->w / texture->width;
		t_tr.y = t_tl.y;
		t_bl.x = t_tl.x;
		t_bl.y = t_tl.y + srcRectangle->h / texture->height;
		t_br.x = t_tr.x;
		t_br.y = t_bl.y;
	}

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
	return BLZ_LowerDraw(texture->id, &quad);
}

int BLZ_LowerDraw(GLuint texture, struct BLZ_SpriteQuad *quad)
{
	struct StreamBatch *batch = NULL;
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
	batch->texture = texture;
	success();
}

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
	if (!id)
	{
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
