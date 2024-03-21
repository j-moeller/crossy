#include "tiny-json_adapter.h"
#include "../../libs/tiny-json/tiny-json.h"
#include "../../../../../src/interface/common.h"
#include "print-buffer.h"
#include <stdbool.h>
#include <string.h>

#define BUFFER_SIZE 256

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    // string must be nullterminated
    char* str = malloc(sizeof(char) * (size) + 1);
    memcpy(str, buf, size);
    str[size] = 0;

    json_t mem[BUFFER_SIZE];
    const json_t* json = json_create(str, mem, sizeof(mem) / sizeof(*mem));
    if (json == NULL) {
        free(str);
        return PARSER_ERROR;
    }

    print_buffer_t* print_buf;
    if (NULL == (print_buf = malloc(sizeof(print_buffer_t)))) {
        free(str);
        return TOOLCHAIN_ERROR;
    }
    if (init_print_buffer(print_buf, size)) {
        free(str);
        free(print_buf);
        return TOOLCHAIN_ERROR;
    }

    if (print_json(print_buf, json)) {
        free(str);
        free(print_buf);
        return TOOLCHAIN_ERROR;
    }

    free(str);
    *out_buf = print_buffer_get_data(print_buf);
    *out_size = print_buffer_get_length(print_buf);
    free(print_buf);

    return PARSER_OKAY;
}

int print_json(print_buffer_t* print_buf, const json_t* json)
{
    const char* name = json_getName(json);
    if (name != NULL) {
        if (print_buffer_printf(print_buf, "\"%s\":", name)) {
            return -1;
        }
    }

    int64_t integer_value;
    double double_value;
    switch (json->type) {
    case JSON_OBJ:
        if (print_buffer_printf(print_buf, "{")) {
            return -1;
        }
        if (print_json_composite(print_buf, json)) {
            return -1;
        }
        if (print_buffer_printf(print_buf, "}")) {
            return -1;
        }
        break;
    case JSON_ARRAY:
        if (print_buffer_printf(print_buf, "[")) {
            return -1;
        }
        if (print_json_composite(print_buf, json)) {
            return -1;
        }
        if (print_buffer_printf(print_buf, "]")) {
            return -1;
        }
        break;
    case JSON_TEXT:
        if (print_buffer_printf(print_buf, "\"%s\"", json_getValue(json))) {
            return -1;
        }
        break;
    case JSON_BOOLEAN:
        if (json_getBoolean(json)) {
            if (print_buffer_printf(print_buf, "true")) {
                return -1;
            }
        } else {
            if (print_buffer_printf(print_buf, "false")) {
                return -1;
            }
        }
        break;
    case JSON_INTEGER:
        integer_value = json_getInteger(json);
        if (print_buffer_printf(print_buf, "%lld", integer_value)) {
            return -1;
        }
        break;
    case JSON_REAL:
        double_value = json_getReal(json);
        if (print_buffer_printf(print_buf, "%g", double_value)) {
            return -1;
        }
        break;
    case JSON_NULL:
        if (print_buffer_printf(print_buf, "null")) {
            return -1;
        }
        break;
    }
    return 0;
}

int print_json_composite(print_buffer_t* print_buf, const json_t* json)
{
    const json_t* child = json_getChild(json);
    if (child != NULL) {
        if (print_json(print_buf, child)) {
            return -1;
        }

        child = json_getSibling(child);
        while (NULL != child) {
            if (print_buffer_printf(print_buf, ",")) {
                return -1;
            }
            if (print_json(print_buf, child)) {
                return -1;
            }
            child = json_getSibling(child);
        }
    }
    return 0;
}