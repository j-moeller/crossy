#ifndef __PRINT_BUFFER_H__
#define __PRINT_BUFFER_H__

#include <stddef.h>
#define MULTIPLE 128 // must be a power of 2

typedef struct print_buffer {
    char* buffer;
    size_t size;     // total size of the buffer
    size_t position; // position of the writer head
} print_buffer_t;

/**
 * @brief Initialize a print_buffer_t
 *
 * Allocates a buffer with size rounded to MULTIPLE.
 * The buffer is nullterminated initially.
 * The internal buffer must be freed to prevent memory leaks. Either by
 free(print_buf->buf) or print_buffer_destroy
 *
 * @param print_buf the print_buffer to be initialized, must be a valid pointer
 to a print_buffer_t
 * @param size the required buffer size
 * @return int 0 on success,
 *         -1 if allocation fails
 */
int init_print_buffer(print_buffer_t* print_buf, size_t size);

/**
 * @brief Frees the internal buffer and the print_buffer struct
 *
 * @param print_buf valid pointer to print_buffer_t
 */
void print_buffer_destroy(print_buffer_t* print_buf);

/**
 * @brief Copies length bytes from source and appends them to print_buf
 *
 * Increases the size of the buffer if required.
 * The buffer is nullterminated.
 * Therefore it's content is a valid string if the copied data doesn't contain
 * nullbytes.
 *
 * @param print_buf valid pointer to print_buffer_t
 * @param source valid pointer to bytearray to be copied
 * @param length amount of bytes to be copied
 * @return int 0 on success,
 *         -1 if reallocation fails
 */
int print_buffer_memcpy(print_buffer_t* print_buf, const char* source,
                        size_t length);

/**
 * @brief Get the pointer to the internal bytearray
 *
 * @param print_buf valid pointer to print_buffer_t
 * @return char* pointer to the internal buffer bytearray
 */
char* print_buffer_get_data(print_buffer_t* print_buf);

/**
 * @brief Get the amount of bytes that were written to the buffer including the
 * final nullbyte
 *
 * @param print_buf valid pointer to print_buffer_t
 * @return size_t amount of bytes that were written to the buffer including the
 * final nullbyte
 */
size_t print_buffer_get_length(print_buffer_t* print_buf);

/**
 * @brief Prints formatted content to the end of the buffer
 *
 * The buffer is reallocated if needed. The buffer is nullterminated afterwards.
 *
 * @param print_buf valid pointer to print_buffer_t
 * @param format format string
 * @param ... format string parameters
 * @return int 0 on success,
 *         -1 if reallocation fails
 */
int print_buffer_printf(print_buffer_t* print_buf, const char* format, ...);

/**
 * @brief Reserves at least size+1 bytes at the end of the buffer by
 * reallocating the buffer if required
 *
 * @param print_buf valid pointer to print_buffer_t
 * @param size amount of reserved bytes
 * @return int 0 on success,
 *         -1 if reallocation fails
 */
int print_buffer_reserve(print_buffer_t* print_buf, size_t size);

#endif //__PRINT_BUFFER_H__