#include "common.h"

int main()
{
	if (Test_Init() != 0) {
		printf("Could not initialize test suite\n");
		return -1;
	}
	plan(2);
	ok(BLZ_Init(5, 100));
	ok(BLZ_Shutdown());
	Test_Shutdown();
	return 0;
}
