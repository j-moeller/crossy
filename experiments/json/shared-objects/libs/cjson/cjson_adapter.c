#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "cJSON/cJSON.h"

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    cJSON* json = cJSON_ParseWithLength(buf, size);
    if (json == NULL) {
        return PARSER_ERROR;
    }

    char* json_buf = cJSON_PrintUnformatted(json);
    int json_buf_len = strlen(json_buf);

    char* ret_buf = malloc(sizeof(char) * (json_buf_len + 1));
    if (ret_buf == NULL) {
        free(json_buf);
        cJSON_Delete(json);
        return TOOLCHAIN_ERROR;
    }
    ret_buf[json_buf_len] = 0;
    memcpy(ret_buf, json_buf, json_buf_len);

    *out_buf = ret_buf;
    *out_size = json_buf_len + 1;

    free(json_buf);
    cJSON_Delete(json);

    return PARSER_OKAY;
}