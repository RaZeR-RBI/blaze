#ifndef _BLAZE_H
#define _BLAZE_H

#include <stdint.h>

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
#define NULL ((void *)0)
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
} blzSpriteEffects;

typedef struct
{
    float x, y;
} blzVector2;

typedef struct
{
    float x, y, w, h;
} blzRectangleF;

typedef struct
{
    uint32_t x, y, w, h;
} blzRectangle;

#ifdef __cplusplus
extern "C"
{
#endif

/* ---------------------------------- API ----------------------------------- */
extern APIENTRY void APICALL blzInit(int32_t max_textures, int32_t max_sprites_per_tex);
extern APIENTRY void APICALL blzShutdown();

extern APIENTRY void APICALL blzBegin();
extern APIENTRY void APICALL blzFlush();
extern APIENTRY void APICALL blzEnd();

// TODO: Draw and batch upload

#ifdef __cplusplus
}
#endif

#endif