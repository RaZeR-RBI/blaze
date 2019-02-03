/*
MIT license

Copyright 2019 (c) Adel Vilkov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef _BLAZE_H
#define _BLAZE_H

#include "stddef.h"
#include "./cassert.h"
#include "./deps/SOIL/SOIL.h"
#include "./glad/include/glad/glad.h"

/* TODO: Check on Mac and compilers other than GCC */
/* \cond */
#define BLZAPICALL
#define BLZAPIENTRY

/* ------------------------------- Data types ------------------------------- */
#define BLZ_TRUE 1
#define BLZ_FALSE 0
/* \endcond */

/**
 * Defines if the sprite should be flipped in any direction.
 */
enum BLZ_SpriteFlip
{
	NONE = 0,
	FLIP_H = 1,
	FLIP_V = 2,
	BOTH = FLIP_H | FLIP_V
};

/**
 * A vector which contains 2 floats.
 */
struct BLZ_Vector2
{
	float x, y;
};

/**
 * A vector which contains 4 floats.
 */
struct BLZ_Vector4
{
	float x, y, z, w;
};

/**
 * A rectangle which has it's top-left corner position, width and height
 * expressed in floats.
 */
struct BLZ_Rectangle
{
	float x, y, w, h;
};

#pragma pack(push, 1)
/**
 * Underlying vertex array structure.
 */
struct BLZ_Vertex
{
	GLfloat x, y;
	GLfloat u, v;
	GLfloat r, g, b, a;
};
#pragma pack(pop)

/**
 * Underlying sprite quad data structure. You can use it to predefine fixed
 * sprite quads (fullscreen quad, for example).
 * @see BLZ_LowerDraw
 * @see BLZ_LowerDrawStatic
 * @see BLZ_LowerDrawImmediate
 */
struct BLZ_SpriteQuad
{
	struct BLZ_Vertex vertices[4];
};

/**
 * Defines a texture.
 */
struct BLZ_Texture
{
	GLuint id;  /** OpenGL texture id (name) */
	int width;  /** Texture width in pixels */
	int height; /** Texture height in pixels */
};

/**
 * Defines a blend factor in blending equation.
 * @see BLZ_BlendFunc
 */
enum BLZ_BlendFactor
{
	ZERO = GL_ZERO,
	ONE = GL_ONE,
	SRC_COLOR = GL_SRC_COLOR,
	ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
	DST_COLOR = GL_DST_COLOR,
	ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
	SRC_ALPHA = GL_SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
	DST_ALPHA = GL_DST_ALPHA,
	ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA
};

/**
 * Defines a blending function.
 * @see BLEND_NORMAL
 * @see BLEND_ADDITIVE
 * @see BLEND_MULTIPLY
 */
struct BLZ_BlendFunc
{
	enum BLZ_BlendFactor source;	  /** Source blend factor */
	enum BLZ_BlendFactor destination; /** Destination blend factor */
};

/**
 * Normal alpha blending (src = SRC_ALPHA, dst = ONE_MINUS_SRC_ALPHA)
 */
extern const struct BLZ_BlendFunc BLEND_NORMAL;
/**
 * Additive blending (src = ONE, dst = ONE)
 */
extern const struct BLZ_BlendFunc BLEND_ADDITIVE;
/**
 * Multiplicative blending (src = DST_COLOR, dst = ZERO)
 */
extern const struct BLZ_BlendFunc BLEND_MULTIPLY;

struct BLZ_SpriteBatch;
/**
 * Defines a dynamic sprite batch which is drawn when BLZ_Present is called.
 * @see BLZ_Draw
 * @see BLZ_LowerDraw
 * @see BLZ_Present
 */
typedef struct BLZ_SpriteBatch BLZ_SpriteBatch;

struct BLZ_StaticBatch;
/**
 * Defines a pre-baked sprite batch which is useful for drawing static geometry
 * like tiles.
 */
typedef struct BLZ_StaticBatch BLZ_StaticBatch;
struct BLZ_Shader;
/**
 * Represents a GLSL shader handle.
 */
typedef struct BLZ_Shader BLZ_Shader;

/**
 * OpenGL function loader signature.
 * @see BLZ_Load
 */
typedef void (*glGetProcAddress)(const char *name);

