#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "json-c/json_tokener.h"
#include "json-c/json_object.h"

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    struct json_tokener* tok = json_tokener_new();

    // enum json_tokener_error jerr;
    json_object* json;

    json = json_tokener_parse_ex(tok, buf, size);
    if (json == NULL || json_tokener_get_error(tok) != json_tokener_success) {
        json_tokener_free(tok);
        json_object_put(json);
        return PARSER_ERROR;
    }

    size_t length;
    const char* json_string = json_object_to_json_string_length(
        json, JSON_C_TO_STRING_PLAIN, &length);
    if (json_string == NULL) {
        json_tokener_free(tok);
        json_object_put(json);
        return TOOLCHAIN_ERROR;
    }

    char* new_buf = malloc(length + 1);
    if (new_buf == NULL) {
        json_tokener_free(tok);
        json_object_put(json);
        return TOOLCHAIN_ERROR;
    }

    memcpy(new_buf, json_string, length);
    new_buf[length] = 0;
    *out_size = length + 1;
    *out_buf = new_buf;

    json_tokener_free(tok);
    json_object_put(json);
    return PARSER_OKAY;
}