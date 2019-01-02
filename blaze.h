#ifndef _BLAZE_H
#define _BLAZE_H

#include "./deps/SOIL/SOIL.h"
#include "./glad/include/glad/glad.h"

#ifndef APIENTRY
#if defined(__WIN32__)
#ifdef __BORLANDC__
#ifdef BUILD_SDL
#define APIENTRY
#else
#define APIENTRY __declspec(dllimport)
#endif
#else
#define APIENTRY __declspec(dllexport)
#endif
#else
#if defined(__GNUC__) && __GNUC__ >= 4
#define APIENTRY __attribute__((visibility("default")))
#elif defined(__GNUC__) && __GNUC__ >= 2
#define APIENTRY __declspec(dllexport)
#else
#define APIENTRY
#endif
#endif
#endif

#ifndef APICALL
#if defined(__WIN32__) && !defined(__GNUC__)
#define APICALL __cdecl
#else
#define APICALL
#endif
#endif

#if !defined(__MACH__)
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((int *)0)
#endif
#endif /* NULL */
#endif

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

struct BLZ_Vertex
{
	GLfloat x, y, z;
	GLfloat padding;
	GLfloat r, g, b, a;
	GLfloat u, v;
};

struct BLZ_SpriteQuad
{
	struct BLZ_Vertex vertices[4];
};