#ifdef __cplusplus
extern "C"
{
#endif

	/* -------------------------------- API --------------------------------- */
	/** \addtogroup global Global
	 * Global state functions.
	 * @{
	 */

	/**
	* Loads the OpenGL functions using the specified loader and initializes the library.
	* @param loader OpenGL function loader which accepts an 'const char *name'.
	* If you're using SDL, pass SDL_GL_GetProcAddress as the value.
	* @return Non-zero on success, zero on failure
	*/
	extern BLZAPIENTRY int BLZAPICALL BLZ_Load(glGetProcAddress loader);
	/**
	 * Sets the viewport size in pixels.
	 * Used in sprite position calculations.
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_SetViewport(int w, int h);
	/**
	 * Returns a pointer to error string, if any.
	 * @return String if there is an error, NULL if there is no error present
	 */
	extern BLZAPIENTRY char *BLZAPICALL BLZ_GetLastError();
	/**
	 * Sets the background clear color.
	 * @see BLZ_Clear
	 */
	extern BLZAPIENTRY void BLZAPICALL BLZ_SetClearColor(struct BLZ_Vector4 color);
	/**
	 * Clears the screen with the specified color.
	 * @see BLZ_SetClearColor
	 */
	extern BLZAPIENTRY void BLZAPICALL BLZ_Clear();
	/**
	 * Sets the currently used blend function.
	 * Note - it will be applied when a sprite will be actually drawn using
	 * \ref BLZ_Present, \ref BLZ_PresentStatic or \ref BLZ_DrawImmediate functions.
	 * @see BLZ_BlendFunc
	 * @see BLZ_Present
	 * @see BLZ_PresentStatic
	 * @see BLZ_DrawImmediate
	 */
	extern BLZAPIENTRY void BLZAPICALL BLZ_SetBlendMode(const struct BLZ_BlendFunc func);
	/** @} */

	/** \addtogroup dynamic Dynamic drawing
	 * Dynamic batched sprite drawing.
	 * Use it when you want to efficiently draw many sprites which share
	 * textures (or texture atlases).
	 * If you want to draw a small number of different sprites which will not
	 * benefit of batching (grouping) them, see \ref immediate module.
	 * If you want to efficiently draw non-changing geometry like tilemaps,
	 * see the \ref static module.
	 * @{
	 */
	/**
	 * Defines initialization flags for dynamic batches.
	 * @see BLZ_CreateBatch
	 * @see BLZ_Draw
	 */
	enum BLZ_InitFlags
	{
		/** Default flags */
		DEFAULT = 0,
		/**
		* Disables sprite vertex array buffering, which lowers GPU memory usage, but
  		* sacrifices sprite drawing speed.
  		*/
		NO_BUFFERING = 1
	};

	/**
	 * Creates a new dynamic batch using the specified parameters.
	 * @param max_buckets Defines maximum sprite buckets. A bucket uses same
	 * texture for all sprites and is limited by max_sprites_per_batch.
	 * @param max_sprites_per_bucket Defines maximum sprite count in one bucket.
	 * @param flags Initialization flags.
	 * @return Pointer to a newly created dynamic batch object.
	 * @see BLZ_Draw
	 * @see BLZ_GetOptions
	 * @see BLZ_FreeBatch
	 */
	extern BLZAPIENTRY struct BLZ_SpriteBatch *BLZAPICALL BLZ_CreateBatch(
		int max_buckets,
		int max_sprites_per_bucket,
		enum BLZ_InitFlags flags);

	/**
	 * Reads options specified in \ref BLZ_CreateBatch for the specified batch object.
	 * @see BLZ_CreateBatch
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_GetOptions(
		struct BLZ_SpriteBatch *batch,
		int *max_buckets,
		int *max_sprites_per_bucket,
		enum BLZ_InitFlags *flags);

	/**
	 * Destroys the specified dynamic batch object.
	 * @see BLZ_CreateBatch
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_FreeBatch(struct BLZ_SpriteBatch *batch);

	/**
	 * Adds a sprite to specified batch using specified parameters.
	 * @param batch The batch to put the sprite in
	 * @param texture Sprite texture
	 * @param position Position of the sprite (top-left corner if origin is NULL)
	 * @param srcRectangle Part of the source texture to draw defined in pixels,
	 * or NULL if the whole texture should be drawn
	 * @param rotation Rotation of the sprite in clockwise direction in radians
	 * @param origin The point around which the sprite should be positioned and
	 * rotated, if NULL, top-left corner (0, 0) will be used
	 * @param scale Scale in X and Y directions, if NULL, defaults to (1,1)
	 * @param color Color to apply to the sprite (color gets multiplied if
	 * default shader is used)
	 * @param effects Defines if the sprite should be flipped in any direction
	 * @see BLZ_Present
	 * @see BLZ_CreateBatch
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_Draw(
		struct BLZ_SpriteBatch *batch,
		struct BLZ_Texture *texture,
		struct BLZ_Vector2 position,
		struct BLZ_Rectangle *srcRectangle,
		float rotation,
		struct BLZ_Vector2 *origin,
		struct BLZ_Vector2 *scale,
		struct BLZ_Vector4 color,
		enum BLZ_SpriteFlip effects);

	/**
	 * Lower level dynamic batching function, called by \ref BLZ_Draw. You can
	 * pass your own quad (fullscreen one, for example).
	 * @see BLZ_Draw
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_LowerDraw(
		struct BLZ_SpriteBatch *batch,
		GLuint texture,
		struct BLZ_SpriteQuad *quad);

	/**
	 * Draws everything from the specified dynamic batch to screen.
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_Present(struct BLZ_SpriteBatch *batch);
	/** @} */

	/** \addtogroup static Static drawing
	 * Static batched sprite drawing.
	 * Use it when you want to efficiently draw static things like tilemaps.
	 * On the first \ref BLZ_PresentStatic call, the sprite data will be 'baked'
	 * into GPU memory and then become non-modifiable.
	 * If you want to draw a small number of different sprites which will not
	 * benefit of batching (grouping) them, see \ref immediate module.
	 * If you want to efficiently draw changing geometry,
	 * see the \ref dynamic module.
	 * @{
	 */
	/**
	 * Creates a 'static' batch object. The difference from the dynamic one is
	 * that when this batch is drawn for the first time, the sprites will be
	 * 'baked' into vertex array and cannot be modified. It's useful for static
	 * level geometry like tilemaps, because individual tile positions shouldn't
	 * be calculated each frame, but they should be transformed as a whole
	 * (by camera, for example). In other words, it lowers CPU usage.
	 * @param texture Texture which will be used to draw the sprites
	 * @param max_sprite_count Maximum count of sprites which can be stored
	 * @see BLZ_DrawStatic
	 * @see BLZ_FreeStatic
	 * @see BLZ_GetOptionsStatic
	 */
	extern BLZAPIENTRY struct BLZ_StaticBatch BLZAPICALL *BLZ_CreateStatic(
		struct BLZ_Texture *texture, int max_sprite_count);

	/**
	 * Reads options specified in \ref BLZ_CreateStatic for the specified static
	 * batch object.
	 * @see BLZ_CreateStatic
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_GetOptionsStatic(
		struct BLZ_StaticBatch *batch,
		int *max_sprite_count);

	/**
	 * Destroys the specified static batch object.
	 * @see BLZ_CreateStatic
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_FreeBatchStatic(
		struct BLZ_StaticBatch *batch);

	/**
	 * Adds a sprite to specified batch using specified parameters.
	 * @param batch The batch to put the sprite in
	 * @param position Position of the sprite (top-left corner if origin is NULL)
	 * @param srcRectangle Part of the source texture to draw defined in pixels,
	 * or NULL if the whole texture should be drawn
	 * @param rotation Rotation of the sprite in clockwise direction in radians
	 * @param origin The point around which the sprite should be positioned and
	 * rotated, if NULL, top-left corner (0, 0) will be used
	 * @param scale Scale in X and Y directions, if NULL, defaults to (1,1)
	 * @param color Color to apply to the sprite (color gets multiplied if
	 * default shader is used)
	 * @param effects Defines if the sprite should be flipped in any direction
	 * @see BLZ_PresentStatic
	 * @see BLZ_CreateStatic
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_DrawStatic(
		struct BLZ_StaticBatch *batch,
		struct BLZ_Vector2 position,
		struct BLZ_Rectangle *srcRectangle,
		float rotation,
		struct BLZ_Vector2 *origin,
		struct BLZ_Vector2 *scale,
		struct BLZ_Vector4 color,
		enum BLZ_SpriteFlip effects);

	/**
	 * Lower level static batching function, called by \ref BLZ_DrawStatic. You can
	 * pass your own quad (fullscreen one, for example).
	 * @see BLZ_DrawStatic
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_LowerDrawStatic(
		struct BLZ_StaticBatch *batch,
		struct BLZ_SpriteQuad *quad);

	/**
	 * Draws everything from the specified static batch to screen. If it's
	 * the first time when the batch is drawn, 'bakes' the sprites into GPU
	 * memory and forbids modification.
	 * @param batch The static batch to draw
	 * @param transformMatrix4x4 The transformation matrix. If NULL, identity
	 * matrix is used (so the sprites will be drawn as specified by
	 * \ref BLZ_DrawStatic calls). Pass a custom matrix to implement camera
	 * functionality.
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_PresentStatic(
		struct BLZ_StaticBatch *batch,
		GLfloat *transformMatrix4x4);
	/** @} */

	/** \addtogroup immediate Immediate drawing
	 * Immediate drawing
	 * @{
	 */
	/**
	 * Immediately draws a sprite using the specified parameters to screen.
	 * @param texture The texture to draw
	 * @param position Position of the sprite (top-left corner if origin is NULL)
	 * @param srcRectangle Part of the source texture to draw defined in pixels,
	 * or NULL if the whole texture should be drawn
	 * @param rotation Rotation of the sprite in clockwise direction in radians
	 * @param origin The point around which the sprite should be positioned and
	 * rotated, if NULL, top-left corner (0, 0) will be used
	 * @param scale Scale in X and Y directions, if NULL, defaults to (1,1)
	 * @param color Color to apply to the sprite (color gets multiplied if
	 * default shader is used)
	 * @param effects Defines if the sprite should be flipped in any direction
	 * @see BLZ_LowerDrawImmediate
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_DrawImmediate(
		struct BLZ_Texture *texture,
		struct BLZ_Vector2 position,
		struct BLZ_Rectangle *srcRectangle,
		float rotation,
		struct BLZ_Vector2 *origin,
		struct BLZ_Vector2 *scale,
		struct BLZ_Vector4 color,
		enum BLZ_SpriteFlip effects);

	/**
	 * Lower level immediate drawing function, called by \ref BLZ_DrawImmediate.
	 * Use it if you have a predefined quad to draw (like a fullscreen one for
	 * post-processing effects, for example).
	 * @see BLZ_DrawImmediate
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_LowerDrawImmediate(
		GLuint texture,
		struct BLZ_SpriteQuad *quad);
	/** @} */

	/** \addtogroup texture Textures
	 * Texture binding and options
	 * @{
	 */
	/**
	 * Defines texture filtering modes.
	 * @see BLZ_SetTextureFiltering
	 */
	enum BLZ_TextureFilter
	{
		NEAREST = GL_NEAREST,
		LINEAR = GL_LINEAR,
	};

	/**
	 * Defines texture wrapping modes.
	 * @see BLZ_SetTextureWrap
	 */
	enum BLZ_TextureWrap
	{
		CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
		REPEAT = GL_REPEAT,
		MIRRORED_REPEAT = GL_MIRRORED_REPEAT
	};

	/**
	 * Returns the maximum count of available texture slots which can be used
	 * simultaneously for \ref BLZ_BindTexture texture binding.
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_GetMaxTextureSlots();
	/**
	 * Binds the specified texture to the specified slot number.
	 * If the first (0, zero) slot is specified, overrides currently bound texture
	 * for \ref BLZ_Present, \ref BLZ_PresentStatic and \ref BLZ_DrawImmediate calls.
	 * Note - the sprite position and size will still be calculated using the texture
	 * passed previously, not the overriding one!
	 * @param texture Texture that should be bound, or NULL to unbind it
	 * @param slot Texture slot (from 0 to \ref BLZ_GetMaxTextureSlots)
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_BindTexture(
		struct BLZ_Texture *texture,
		int slot);

	/**
	 * Sets the texture filtering mode.
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_SetTextureFiltering(
		struct BLZ_Texture *texture,
		enum BLZ_TextureFilter minification,
		enum BLZ_TextureFilter magnification);

	/**
	 * Sets the texture wrapping mode.
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_SetTextureWrap(
		struct BLZ_Texture *texture,
		enum BLZ_TextureWrap x,
		enum BLZ_TextureWrap y);

	/** @} */

	/** \addtogroup rendertarget Render targets
	 * Render targets.
	 * Allows rendering to texture and using it later for some other purpose
	 * like post-processing effects.
	 * @{
	 */

	/**
	 * Render target object.
	 */
	struct BLZ_RenderTarget
	{
		/** OpenGL framebuffer ID */
		GLuint id;
		/** Underlying texture */
		struct BLZ_Texture texture;
	};

	/**
	 * Creates a RGBA render target using specified parameters.
	 */
	extern BLZAPIENTRY struct BLZ_RenderTarget *BLZAPICALL BLZ_CreateRenderTarget(
		int width, int height);

	/**
	 * Binds the specified render target for output.
	 * @param target The render target to use, or NULL if we should draw to screen
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_BindRenderTarget(struct BLZ_RenderTarget *target);
	/**
	 * Destroys the specified render target.
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_FreeRenderTarget(struct BLZ_RenderTarget *target);
	/** @} */

	/** \addtogroup shaders Shaders
	 * Shader compilation and freeing.
	 * @{
	 */
	/**
	 * Compiles a shader program from the specified sources.
	 * @param vert Vertex shader source string
	 * @param frag Fragment shader source string
	 * @see BLZ_UseShader
	 * @see BLZ_FreeShader
	 */
	extern BLZAPIENTRY struct BLZ_Shader *BLZAPICALL BLZ_CompileShader(
		char *vert, char *frag);

	/**
	 * Sets the specified shader as the current.
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_UseShader(BLZ_Shader *program);
	/**
	 * Frees memory used by specified shader program.
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_FreeShader(BLZ_Shader *program);

	/**
	 * Returns a pointer to the default shader program.
	 */
	extern BLZAPIENTRY struct BLZ_Shader BLZAPICALL *BLZ_GetDefaultShader();

	/** @} */

	/** \addtogroup shader_params Shader parameters
	 * Shader parameters (uniforms) querying and setting. Identical to
	 * glGetUniformLocation and glUniformXXX OpenGL calls.
	 * @{
	 */
	/**
	 * Returns the uniform location for the specified shader program.
	 */
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

	/** @} */

	/** \addtogroup image Image loading
     * Allows to load textures from files and memory. Powered by SOIL
	 * (Simple OpenGL Image Library).
	 * @{
     */
	/**
	 * Defines which channels should be used when loading a texture.
	 */
	enum BLZ_ImageChannels
	{
		AUTO = SOIL_LOAD_AUTO,
		GRAYSCALE = SOIL_LOAD_L,
		GRAYSCALE_ALPHA = SOIL_LOAD_LA,
		RGB = SOIL_LOAD_RGB,
		RGBA = SOIL_LOAD_RGBA
	};

	/**
	 * Defines various options for image loading.
	 */
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

	/**
	 * Defines formats in which images can be saved.
	 * @see BLZ_SaveScreenshot
	 */
	enum BLZ_SaveImageFormat
	{
		TGA = SOIL_SAVE_TYPE_TGA,
		BMP = SOIL_SAVE_TYPE_BMP,
		DDS = SOIL_SAVE_TYPE_DDS
	};

	/**
	 * Loads a texture from file.
	 * @param filename Path to the texture
	 * @param channels Channels to load. Use AUTO to determine them automatically.
	 * @param texture_id OpenGL texture ID to use. Pass 0 to create a new texture.
	 * @param flags Additional flags
	 */
	extern BLZAPIENTRY struct BLZ_Texture *BLZAPICALL BLZ_LoadTextureFromFile(
		const char *filename,
		enum BLZ_ImageChannels channels,
		unsigned int texture_id,
		enum BLZ_ImageFlags flags);

	/**
	 * Loads a texture from memory.
	 * @param buffer Memory buffer pointer
	 * @param buffer_length Memory buffer size
	 * @param force_channels Channels to load. Use AUTO to determine them automatically.
	 * @param texture_id OpenGL texture ID to use. Pass 0 to create a new texture.
	 * @param flags Additional flags
	 */
	extern BLZAPIENTRY struct BLZ_Texture *BLZAPICALL BLZ_LoadTextureFromMemory(
		const unsigned char *const buffer,
		int buffer_length,
		enum BLZ_ImageChannels force_channels,
		unsigned int texture_id,
		enum BLZ_ImageFlags flags);

	/**
	 * Saves a screenshot to file.
	 * @param filename A path to file
	 * @param format Format to be used
	 * @param x X coordinate of region (pass 0 to capture everything)
	 * @param y Y coordinage of region (pass 0 to capture everything)
	 * @param width Width of region (pass window width to capture everything)
	 * @param height Height of region (pass window height to capture everything)
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_SaveScreenshot(
		const char *filename,
		enum BLZ_SaveImageFormat format,
		int x, int y,
		int width, int height);

	/**
	 * Frees the specified texture.
	 */
	extern BLZAPIENTRY int BLZAPICALL BLZ_FreeTexture(struct BLZ_Texture *texture);
	/** @} */

#ifdef __cplusplus
}
#endif

/* Compile-time assertions */
/* \cond */
#define BLZ_ASSERT(expr) CASSERT(expr, blaze_h)
BLZ_ASSERT(sizeof(struct BLZ_Vertex) == 32)
BLZ_ASSERT(offsetof(struct BLZ_Vertex, x) == 0)
BLZ_ASSERT(offsetof(struct BLZ_Vertex, u) == 8)
BLZ_ASSERT(offsetof(struct BLZ_Vertex, r) == 16)
#undef BLZ_ASSERT
/* \endcond */

#endif
