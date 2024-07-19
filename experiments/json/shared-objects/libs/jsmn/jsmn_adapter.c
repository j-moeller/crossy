#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "print-buffer.h"

#include "jsmn/jsmn.h"

#define BUFFER_SIZE 256

int print_json_composite(print_buffer_t* print_buf, const char* json,
                         jsmntok_t* tokens, size_t index);

int print_json(print_buffer_t* print_buf, const char* json, jsmntok_t* tokens,
               size_t index)
{
    jsmntok_t* token = &tokens[index];
    int written = -1;
    switch (token->type) {
    case JSMN_STRING:
        if (token->size > 0) {
            // this is a field name, print the name and then the following
            // value
            if (print_buffer_printf(print_buf,
                                    "\"%.*s\":", token->end - token->start,
                                    json + token->start)) {
                return -1;
            }

            written = print_json(print_buf, json, tokens, index + 1);
            if (written > 0) {
                return written + 1;
            } else if (written == 0) {
                // parsing error
                return -2;
            } else {
                // propagate error
                return written;
            }
        } else {
            // this is a string value
            if (print_buffer_printf(print_buf, "\"%.*s\"",
                                    token->end - token->start,
                                    json + token->start)) {
                return -1;
            }
            return 1;
        }
    case JSMN_PRIMITIVE:
        if (print_buffer_printf(print_buf, "%.*s", token->end - token->start,
                                json + token->start)) {
            return -1;
        }
        return 1;

    case JSMN_OBJECT:
        if (print_buffer_printf(print_buf, "{")) {
            return -1;
        }
        written = print_json_composite(print_buf, json, tokens, index);
        if (written < 0) {
            return written;
        }
        if (print_buffer_printf(print_buf, "}")) {
            return -1;
        }
        return written;
    case JSMN_ARRAY:
        if (print_buffer_printf(print_buf, "[")) {
            return -1;
        }
        written = print_json_composite(print_buf, json, tokens, index);
        if (written < 0) {
            return written;
        }
        if (print_buffer_printf(print_buf, "]")) {
            return -1;
        }
        return written;
    case JSMN_UNDEFINED:
        return -2;
    }
    return -1;
}

int print_json_composite(print_buffer_t* print_buf, const char* json,
                         jsmntok_t* tokens, size_t index)
{
    jsmntok_t* token = &tokens[index];
    int number_of_children = token->size;
    int offset = 1;

    for (int i = 0; i < number_of_children; i++) {
        int written = print_json(print_buf, json, tokens, index + offset);
        if (written < 0) {
            return written;
        }
        offset += written;
        if (i < number_of_children - 1) {
            // this is not the last child
            if (print_buffer_printf(print_buf, ",")) {
                return -1;
            }
        }
    }

    return offset;
}

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    jsmn_parser parser;
    jsmntok_t* tokens;
    size_t token_buf_size = BUFFER_SIZE;

    if (NULL == (tokens = malloc(sizeof(jsmntok_t) * token_buf_size))) {
        return TOOLCHAIN_ERROR;
    }

    print_buffer_t* print_buf;
    if (NULL == (print_buf = malloc(sizeof(print_buffer_t)))) {
        free(tokens);
        return TOOLCHAIN_ERROR;
    }
    if (init_print_buffer(print_buf, size)) {
        free(tokens);
        print_buffer_destroy(print_buf);
        return TOOLCHAIN_ERROR;
    }

    jsmn_init(&parser);
    int res = jsmn_parse(&parser, buf, size, tokens, token_buf_size);

    while (res == JSMN_ERROR_NOMEM) {
        token_buf_size *= 2;
        jsmntok_t* new_tokens =
            (jsmntok_t*)realloc(tokens, sizeof(jsmntok_t) * token_buf_size);
        if (new_tokens == NULL) {
            free(tokens);
            print_buffer_destroy(print_buf);
            return TOOLCHAIN_ERROR;
        }
        tokens = new_tokens;
        res = jsmn_parse(&parser, buf, size, tokens, token_buf_size);
    }

    if (res == JSMN_ERROR_INVAL || res == JSMN_ERROR_PART) {
        free(tokens);
        print_buffer_destroy(print_buf);
        return PARSER_ERROR;
    }

    // traverse and print see
    // https://github.com/zserge/jsmn/blob/master/example/jsondump.c

    res = print_json(print_buf, buf, tokens, 0);
    if (res < 0) {
        free(tokens);
        print_buffer_destroy(print_buf);
        if (res == -1) {
            return TOOLCHAIN_ERROR;
        } else {
            return PARSER_ERROR;
        }
    }

    *out_buf = print_buffer_get_data(print_buf);
    *out_size = print_buffer_get_length(print_buf);
    free(tokens);
    free(print_buf);

    return PARSER_OKAY;
}
