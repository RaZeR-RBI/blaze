#include "common.h"

int checkSDLError(int line)
{
	const char *error = SDL_GetError();
	if (*error != '\0')
	{
		printf("SDL Error: %s\n", error);
		if (line != -1)
			printf(" + line: %i\n", line);
		SDL_ClearError();
		return -1;
	}
	return 0;
}

int Test_Init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		die("Unable to initialize SDL");
	}

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	window = SDL_CreateWindow("TEST",
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							  WINDOW_WIDTH, WINDOW_HEIGHT,
							  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!window)
		die("Unable to create window");
	checkSDLError(__LINE__);

	context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, context);

	if (!BLZ_Load((glGetProcAddress)SDL_GL_GetProcAddress))
	{
		die("Unable to load OpenGL library");
	}
	return 0;
}

void Test_Shutdown()
{
	SDL_Quit();
}

GLuint create_buffer(GLenum type, int size, GLenum usage)
{
	GLuint bo;
	glGenBuffers(1, &bo);
	glBindBuffer(type, bo);
	glBufferData(type, (GLsizeiptr)size, NULL, usage);
	return bo;
}

void Feedback_Enable(int count)
{
	GLuint tbo = create_buffer(GL_ARRAY_BUFFER, sizeof(GLfloat) * count, GL_STATIC_READ);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
}

void Feedback_Read(GLfloat* buffer, int count)
{
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(GLfloat) * count, buffer);
}

void Feedback_Begin()
{
	glBeginTransformFeedback(GL_TRIANGLES);
}

void Feedback_End()
{
	glEndTransformFeedback();
	glFlush();
}
