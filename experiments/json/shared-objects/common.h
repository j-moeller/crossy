#ifndef SHARED_OBJECTS_COMMON_H
#define SHARED_OBJECTS_COMMON_H

// Directives to enable desired functions to be exported
#if __GNUC__ >= 4
#define LIB_EXPORT __attribute__((visibility("default")))
#else
#define LIB_EXPORT
#endif

#define PARSER_OKAY 0
#define PARSER_ERROR 1
#define TOOLCHAIN_ERROR -1 // for errors that are not related to parsing

#endif // SHARED_OBJECTS_COMMON_H
