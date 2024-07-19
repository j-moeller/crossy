#include <cstdlib>

#include "common.h"

extern "C" int run(const char* buf, size_t size, char** out_buf,
                   size_t* out_size);

extern "C" LIB_EXPORT int diff_target(const char* data, size_t size,
                                      char** out_data, size_t* out_size)
{
/**
 * We want to catch every exception thrown in the library. However, when
 * compiling v8, we don't need (= can not use) exception handling because it is
 * compiled with -fno-exceptions.
 */
#ifdef HANDLE_EXCEPTIONS
    int ret;
    try {
        ret = run(data, size, out_data, out_size);
    } catch (...) {
        return PARSER_ERROR;
    }

    return ret;
#else
    return run(data, size, out_data, out_size);
#endif
}

extern "C" LIB_EXPORT int fuzz_target(const char* data, size_t size)
{
    char* out_data = NULL;
    size_t out_size;

    int ret = diff_target(data, size, &out_data, &out_size);

    if (out_data != NULL) {
        free(out_data);
    }

    return ret;
}