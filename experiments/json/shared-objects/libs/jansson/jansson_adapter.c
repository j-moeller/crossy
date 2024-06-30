#include <string.h>

#include "common.h"

#include "jansson.h"

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    json_t* json;
    json_error_t error;
    if (NULL == (json = json_loadb(buf, size, 0, &error))) {
        return PARSER_ERROR;
    }

    size_t required_size =
        json_dumpb(json, NULL, 0, JSON_COMPACT); // determine required size of buffer
    if (required_size == 0) {
        json_decref(json);
        return PARSER_ERROR;
    }

    char* write_buffer; // create buffer
    if (NULL == (write_buffer = (char*)malloc(required_size + 1))) {
        json_decref(json);
        return TOOLCHAIN_ERROR;
    }

    // write json string to buffer
    int bytes_written = json_dumpb(json, write_buffer, required_size, JSON_COMPACT);
    if (bytes_written == 0) {
        json_decref(json);
        free(write_buffer);
        return PARSER_ERROR;
    }
    write_buffer[required_size] = 0;

    *out_buf = write_buffer;
    *out_size = required_size + 1;

    json_decref(json); // use this instead of free to prevent memory leaks

    return PARSER_OKAY;
}