#ifndef JVM_CONNECTOR
#define JVM_CONNECTOR

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <jni.h>
#include <string>

void initJVM(std::string classPath, std::string className,
             std::string methodName);

int run(const uint8_t* data, size_t size, uint8_t** out, size_t* out_size);

#endif