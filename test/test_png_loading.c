
#include "common.h"

unsigned char MINIMAL_PNG[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
  0x0a, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63, 0x00, 0x01, 0x00, 0x00,
  0x05, 0x00, 0x01, 0x0d, 0x0a, 0x2d, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x49,
  0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
unsigned int MINIMAL_PNG_LEN = 70;

int main()
{
	if (Test_Init() != 0) {
		printf("Could not initialize test suite\n");
		return -1;
	}
	plan(3);
	ok(BLZ_LoadTextureFromFile("test/pnggrad8rgb.png", AUTO, 0, 0) != NULL);
	ok(BLZ_LoadTextureFromMemory(MINIMAL_PNG, MINIMAL_PNG_LEN, AUTO, 0, 0) != NULL);
	ok(!BLZ_LoadTextureFromMemory(MINIMAL_PNG, 1, AUTO, 0, 0));
	Test_Shutdown();
	done_testing();
}
