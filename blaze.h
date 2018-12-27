#ifndef _BLAZE_H
#define _BLAZE_H

#include "./deps/SOIL/SOIL.h"
#include <GL/glcorearb.h>

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

struct BLZ_SpriteQuad
{
	struct BLZ_Vector4 corners[4];
	struct BLZ_Vector4 colors[4];
	struct BLZ_Vector2 texcoord[4];
};

struct BLZ_StaticBatch;
typedef struct BLZ_StaticBatch BLZ_StaticBatch;

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

	extern APIENTRY int APICALL BLZ_LoadTextureFromFile(
		const char *filename,
		enum BLZ_ImageChannels channels,
		unsigned int texture_id,
		enum BLZ_ImageFlags flags);

	extern APIENTRY int APICALL BLZ_LoadTextureFromMemory(
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

#ifdef __cplusplus
}
#endif

#endif
