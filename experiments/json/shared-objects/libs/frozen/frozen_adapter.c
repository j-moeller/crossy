#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "frozen/frozen.h"

#define BUFFER_SIZE 8192

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    char* json_buf = (char*)malloc(BUFFER_SIZE);
    if (NULL == json_buf) {
        return TOOLCHAIN_ERROR;
    }

    struct json_out out = JSON_OUT_BUF(json_buf, BUFFER_SIZE);
    if (0 > json_prettify(buf, size, &out)) {
        free(json_buf);
        return PARSER_ERROR;
    }

    *out_buf = json_buf;
    *out_size = out.u.buf.len;

    return PARSER_OKAY;
}