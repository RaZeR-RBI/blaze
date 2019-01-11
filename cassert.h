/* Compile-time assertion check */
#define CASSERT(predicate, file) _impl_CASSERT_LINE(predicate, __LINE__, file)

#define _impl_PASTE(a, b) a##b
#define _impl_CASSERT_LINE(predicate, line, file) \
	typedef char _impl_PASTE(assertion_failed_##file##_, line)[2 * !!(predicate)-1];
