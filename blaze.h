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

struct BLZ_StaticBatch;
typedef struct BLZ_StaticBatch BLZ_StaticBatch;
struct BLZ_Texture;
typedef struct BLZ_Texture BLZ_Texture;

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

	/* Dynamic drawing */
	extern APIENTRY int APICALL BLZ_Draw(
		GLuint texture,
		struct BLZ_Vector2 *position,
		struct BLZ_Rectangle *srcRectangle,
		float rotation,
		struct BLZ_Vector2 *origin,
		struct BLZ_Vector2 *scale,
		enum BLZ_SpriteEffects effects,
		float layerDepth);

	extern APIENTRY int APICALL BLZ_LowerDraw(
		GLuint texture,
		struct BLZ_SpriteQuad *quad);

	extern APIENTRY int APICALL BLZ_Flush();
	extern APIENTRY int APICALL BLZ_Present();

	/* TODO: Static drawing */

	/* Shaders */
	extern APIENTRY GLuint APICALL BLZ_CompileShader(char *vert, char *frag);
	extern APIENTRY int APICALL BLZ_UseShader(GLuint program);
	extern APIENTRY int APICALL BLZ_DeleteShader(GLuint program);
	extern APIENTRY GLuint BLZ_GetDefaultShader();

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

	extern APIENTRY BLZ_Texture* APICALL BLZ_LoadTextureFromFile(
		const char *filename,
		enum BLZ_ImageChannels channels,
		unsigned int texture_id,
		enum BLZ_ImageFlags flags);

	extern APIENTRY BLZ_Texture* APICALL BLZ_LoadTextureFromMemory(
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

	extern APIENTRY int APICALL BLZ_FreeImage(struct BLZ_Texture *texture);

#ifdef __cplusplus
}
#endif

#endif
