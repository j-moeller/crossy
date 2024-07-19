#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "yajl/src/api/yajl_gen.h"
#include "yajl/src/api/yajl_parse.h"

/* non-zero when we're reformatting a stream */
static int s_streamReformat = 0;

#define GEN_AND_RETURN(func)                                                   \
    {                                                                          \
        yajl_gen_status __stat = func;                                         \
        if (__stat == yajl_gen_generation_complete && s_streamReformat) {      \
            yajl_gen_reset(g, "\n");                                           \
            __stat = func;                                                     \
        }                                                                      \
        return __stat == yajl_gen_status_ok;                                   \
    }

static int reformat_null(void* ctx)
{
    yajl_gen g = (yajl_gen)ctx;
    GEN_AND_RETURN(yajl_gen_null(g));
}

static int reformat_boolean(void* ctx, int boolean)
{
    yajl_gen g = (yajl_gen)ctx;
    GEN_AND_RETURN(yajl_gen_bool(g, boolean));
}

static int reformat_number(void* ctx, const char* s, size_t l)
{
    yajl_gen g = (yajl_gen)ctx;
    GEN_AND_RETURN(yajl_gen_number(g, s, l));
}

static int reformat_string(void* ctx, const unsigned char* stringVal,
                           size_t stringLen)
{
    yajl_gen g = (yajl_gen)ctx;
    GEN_AND_RETURN(yajl_gen_string(g, stringVal, stringLen));
}

static int reformat_map_key(void* ctx, const unsigned char* stringVal,
                            size_t stringLen)
{
    yajl_gen g = (yajl_gen)ctx;
    GEN_AND_RETURN(yajl_gen_string(g, stringVal, stringLen));
}

static int reformat_start_map(void* ctx)
{
    yajl_gen g = (yajl_gen)ctx;
    GEN_AND_RETURN(yajl_gen_map_open(g));
}

static int reformat_end_map(void* ctx)
{
    yajl_gen g = (yajl_gen)ctx;
    GEN_AND_RETURN(yajl_gen_map_close(g));
}

static int reformat_start_array(void* ctx)
{
    yajl_gen g = (yajl_gen)ctx;
    GEN_AND_RETURN(yajl_gen_array_open(g));
}

static int reformat_end_array(void* ctx)
{
    yajl_gen g = (yajl_gen)ctx;
    GEN_AND_RETURN(yajl_gen_array_close(g));
}

static yajl_callbacks callbacks = {reformat_null,
                                   reformat_boolean,
                                   NULL,
                                   NULL,
                                   reformat_number,
                                   reformat_string,
                                   reformat_start_map,
                                   reformat_map_key,
                                   reformat_end_map,
                                   reformat_start_array,
                                   reformat_end_array};

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    // Fuzzer input uses length to specify end, but parser requires
    // null-terminated string
    unsigned char* null_buf = malloc(sizeof(unsigned char) * (size + 1));
    if (null_buf == NULL) {
        return TOOLCHAIN_ERROR;
    }
    null_buf[size] = 0;
    memcpy(null_buf, buf, size);

    // Use yajls reformat JSON approach, see
    // https://lloyd.github.io/yajl/yajl-2.1.0/reformatter_2json_reformat_8c-example.html
    yajl_handle gen_handle;

    /* generator config */
    yajl_gen gen;
    yajl_status gen_stats;
    int retval = PARSER_OKAY;

    /* configure gen */
    gen = yajl_gen_alloc(NULL);
    // yajl_gen_config(gen, yajl_gen_beautify, 0);
    // yajl_gen_config(gen, yajl_gen_escape_solidus, 1);
    // yajl_gen_config(gen, yajl_gen_validate_utf8, 1);

    /* configure gen_handle */
    gen_handle = yajl_alloc(&callbacks, NULL, (void*)gen);
    // yajl_config(gen_handle, yajl_allow_comments, 1);
    // yajl_config(gen_handle, yajl_dont_validate_strings, 1);

    // yajl_config(gen_handle, yajl_allow_multiple_values, 1);
    // s_streamReformat = 1;

    gen_stats = yajl_parse(gen_handle, null_buf, size);
    free(null_buf);

    if (gen_stats == yajl_status_ok) {
        const unsigned char* yajl_buf;
        size_t yajl_buf_len;
        yajl_gen_get_buf(gen, &yajl_buf, &yajl_buf_len);
        // fwrite(yajl_buf, 1, yajl_buf_len, stdout); // print

        char* ret_buf;

        if (yajl_buf_len > 0 && yajl_buf[yajl_buf_len - 1] == '\0') {
            ret_buf = malloc(sizeof(char) * yajl_buf_len);
            memcpy(ret_buf, (char*)yajl_buf, yajl_buf_len);
            *out_buf = ret_buf;
            *out_size = yajl_buf_len;
        } else {
            ret_buf = malloc(sizeof(char) * (yajl_buf_len + 1));
            ret_buf[yajl_buf_len] = 0;
            memcpy(ret_buf, (char*)yajl_buf, yajl_buf_len);
            *out_buf = ret_buf;
            *out_size = yajl_buf_len + 1;
        }

        retval = PARSER_OKAY;
    } else {
        *out_buf = 0;
        *out_size = 0;
        retval = PARSER_ERROR;
    }

    yajl_gen_clear(gen);
    yajl_gen_free(gen);
    yajl_free(gen_handle);

    return retval;
}