#ifndef _BLAZE_H
#define _BLAZE_H

#include "stddef.h"
#include "./cassert.h"
#include "./deps/SOIL/SOIL.h"
#include "./glad/include/glad/glad.h"

/* TODO: Check on Mac and compilers other than GCC */
#define BLZAPICALL
#define BLZAPIENTRY

/* ------------------------------- Data types ------------------------------- */
#define BLZ_TRUE 1
#define BLZ_FALSE 0

enum BLZ_SpriteEffects
{
	NONE = 0,
	FLIP_H = 1,
	FLIP_V = 2,
	BOTH = FLIP_H | FLIP_V
};

enum BLZ_ClearOptions
{
	COLOR_BUFFER = GL_COLOR_BUFFER_BIT,
	DEPTH_BUFFER = GL_DEPTH_BUFFER_BIT,
	STENCIL_BUFFER = GL_STENCIL_BUFFER_BIT,
	ALL = COLOR_BUFFER | DEPTH_BUFFER | STENCIL_BUFFER
};

struct BLZ_Vector2
{
	float x, y;
};

struct BLZ_Vector3
{
	float x, y, z;
};

struct BLZ_Vector4
{
	float x, y, z, w;
};

struct BLZ_RectangleF
{
	float x, y, w, h;
};

struct BLZ_Rectangle
{
	unsigned int x, y, w, h;
};

struct BLZ_Color
{
	unsigned char r, g, b, a;
};

#pragma pack(push, 1)
struct BLZ_Vertex
{
	GLfloat x, y;
	GLfloat u, v;
	GLfloat r, g, b, a;
};
#pragma pack(pop)

struct BLZ_SpriteQuad
{
	struct BLZ_Vertex vertices[4];
};

struct BLZ_Texture
{
	GLuint id;
	int width, height;
};

struct BLZ_BlendFunc
{
	GLenum source, destination;
};

extern const struct BLZ_BlendFunc BLEND_NORMAL;
extern const struct BLZ_BlendFunc BLEND_ADDITIVE;
extern const struct BLZ_BlendFunc BLEND_MULTIPLY;

struct BLZ_SpriteBatch;
typedef struct BLZ_SpriteBatch BLZ_SpriteBatch;

struct BLZ_StaticBatch;
typedef struct BLZ_StaticBatch BLZ_StaticBatch;
struct BLZ_Shader;
typedef struct BLZ_Shader BLZ_Shader;

typedef void (*glGetProcAddress)(const char *name);

