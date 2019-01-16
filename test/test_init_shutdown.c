#include "common.h"

int main(int argc, char *argv[])
{
	int max_tex, max_sprites;
	enum BLZ_InitFlags flags;
	struct BLZ_SpriteBatch* batch = NULL;
	if (Test_Init() != 0) {
		printf("Could not initialize test suite\n");
		return -1;
	}
	plan(13);

	batch =BLZ_CreateBatch(5, 100, DEFAULT);
	ok(batch != NULL, "initialized with 5, 100");
	ok(BLZ_GetOptions(batch, &max_tex, &max_sprites, &flags), "retrieved options");
	ok(max_tex == 5, "max texture count is 5");
	ok(max_sprites == 100, "max sprite count per texture is 100");
	ok(flags == DEFAULT, "default flags");
	batch = BLZ_CreateBatch(10, 50, NO_BUFFERING);
	ok(batch != NULL, "re-initialized");
	ok(BLZ_GetOptions(batch, &max_tex, &max_sprites, &flags), "retrieved options again");
	ok(max_tex == 10, "max texture count is 10");
	ok(max_sprites == 50, "max sprite count per texture is 50");
	ok(flags == NO_BUFFERING, "specified flags");
	ok(BLZ_FreeBatch(batch), "shutdown");
	ok(!BLZ_GetOptions(batch, &max_tex, &max_sprites, &flags), "fails because not initialized");
	ok(BLZ_CreateBatch(0, 0, DEFAULT) == NULL, "should not initialize with wrong params");

	Test_Shutdown();
	done_testing();
}
