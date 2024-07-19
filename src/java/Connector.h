#ifndef JVM_CONNECTOR
#define JVM_CONNECTOR

#include <cstdint>

#define INIT_FINISHED 0
#define RUN_FINISHED 1
#define REGISTER_CTRS 2

extern "C" void init();

#endif