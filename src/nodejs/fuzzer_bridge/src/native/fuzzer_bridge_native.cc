// fuzzer_bridge_native.cc
#include <node.h>
#include <v8.h>
// #include <v8-coverage.h>

#include <fcntl.h>
#include <iostream>
#include <memory>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace FuzzerBridgeNative
{
static uint8_t* shm_edges = NULL;
static uint32_t shm_edges_size;
static std::string shm_edges_name;

static sem_t* sem_empty;
static sem_t* sem_full;
static sem_t* sem_mutex;

std::string toStdString(v8::Local<v8::String> s, v8::Isolate* isolate)
{
    int len = s->Utf8Length(isolate);
    char* buffer = new char[len + 1];
    s->WriteUtf8(isolate, buffer, len + 1);
    buffer[len] = 0;
    std::string ret = std::string(buffer);
    delete[] buffer;
    return ret;
}

void openSharedMemory(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate* isolate = args.GetIsolate();

    // Check the number of arguments passed.
    if (args.Length() != 2) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Wrong number of arguments")
                .ToLocalChecked()));
        return;
    }

    // Check the argument types
    if (!args[0]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Argument0 is not a string")
                .ToLocalChecked()));
        return;
    }

    // Is arg[1]>=0 and integer?
    if (!args[1]->IsNumber() || !args[1]->IsUint32()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Argument1 is not a number")
                .ToLocalChecked()));
        return;
    }

    std::string shm_edges_name_arg =
        toStdString(args[0].As<v8::String>(), isolate);
    uint32_t size_arg = args[1].As<v8::Uint32>()->Value();

    if (size_arg <= 0) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Argument1 must be larger than 0")
                .ToLocalChecked()));
    }

    // Open Shared Memory
    std::cerr << "Edge names: " << shm_edges_name_arg.c_str() << std::endl;
    int fd = shm_open(shm_edges_name_arg.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

    if (fd <= 0) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "shm_open failed")
                .ToLocalChecked()));
    }

    // MMap
    shm_edges = (uint8_t*)mmap(NULL, sizeof(uint8_t) * size_arg,
                               PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shm_edges == NULL) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "mmap failed").ToLocalChecked()));
    }

    shm_edges_size = size_arg;
    shm_edges_name = shm_edges_name_arg;
}

void writeToSharedMemory(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate* isolate = args.GetIsolate();

    if (shm_edges == NULL) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate,
                                    "Shared memory has not been initialized")
                .ToLocalChecked()));
        return;
    }

    // Check the number of arguments passed.
    if (args.Length() != 1) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Wrong number of arguments")
                .ToLocalChecked()));
        return;
    }

    // Check the argument types
    if (!args[0]->IsUint8Array()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Argument0 is not a Uint8Array")
                .ToLocalChecked()));
        return;
    }

    v8::Local<v8::Uint8Array> message = args[0].As<v8::Uint8Array>();

    if (message->Length() > shm_edges_size) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate,
                                    "Argument0.size exceeds shared memory size")
                .ToLocalChecked()));
        return;
    }

    message->CopyContents(shm_edges, message->ByteLength());
}

void openSemaphores(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate* isolate = args.GetIsolate();

    // Check the number of arguments passed.
    if (args.Length() != 1) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Wrong number of arguments")
                .ToLocalChecked()));
        return;
    }

    // Check the argument types
    if (!args[0]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Argument0 is not a String")
                .ToLocalChecked()));
        return;
    }

    std::string prefix = toStdString(args[0].As<v8::String>(), isolate);

    {
        std::string name = prefix + "-full";
        sem_full = sem_open(name.c_str(), 0);
        if (sem_full == SEM_FAILED) {
            perror("semaphores.full");
            exit(EXIT_FAILURE);
        }
    }
    {
        std::string name = prefix + "-empty";
        sem_empty = sem_open(name.c_str(), 0);
        if (sem_empty == SEM_FAILED) {
            perror("sem_empty");
            exit(EXIT_FAILURE);
        }
    }
    {
        std::string name = prefix + "-mutex";
        sem_mutex = sem_open(name.c_str(), 0);
        if (sem_mutex == SEM_FAILED) {
            perror("sem_mutex");
            exit(EXIT_FAILURE);
        }
    }
}

void notifyParent(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    sem_post(sem_mutex);
    sem_post(sem_full);
}

void waitForParent(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    sem_wait(sem_empty);
    sem_wait(sem_mutex);
}

void Initialize(v8::Local<v8::Object> exports)
{
    NODE_SET_METHOD(exports, "openSharedMemory", openSharedMemory);
    NODE_SET_METHOD(exports, "writeToSharedMemory", writeToSharedMemory);
    NODE_SET_METHOD(exports, "openSemaphores", openSemaphores);
    NODE_SET_METHOD(exports, "notifyParent", notifyParent);
    NODE_SET_METHOD(exports, "waitForParent", waitForParent);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)

} // namespace FuzzerBridgeNative