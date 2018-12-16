#include "common.h"
#include "../deps/tap.c/tap.h"

int main()
{
	Test_Init();
	plan(2);
	ok(BLZ_Init(5, 100));
	ok(BLZ_Shutdown());
	Test_Shutdown();
	return 0;
}
