#ifndef _BLAZE_H
#define _BLAZE_H

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
typedef enum
{
    None = 0,
    FlipH = 1,
    FlipV = 2,
    Both = FlipH | FlipV
} BLZ_SpriteEffects;

struct BLZ_Vector2
{
    float x, y;
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

struct _BLZ_StaticBatch BLZ_StaticBatch;

#ifdef __cplusplus
extern "C"
{
#endif

/* ---------------------------------- API ----------------------------------- */
// Init and shutdown
extern APIENTRY int APICALL BLZ_Init(int max_textures, int max_sprites_per_tex);
extern APIENTRY int APICALL BLZ_Shutdown();

extern APIENTRY char* APICALL BLZ_GetLastError();

// Dynamic drawing
extern APIENTRY int APICALL BLZ_Begin();
extern APIENTRY int APICALL BLZ_Draw(
    GLuint texture,
    struct BLZ_Vector2 position,
    struct BLZ_Rectangle* srcRectangle,
    float rotation,
    struct BLZ_Vector2 origin,
    struct BLZ_Vector2 scale,
    BLZ_SpriteEffects effects,
    float layerDepth);

extern APIENTRY int APICALL BLZ_Flush();
extern APIENTRY int APICALL BLZ_End();


// TODO: Static drawing

#ifdef __cplusplus
}
#endif

#endif
