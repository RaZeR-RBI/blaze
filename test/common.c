#include "common.h"
#include "errno.h"
#include "stdint.h"
#include "stdio.h"

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
	SDL_GL_SetSwapInterval(0);

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

/* Renderer output comparation */
#pragma pack(push, 1)
struct BMP_Header
{
	unsigned char marker_1; /* B */
	unsigned char marker_2; /* M */
	uint32_t size;
	uint16_t reserved_1;
	uint16_t reserved_2;
	uint32_t data_offset;
};
#pragma pack(pop)
#define BMP_HEADER_SIZE sizeof(struct BMP_Header)
#define MIN(x, y) ((x) < (y)) ? (x) : (y)

static unsigned char *read_bmp(const char *name, int *len)
{
	unsigned char *data = NULL;
	unsigned char buffer[1024];
	struct BMP_Header header;
	int bytes_read = 0, total_bytes_read = 0, bytes_needed = 0;
	int data_size = 0;
	FILE *file = fopen(name, "rb");
	if (file == NULL)
	{
		printf("Could not open file '%s': %s\n", name, strerror(errno));
		return NULL;
	}
	bytes_read = fread(&header, 1, BMP_HEADER_SIZE, file);
	if (bytes_read < BMP_HEADER_SIZE)
	{
		printf("Unexpected EOF in '%s' when reading header\n", name);
		return NULL;
	}
	if (header.marker_1 != 'B' || header.marker_2 != 'M')
	{
		printf("Invalid or unsupported BMP header\n");
		return NULL;
	}
	if (fseek(file, header.data_offset - BMP_HEADER_SIZE, SEEK_CUR) != 0)
	{
		printf("Could not seek bitmap data in '%s'\n", name);
		return NULL;
	}
	data_size = header.size - header.data_offset;
	*len = data_size;
	data = malloc(data_size);
	do
	{
		bytes_needed = MIN(sizeof(buffer), data_size - total_bytes_read);
		bytes_read = fread(data + total_bytes_read, 1, bytes_needed, file);
		if (bytes_read < bytes_needed)
		{
			free(data);
			printf("Unexpected EOF in '%s' when reading data\n", file);
			return NULL;
		}
		total_bytes_read += bytes_read;
	} while (total_bytes_read < data_size);
	return data;
}

int Validate_Output(const char *test_name, float likeness)
{
	char ref_path[255], actual_path[255];
	int i, len1, len2, wrong_bytes = 0;
	unsigned char *ref, *actual;
	float actual_likeness;
	sprintf(ref_path, "test/refs/%s.bmp", test_name);
	sprintf(actual_path, "%s.bmp", test_name);

	BLZ_SaveScreenshot(actual_path, BMP, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	ref = read_bmp((const char *)ref_path, &len1);
	actual = read_bmp((const char *)actual_path, &len2);
	if (ref == NULL || actual == NULL)
	{
		free(ref);
		free(actual);
		return 0;
	}
	if (len1 != len2)
	{
		printf("Could not compare %s: file size differs\n", test_name);
		free(ref);
		free(actual);
		return 0;
	}
	for (i = 0; i < len1; i++)
	{
		if (*(ref + i) != *(actual + i))
		{
			wrong_bytes++;
		}
	}
	actual_likeness = 1.0f - ((float)wrong_bytes / (float)len1);
	printf("'%s': pixel similarity to reference is %.2f%%, min: %.2f%%\n",
		   test_name, actual_likeness * 100.0f, likeness * 100.0f);
	free(ref);
	free(actual);
	return actual_likeness >= likeness;
}
