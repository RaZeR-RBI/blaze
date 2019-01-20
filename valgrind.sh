export LD_LIBRARY_PATH=$(pwd)
VALGRIND_OPTS="--leak-check=yes --suppressions=valgrind.supp --num-callers=20"
valgrind $VALGRIND_OPTS ./test_init_shutdown.out
valgrind $VALGRIND_OPTS ./test_draw_dynamic.out
valgrind $VALGRIND_OPTS ./test_blend_modes.out
valgrind $VALGRIND_OPTS ./test_draw_static.out
valgrind $VALGRIND_OPTS ./test_custom_shader.out
