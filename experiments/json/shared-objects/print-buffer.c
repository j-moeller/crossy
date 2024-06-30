#include "print-buffer.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int init_print_buffer(print_buffer_t* print_buf, size_t size)
{
    size = (size + MULTIPLE - 1) & -MULTIPLE; // round to MULTIPLE

    char* buf;
    if (NULL == (buf = (char*)malloc(sizeof(char) * size))) {
        return -1;
    }
    buf[0] = 0;
    print_buf->buffer = buf;
    print_buf->size = size;
    print_buf->position = 0;
    return 0;
}

void print_buffer_destroy(print_buffer_t* print_buf)
{
    free(print_buf->buffer);
    free(print_buf);
}

int print_buffer_memcpy(print_buffer_t* print_buf, const char* source,
                        size_t length)
{
    if (print_buffer_reserve(print_buf, length)) {
        return -1;
    }

    // append to the buffer and move position
    memcpy(&print_buf->buffer[print_buf->position], source, length);
    print_buf->position += length;
    print_buf->buffer[print_buf->position] = 0;
    return 0;
}

char* print_buffer_get_data(print_buffer_t* print_buf)
{
    return print_buf->buffer;
}

size_t print_buffer_get_length(print_buffer_t* print_buf)
{
    return print_buf->position + 1;
}

int print_buffer_printf(print_buffer_t* print_buf, const char* format, ...)
{
    va_list args;
    va_list copy;
    va_start(args, format);
    va_copy(copy, args);

    int required_size = vsnprintf(NULL, 0, format, args) + 1;
    if (print_buffer_reserve(print_buf, required_size)) {
        return -1;
    }

    vsnprintf(&print_buf->buffer[print_buf->position], required_size, format,
              copy);
    print_buf->position += required_size - 1;
    va_end(args);
    va_end(copy);
    return 0;
}

int print_buffer_reserve(print_buffer_t* print_buf, size_t size)
{
    // if there is not enough space left realloc the buffer and double its size
    size_t new_size = print_buf->size;
    while (print_buf->position + size + 1 >
           new_size) { // double size of buffer as many times as required
        new_size *= 2;
    }
    if (new_size > print_buf->size) { // realloc buffer to new size
        char* new_buf;
        if (NULL == (new_buf = (char*)realloc(print_buf->buffer, new_size))) {
            return -1;
        }
        print_buf->buffer = new_buf;
        print_buf->size = new_size;
    }
    return 0;
}
