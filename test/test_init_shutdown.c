#include "common.h"

int main()
{
	int max_tex, max_sprites;
	enum BLZ_InitFlags flags;
	if (Test_Init() != 0) {
		printf("Could not initialize test suite\n");
		return -1;
	}
	plan(13);

	ok(BLZ_Init(5, 100, DEFAULT), "initialized with 5, 100");
	ok(BLZ_GetOptions(&max_tex, &max_sprites, &flags), "retrieved options");
	ok(max_tex == 5, "max texture count is 5");
	ok(max_sprites == 100, "max sprite count per texture is 100");
	ok(flags == DEFAULT, "default flags");
	ok(BLZ_Init(10, 50, NO_TRIPLEBUFFER), "re-initialized");
	ok(BLZ_GetOptions(&max_tex, &max_sprites, &flags), "retrieved options again");
	ok(max_tex == 10, "max texture count is 10");
	ok(max_sprites == 50, "max sprite count per texture is 50");
	ok(flags == NO_TRIPLEBUFFER, "specified flags");
	ok(BLZ_Shutdown(), "shutdown");
	ok(!BLZ_GetOptions(&max_tex, &max_sprites, &flags), "fails because not initialized");
	ok(!BLZ_Init(0, 0, DEFAULT), "should not initialize with wrong params");

	Test_Shutdown();
	done_testing();
}
