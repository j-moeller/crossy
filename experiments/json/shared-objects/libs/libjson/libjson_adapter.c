#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "print-buffer.h"

#include "libjson/json.h"

// copied from https://github.com/vincenthz/libjson/blob/master/jsonlint.c

int json_parse_callback(void* userdata, int type, const char* data,
                        uint32_t length)
{
    // invokes the json_printer which in turn invokes json_print_callback
    json_printer* printer = userdata;
    return json_print_raw(printer, type, data, length);
}

static int json_print_callback(void* userdata, const char* data,
                               uint32_t length)
{
    // prints to the print_buffer
    print_buffer_t* print_buf = userdata;
    return print_buffer_memcpy(print_buf, data, length);
}

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    json_parser parser;
    json_printer printer;
    print_buffer_t* print_buf;

    if (NULL == (print_buf = malloc(sizeof(print_buffer_t)))) {
        printf("TOOLCHAIN_ERROR malloc\n");
        return TOOLCHAIN_ERROR;
    }
    if (init_print_buffer(print_buf, size)) {
        free(print_buf);
        printf("TOOLCHAIN_ERROR init_print_buffer\n");
        return TOOLCHAIN_ERROR;
    }

    if (json_print_init(&printer, json_print_callback, print_buf)) {
        print_buffer_destroy(print_buf);
        printf("TOOLCHAIN_ERROR json_print_init\n");
        return TOOLCHAIN_ERROR;
    }

    if (json_parser_init(&parser, NULL, &json_parse_callback, &printer)) {
        print_buffer_destroy(print_buf);
        json_print_free(&printer);
        printf("TOOLCHAIN_ERROR json_parser_init\n");
        return TOOLCHAIN_ERROR; // only fails if memory allocation fails
    }

    if (json_parser_string(&parser, buf, size, NULL)) {
        print_buffer_destroy(print_buf);
        json_print_free(&printer);
        json_parser_free(&parser);
        // printf("PARSER_ERROR: %s\n", error_buf);
        return PARSER_ERROR;
    }

    if (!json_parser_is_done(&parser)) {
        print_buffer_destroy(print_buf);
        json_print_free(&printer);
        json_parser_free(&parser);
        return PARSER_ERROR;
    }

    *out_buf = print_buffer_get_data(print_buf);
    *out_size = print_buffer_get_length(print_buf);
    free(print_buf); // only free the struct, the buffer is kept and used for
                     // the return
    json_print_free(&printer);
    json_parser_free(&parser);
    return PARSER_OKAY;
}