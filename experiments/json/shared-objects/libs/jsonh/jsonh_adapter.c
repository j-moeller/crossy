#include "json.h/json.h"

#include "common.h"

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    struct json_value_s* root = json_parse(buf, size);
    char* json_buf = json_write_minified(root, out_size);
    if (json_buf == NULL) {
        if (root != NULL) {
            free(root);
        }
        return PARSER_ERROR;
    }

    int json_buf_len = strlen(json_buf);

    char* ret_buf = malloc(sizeof(char) * (json_buf_len + 1));
    ret_buf[json_buf_len] = 0;
    memcpy(ret_buf, json_buf, json_buf_len);

    *out_buf = ret_buf;
    *out_size = json_buf_len + 1;

    free(json_buf);
    free(root);

    return PARSER_OKAY;
}