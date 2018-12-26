#include "common.h"

int main()
{
	int max_tex, max_sprites;
	if (Test_Init() != 0) {
		printf("Could not initialize test suite\n");
		return -1;
	}
	plan(11);

	ok(BLZ_Init(5, 100), "initialized with 5, 100");
	ok(BLZ_GetOptions(&max_tex, &max_sprites), "retrieved options");
	ok(max_tex == 5, "max texture count is 5");
	ok(max_sprites == 100, "max sprite count per texture is 100");
	ok(BLZ_Init(10, 50), "re-initialized with 10, 50");
	ok(BLZ_GetOptions(&max_tex, &max_sprites), "retrieved options again");
	ok(max_tex == 10, "max texture count is 10");
	ok(max_sprites == 50, "max sprite count per texture is 50");
	ok(BLZ_Shutdown(), "shutdown");
	ok(!BLZ_GetOptions(&max_tex, &max_sprites), "fails because not initialized");
	ok(!BLZ_Init(0, 0), "should not initialize with wrong params");

	Test_Shutdown();
	done_testing();
}
