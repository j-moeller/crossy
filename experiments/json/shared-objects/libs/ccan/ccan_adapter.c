#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "ccan/ccan/json/json.h"

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    // The ccan json lib only accepts nullterminated strings
    char* terminated_buffer = malloc(sizeof(char) * (size + 1));
    if (terminated_buffer == NULL) {
        return TOOLCHAIN_ERROR;
    }
    memcpy(terminated_buffer, buf, size);
    terminated_buffer[size] = 0;

    JsonNode* json = json_decode(terminated_buffer);

    if (json == NULL) {
        free(terminated_buffer);
        return PARSER_ERROR;
    }

    char* json_buf = json_encode(json);
    int json_buf_len = strlen(json_buf);

    char* ret_buf = malloc(sizeof(char) * (json_buf_len + 1));
    if (ret_buf == NULL) {
        free(json_buf);
        free(terminated_buffer);
        json_delete(json);
        return TOOLCHAIN_ERROR;
    }
    ret_buf[json_buf_len] = 0;
    memcpy(ret_buf, json_buf, json_buf_len);

    *out_buf = ret_buf;
    *out_size = json_buf_len + 1;

    free(json_buf);
    free(terminated_buffer);
    json_delete(json);

    return PARSER_OKAY;
}