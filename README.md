# blaze - simple and fast 2D sprite C graphics library
[![Coverage data](coverage.svg)](https://razer-rbi.github.io/blaze/coverage/blaze/blaze.c.gcov.html)

This library provides an OpenGL-based 2D sprite renderer which implements the following functionality:
* **Dynamic batched drawing**. Designed for moving sprites.

  The sprites are batched into several configurable buckets which use VAOs
  (vertex array objects) and are drawn **sorted by their texture** to minimize
  texture state changes and increase performance. The VAOs are double-buffered by
  default, so they can be pushed faster without waiting for GPU to synchronize.

>

    batch = BLZ_CreateBatch(...);

    /* Put a sprite into batch (note: nothing is actually drawn yet) */
    BLZ_Draw(batch, texture, position, srcRect, rotation, origin, scale, color, flip);

    /* Flush our batch to screen. The sprites are sorted by the texture */
    BLZ_Present(batch);


* **Static batched drawing**. Designed for static geometry.

  The sprites are put into static VAO and use the same specified texture.
  Useful for static geometry like tilemaps, which do not change over time.
  It's possible to transform the geometry by supplying a transform matrix.
  The sprites are 'baked' into the GPU memory when they are first presented and
  cannot be modified afterwards.

>

    static_geom = BLZ_CreateStatic(texture, ...);

    /* Put a sprite into batch (note: nothing is actually drawn yet) */
    BLZ_DrawStatic(batch, position, srcRect, rotation, origin, scale, color, flip);

    /* Flush our batch to screen. The batch is cannot be modified after that */
    BLZ_PresentStatic(batch, transform_matrix);


* **Immediate drawing**. When you need to draw a small number of sprites.

  The sprite is immediately drawn to the screen using the specified parameters.
  Can be useful for small amount of sprites where batching will not improve
  performance, or post-processing effects (to draw a fullscreen quad, for example).

>

    BLZ_DrawImmediate(texture, position, srcRect, rotation, origin, scale, color, flip);


* **Texture loading, binding and configuration**.
 Loading images is implemented by SOIL (Simple OpenGL Image Library).
 Multitexturing is supported.
* **Render targets**. Draw to textures and use them later, e.g.
 post-processing effects or screen-in-screen rendering.
* **Shaders**. Use custom GLSL shaders and pass parameters to them.

Sprite positioning algorithm is identical to XNA/MonoGame behaviour -
[this SO answer has an explanation](https://gamedev.stackexchange.com/a/127692).

# Language bindings
* [Rust](https://github.com/razer-rbi/blaze-rs)

# Documentation and examples
API documentation [is available here.](https://razer-rbi.github.io/blaze/api/index.html)

For examples, check out the **test** directory. It covers all implemented
functionality and uses SDL for window creation.

# Initializing the library
Call the `BLZ_Load(...)` function and supply an OpenGL procedure loader.
For example, if you're using SDL, that would be `SDL_GL_GetProcAddress`.
Basically, a loader is a function which accepts a string name of OpenGL function
to load and returns a pointer to it. Or, to be more precise:

    typedef void* (*glGetProcAddress)(const char *name);


# Installing
This projects uses the [clib package manager](https://github.com/clibs/clib).
There are two ways to get the library into your project. One is to use the clib
installer:

    clib install razer-rbi/blaze

Be careful - it will not download the Makefile.

The second way is to clone the repo directly, installing the dependencies and
building using the bundled Makefile:

    git clone https://github.com/RaZeR-RBI/blaze
    cd blaze
    clib install
    make


# Running tests and valgrind checks
Make sure you've cloned the repository.
To run tests, you'll need **gcov** and **lcov**.
Run the following script to execute tests and generate a report:

    ./build_and_test.sh

If you have **valgrind** installed, you can run valgrind checks:

    ./valgrind.sh

Note: a valgrind suppression file is included, which should suppress
driver-related bugs. You can run valgrind on test executables
directly - check the script to understand which parameters should be supplied.

# Generating documentation
The documentation is generated by doxygen by using the following command:

    doxygen Doxyfile

Make sure that you have a version that supports Markdown (1.8+).

# TODO
* C# bindings for [Imagini](https://github.com/project-grove/imagini)
