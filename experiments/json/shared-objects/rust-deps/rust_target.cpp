#include <cstdio>
#include <cstdlib>
#include <string.h>
#include "common.h"

extern "C" {
int run(const char*, size_t, char**, size_t*);
int rust_run(const char*, size_t, char**, size_t*);
void rust_free(char*, size_t);
}

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    char* rust_buf = nullptr;
    size_t rust_buf_size = 0;
    int res = rust_run(buf, size, &rust_buf, &rust_buf_size);

    if (rust_buf != nullptr && rust_buf_size > 0) {
        // copy the returned buffer to a new one, allocated in C
        char *new_buf = static_cast<char*>(malloc(sizeof(char) * rust_buf_size));
        if (new_buf == nullptr) {
            rust_free(rust_buf, rust_buf_size);
            return TOOLCHAIN_ERROR;
        }

        memcpy(new_buf, rust_buf, rust_buf_size);
        rust_free(rust_buf, rust_buf_size);

        *out_buf = new_buf;
        *out_size = rust_buf_size;
    } else {
        *out_buf = nullptr;
        *out_size = 0;
    }

    return res;
}