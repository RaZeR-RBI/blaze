make clean && make
export LD_LIBRARY_PATH=.
./test_init_shutdown.out
./test_png_loading.out
./test_draw_dynamic.out
./test_blend_modes.out
./test_draw_static.out
./test_custom_shader.out
./test_multitexturing.out
./test_render_target.out
gcov blaze.c
geninfo .
rm -rf docs/coverage/*
mkdir docs
mkdir docs/coverage
genhtml blaze.gcda.info -o docs/coverage