struct BLZ_Texture
{
	GLuint id;
	int width, height;
};

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
	/* Init and shutdown */
	enum BLZ_InitFlags
	{
		DEFAULT = 0,
		NO_TRIPLEBUFFER = 1
	};

	extern APIENTRY int APICALL BLZ_Load(glGetProcAddress loader);
	extern APIENTRY int APICALL BLZ_Init(
		int max_textures,
		int max_sprites_per_tex,
		enum BLZ_InitFlags flags);

	extern APIENTRY int APICALL BLZ_GetOptions(
		int *max_textures,
		int *max_sprites_per_tex,
		enum BLZ_InitFlags *flags);

	extern APIENTRY int APICALL BLZ_SetViewport(int w, int h);
	extern APIENTRY int APICALL BLZ_Shutdown();
	extern APIENTRY char *APICALL BLZ_GetLastError();

	/* Wrapper functions */
	extern APIENTRY void APICALL BLZ_SetClearColor(struct BLZ_Vector4 color);
	extern APIENTRY void APICALL BLZ_Clear(enum BLZ_ClearOptions options);

	/* Dynamic drawing */
	extern APIENTRY int APICALL BLZ_Draw(
		struct BLZ_Texture* texture,
		struct BLZ_Vector2 position,
		struct BLZ_Rectangle *srcRectangle,
		float rotation,
		struct BLZ_Vector2 *origin,
		struct BLZ_Vector2 *scale,
		struct BLZ_Vector4 color,
		enum BLZ_SpriteEffects effects,
		float layerDepth);

	extern APIENTRY int APICALL BLZ_LowerDraw(
		GLuint texture,
		struct BLZ_SpriteQuad *quad);

	extern APIENTRY int APICALL BLZ_Flush();
	extern APIENTRY int APICALL BLZ_Present();

	/* TODO: Static drawing */

	/* Shaders */
	extern APIENTRY BLZ_Shader *APICALL BLZ_CompileShader(char *vert, char *frag);
	extern APIENTRY int APICALL BLZ_UseShader(BLZ_Shader *program);
	extern APIENTRY int APICALL BLZ_DeleteShader(BLZ_Shader *program);
	extern APIENTRY BLZ_Shader *BLZ_GetDefaultShader();

	extern APIENTRY GLint BLZ_GetUniformLocation(
		struct BLZ_Shader *shader,
		const char *name);

	extern APIENTRY void APICALL BLZ_Uniform1f(GLint location,
											   GLfloat v0);

	extern APIENTRY void APICALL BLZ_Uniform2f(GLint location,
											   GLfloat v0,
											   GLfloat v1);

	extern APIENTRY void APICALL BLZ_Uniform3f(GLint location,
											   GLfloat v0,
											   GLfloat v1,
											   GLfloat v2);

	extern APIENTRY void APICALL BLZ_Uniform4f(GLint location,
											   GLfloat v0,
											   GLfloat v1,
											   GLfloat v2,
											   GLfloat v3);

	extern APIENTRY void APICALL BLZ_Uniform1i(GLint location,
											   GLint v0);

	extern APIENTRY void APICALL BLZ_Uniform2i(GLint location,
											   GLint v0,
											   GLint v1);

	extern APIENTRY void APICALL BLZ_Uniform3i(GLint location,
											   GLint v0,
											   GLint v1,
											   GLint v2);

	extern APIENTRY void APICALL BLZ_Uniform4i(GLint location,
											   GLint v0,
											   GLint v1,
											   GLint v2,
											   GLint v3);

	extern APIENTRY void APICALL BLZ_Uniform1ui(GLint location,
												GLuint v0);

	extern APIENTRY void APICALL BLZ_Uniform2ui(GLint location,
												GLuint v0,
												GLuint v1);

	extern APIENTRY void APICALL BLZ_Uniform3ui(GLint location,
												GLuint v0,
												GLuint v1,
												GLuint v2);

	extern APIENTRY void APICALL BLZ_Uniform4ui(GLint location,
												GLuint v0,
												GLuint v1,
												GLuint v2,
												GLuint v3);

	extern APIENTRY void APICALL BLZ_UniformMatrix2fv(GLint location,
													  GLsizei count,
													  GLboolean transpose,
													  const GLfloat *value);

	extern APIENTRY void APICALL BLZ_UniformMatrix3fv(GLint location,
													  GLsizei count,
													  GLboolean transpose,
													  const GLfloat *value);

	extern APIENTRY void APICALL BLZ_UniformMatrix4fv(GLint location,
													  GLsizei count,
													  GLboolean transpose,
													  const GLfloat *value);

	extern APIENTRY void APICALL BLZ_UniformMatrix2x3fv(GLint location,
														GLsizei count,
														GLboolean transpose,
														const GLfloat *value);

	extern APIENTRY void APICALL BLZ_UniformMatrix3x2fv(GLint location,
														GLsizei count,
														GLboolean transpose,
														const GLfloat *value);

	extern APIENTRY void APICALL BLZ_UniformMatrix2x4fv(GLint location,
														GLsizei count,
														GLboolean transpose,
														const GLfloat *value);

	extern APIENTRY void APICALL BLZ_UniformMatrix4x2fv(GLint location,
														GLsizei count,
														GLboolean transpose,
														const GLfloat *value);

	extern APIENTRY void APICALL BLZ_UniformMatrix3x4fv(GLint location,
														GLsizei count,
														GLboolean transpose,
														const GLfloat *value);

	extern APIENTRY void APICALL BLZ_UniformMatrix4x3fv(GLint location,
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

	extern APIENTRY struct BLZ_Texture *APICALL BLZ_LoadTextureFromFile(
		const char *filename,
		enum BLZ_ImageChannels channels,
		unsigned int texture_id,
		enum BLZ_ImageFlags flags);

	extern APIENTRY struct BLZ_Texture *APICALL BLZ_LoadTextureFromMemory(
		const unsigned char *const buffer,
		int buffer_length,
		enum BLZ_ImageChannels force_channels,
		unsigned int texture_id,
		enum BLZ_ImageFlags flags);

	extern APIENTRY int APICALL BLZ_SaveScreenshot(
		const char *filename,
		enum BLZ_SaveImageFormat format,
		int x, int y,
		int width, int height);

	extern APIENTRY int APICALL BLZ_FreeTexture(struct BLZ_Texture *texture);

#ifdef __cplusplus
}
#endif

#endif