#ifdef __cplusplus
extern "C"
{
#endif

	/* -------------------------------- API --------------------------------- */
	/* Global state */
	extern BLZAPIENTRY int BLZAPICALL BLZ_Load(glGetProcAddress loader);
	extern BLZAPIENTRY int BLZAPICALL BLZ_SetViewport(int w, int h);
	extern BLZAPIENTRY char *BLZAPICALL BLZ_GetLastError();

	extern BLZAPIENTRY void BLZAPICALL BLZ_SetClearColor(struct BLZ_Vector4 color);
	extern BLZAPIENTRY void BLZAPICALL BLZ_Clear(enum BLZ_ClearOptions options);
	extern BLZAPIENTRY void BLZAPICALL BLZ_SetBlendMode(const struct BLZ_BlendFunc func);

	/* Dynamic drawing */
	enum BLZ_InitFlags
	{
		DEFAULT = 0,
		NO_BUFFERING = 1
	};

	extern BLZAPIENTRY struct BLZ_SpriteBatch *BLZAPICALL BLZ_CreateBatch(
		int max_batches,
		int max_sprites_per_batch,
		enum BLZ_InitFlags flags);

	extern BLZAPIENTRY int BLZAPICALL BLZ_GetOptions(
		struct BLZ_SpriteBatch *batch,
		int *max_batches,
		int *max_sprites_per_batch,
		enum BLZ_InitFlags *flags);

	extern BLZAPIENTRY int BLZAPICALL BLZ_FreeBatch(struct BLZ_SpriteBatch *batch);

	extern BLZAPIENTRY int BLZAPICALL BLZ_Draw(
		struct BLZ_SpriteBatch *batch,
		struct BLZ_Texture *texture,
		struct BLZ_Vector2 position,
		struct BLZ_Rectangle *srcRectangle,
		float rotation,
		struct BLZ_Vector2 *origin,
		struct BLZ_Vector2 *scale,
		struct BLZ_Vector4 color,
		enum BLZ_SpriteEffects effects);

	extern BLZAPIENTRY int BLZAPICALL BLZ_LowerDraw(
		struct BLZ_SpriteBatch *batch,
		GLuint texture,
		struct BLZ_SpriteQuad *quad);

	extern BLZAPIENTRY int BLZAPICALL BLZ_Present(struct BLZ_SpriteBatch *batch);

	/* Static drawing */
	extern BLZAPIENTRY struct BLZ_StaticBatch BLZAPICALL *BLZ_CreateStatic(
		struct BLZ_Texture *texture, int max_sprite_count);

	extern BLZAPIENTRY int BLZAPICALL BLZ_GetOptionsStatic(
		struct BLZ_StaticBatch *batch,
		int *max_sprite_count);

	extern BLZAPIENTRY int BLZAPICALL BLZ_FreeBatchStatic(
		struct BLZ_StaticBatch *batch);

	extern BLZAPIENTRY int BLZAPICALL BLZ_DrawStatic(
		struct BLZ_StaticBatch *batch,
		struct BLZ_Texture *texture,
		struct BLZ_Vector2 position,
		struct BLZ_Rectangle *srcRectangle,
		float rotation,
		struct BLZ_Vector2 *origin,
		struct BLZ_Vector2 *scale,
		struct BLZ_Vector4 color,
		enum BLZ_SpriteEffects effects);

	extern BLZAPIENTRY int BLZAPICALL BLZ_LowerDrawStatic(
		struct BLZ_StaticBatch *batch,
		struct BLZ_SpriteQuad *quad);

	extern BLZAPIENTRY int BLZAPICALL BLZ_PresentStatic(
		struct BLZ_SpriteBatch *batch);

	/* TODO: Texture binding and filtering settings */
	/* TODO: Rendertargets */

	/* Shaders */
	extern BLZAPIENTRY struct BLZ_Shader *BLZAPICALL BLZ_CompileShader(
		char *vert, char *frag);

	extern BLZAPIENTRY int BLZAPICALL BLZ_UseShader(BLZ_Shader *program);
	extern BLZAPIENTRY int BLZAPICALL BLZ_FreeShader(BLZ_Shader *program);
	extern BLZAPIENTRY struct BLZ_Shader BLZAPICALL *BLZ_GetDefaultShader();

	extern BLZAPIENTRY GLint BLZAPICALL BLZ_GetUniformLocation(
		struct BLZ_Shader *shader,
		const char *name);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform1f(GLint location,
													 GLfloat v0);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform2f(GLint location,
													 GLfloat v0,
													 GLfloat v1);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform3f(GLint location,
													 GLfloat v0,
													 GLfloat v1,
													 GLfloat v2);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform4f(GLint location,
													 GLfloat v0,
													 GLfloat v1,
													 GLfloat v2,
													 GLfloat v3);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform1i(GLint location,
													 GLint v0);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform2i(GLint location,
													 GLint v0,
													 GLint v1);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform3i(GLint location,
													 GLint v0,
													 GLint v1,
													 GLint v2);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform4i(GLint location,
													 GLint v0,
													 GLint v1,
													 GLint v2,
													 GLint v3);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform1ui(GLint location,
													  GLuint v0);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform2ui(GLint location,
													  GLuint v0,
													  GLuint v1);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform3ui(GLint location,
													  GLuint v0,
													  GLuint v1,
													  GLuint v2);

	extern BLZAPIENTRY void BLZAPICALL BLZ_Uniform4ui(GLint location,
													  GLuint v0,
													  GLuint v1,
													  GLuint v2,
													  GLuint v3);

	extern BLZAPIENTRY void BLZAPICALL BLZ_UniformMatrix2fv(GLint location,
															GLsizei count,
															GLboolean transpose,
															const GLfloat *value);

	extern BLZAPIENTRY void BLZAPICALL BLZ_UniformMatrix3fv(GLint location,
															GLsizei count,
															GLboolean transpose,
															const GLfloat *value);

	extern BLZAPIENTRY void BLZAPICALL BLZ_UniformMatrix4fv(GLint location,
															GLsizei count,
															GLboolean transpose,
															const GLfloat *value);

	extern BLZAPIENTRY void BLZAPICALL BLZ_UniformMatrix2x3fv(GLint location,
															  GLsizei count,
															  GLboolean transpose,
															  const GLfloat *value);

	extern BLZAPIENTRY void BLZAPICALL BLZ_UniformMatrix3x2fv(GLint location,
															  GLsizei count,
															  GLboolean transpose,
															  const GLfloat *value);

	extern BLZAPIENTRY void BLZAPICALL BLZ_UniformMatrix2x4fv(GLint location,
															  GLsizei count,
															  GLboolean transpose,
															  const GLfloat *value);

	extern BLZAPIENTRY void BLZAPICALL BLZ_UniformMatrix4x2fv(GLint location,
															  GLsizei count,
															  GLboolean transpose,
															  const GLfloat *value);

	extern BLZAPIENTRY void BLZAPICALL BLZ_UniformMatrix3x4fv(GLint location,
															  GLsizei count,
															  GLboolean transpose,
															  const GLfloat *value);

	extern BLZAPIENTRY void BLZAPICALL BLZ_UniformMatrix4x3fv(GLint location,
															  GLsizei count,
															  GLboolean transpose,
															  const GLfloat *value);

	/* Image loading */
	enum BLZ_ImageChannels
	{
		AUTO = SOIL_LOAD_AUTO,
		GRAYSCALE = SOIL_LOAD_L,
		GRAYSCALE_ALPHA = SOIL_LOAD_LA,
		RGB = SOIL_LOAD_RGB,
		RGBA = SOIL_LOAD_RGBA
	};

	enum BLZ_ImageFlags
	{
		POWER_OF_TWO = 1,
		MIPMAPS = 2,
		TEXTURE_REPEATS = 4,
		MULTIPLY_ALPHA = 8,
		INVERT_Y = 16,
		COMPRESS_TO_DXT = 32,
		DDS_LOAD_DIRECT = 64,
		NTSC_SAFE_RGB = 128,
		CoCg_Y = 256,
		TEXTURE_RECTANGLE = 512
	};

	enum BLZ_SaveImageFormat
	{
		TGA = SOIL_SAVE_TYPE_TGA,
		BMP = SOIL_SAVE_TYPE_BMP,
		DDS = SOIL_SAVE_TYPE_DDS
	};

	extern BLZAPIENTRY struct BLZ_Texture *BLZAPICALL BLZ_LoadTextureFromFile(
		const char *filename,
		enum BLZ_ImageChannels channels,
		unsigned int texture_id,
		enum BLZ_ImageFlags flags);

	extern BLZAPIENTRY struct BLZ_Texture *BLZAPICALL BLZ_LoadTextureFromMemory(
		const unsigned char *const buffer,
		int buffer_length,
		enum BLZ_ImageChannels force_channels,
		unsigned int texture_id,
		enum BLZ_ImageFlags flags);

	extern BLZAPIENTRY int BLZAPICALL BLZ_SaveScreenshot(
		const char *filename,
		enum BLZ_SaveImageFormat format,
		int x, int y,
		int width, int height);

	extern BLZAPIENTRY int BLZAPICALL BLZ_FreeTexture(struct BLZ_Texture *texture);

#ifdef __cplusplus
}
#endif

/* Compile-time assertions */
#define BLZ_ASSERT(expr) CASSERT(expr, blaze_h)
BLZ_ASSERT(sizeof(struct BLZ_Vertex) == 32)
BLZ_ASSERT(offsetof(struct BLZ_Vertex, x) == 0)
BLZ_ASSERT(offsetof(struct BLZ_Vertex, u) == 8)
BLZ_ASSERT(offsetof(struct BLZ_Vertex, r) == 16)
#undef BLZ_ASSERT

#endif
