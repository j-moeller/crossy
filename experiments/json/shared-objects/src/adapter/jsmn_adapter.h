#include "../../libs/jsmn/jsmn.h"
#include "print-buffer.h"

/**
 * @brief print all values from a JSON object or array to a print_buffer
 *
 * @param print_buf valid pointer to print_buffer_t
 * @param json pointer to json string
 * @param tokens pointer to array of parsed json tokens
 * @param index index of the parent token of the object ot array
 * @return int: the number of tokens written, -1 if reallocation of the
 * print_buffer fails, -2 if the parsed json tree was incorrect
 */
int print_json_composite(print_buffer_t* print_buf, const char* json,
                         jsmntok_t* tokens, size_t index);

/**
 * @brief print json to a print_buffer
 *
 * prints the tokens name and it's value
 * if the value is an object or array, the values are traversed and printed
 * recursively
 *
 * @param print_buf valid pointer to print_buffer_t
 * @param json pointer to json string
 * @param tokens pointer to array of parsed json tokens
 * @param index index of the token to be printed
 * @return int: the number of tokens written, -1 if reallocation of the
 * print_buffer fails, -2 if the parsed json tree was incorrect
 */
int print_json(print_buffer_t* print_buf, const char* json, jsmntok_t* tokens,
               size_t index);
