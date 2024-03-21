#include "Launcher.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <jni.h>
#include <ostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include <vector>

// Uses the JNI to start Java Targets
// https://docs.oracle.com/en/java/javase/19/docs/specs/jni/

static JavaVM* vm;
static JNIEnv* env;
static jobject object;
static jmethodID target_method;

static uint32_t* shm_out;

void initJVM(std::string classPath, std::string className,
             std::string methodName)

{
    const char* javaAgentOptionString = "-javaagent:build/java/paco-agent.jar";
    char javaAgentOption[strlen(javaAgentOptionString) + 1];
    strcpy(javaAgentOption, javaAgentOptionString);

    std::string classPathOptionString = "-Djava.class.path=" + classPath;
    char classPathOption[classPathOptionString.length() + 1];
    strcpy(classPathOption, classPathOptionString.c_str());

    const char* libraryPathOptionString = "-Djava.library.path=build/java";
    char libraryPathOption[strlen(libraryPathOptionString) + 1];
    strcpy(libraryPathOption, libraryPathOptionString);

    JavaVMInitArgs vm_args;

    JavaVMOption* options = new JavaVMOption[3]; // JVM invocation options
    options[0].optionString = javaAgentOption;
    options[1].optionString = classPathOption; // where to find java .class
    options[2].optionString =
        libraryPathOption; // where to find libconnector.so
    // options[3].optionString = "-DDEBUG";

    vm_args.version = JNI_VERSION_1_6; // min version
    vm_args.nOptions = 3;
    vm_args.options = options;      // provide options
    vm_args.ignoreUnrecognized = 1; // fail on error

    // Construct a VM
    jint res = JNI_CreateJavaVM(&vm, (void**)&env, &vm_args);

    delete[] options; // we no longer need the initialisation options.

    if (res != JNI_OK) {
        throw std::runtime_error("JVM init failed");
    }

    // load the class
    jclass cls = env->FindClass(className.c_str());
    if (cls == nullptr) {
        vm->DestroyJavaVM();
        throw std::runtime_error("Java class not found");
    }

    // invoke the constructor to create an object of the class
    jmethodID constructor = env->GetMethodID(cls, "<init>", "()V");
    if (constructor == nullptr) {
        vm->DestroyJavaVM();
        throw std::runtime_error("Java class constructor not found");
    }
    object = env->NewObject(cls, constructor);

    // find the target method of the object
    target_method = env->GetMethodID(cls, methodName.c_str(), "([B)[B");
    if (target_method == nullptr) {
        vm->DestroyJavaVM();
        throw std::runtime_error("Java target method not found");
    }
}

int run(const uint8_t* data, uint32_t size, uint8_t* out_buf, uint32_t out_size)
{
    // create a java byte array
    jbyteArray buf = env->NewByteArray(size);
    if (nullptr == buf) {
        return -1;
    }

    // fill the bytearray with data
    env->SetByteArrayRegion(buf, 0, size, (jbyte*)data);

    // call the target method
    jbyteArray arr =
        (jbyteArray)env->CallObjectMethod(object, target_method, buf);

    // free the java input array
    env->ReleaseByteArrayElements(buf, nullptr, JNI_ABORT);

    // check for exceptions
    if (env->ExceptionCheck() == JNI_TRUE || arr == nullptr) {
        // env->ExceptionDescribe();
        env->ExceptionClear();
        return 1;
    }

    uint32_t buffer_size = env->GetArrayLength(arr);
    if (buffer_size + 1 > out_size) {
        return -1;
    }

    jbyte* jvm_out = env->GetByteArrayElements(arr, nullptr);

    // copy bytes from java returned array
    memcpy(out_buf, jvm_out, buffer_size);
    out_buf[buffer_size] = 0;

    env->ReleaseByteArrayElements(arr, jvm_out, JNI_ABORT);

    return 0;
}

int main(int argc, char** argv)
{
    // QUESTION: I somehow do not pass the executable as the first argument, is
    // this required?
    if (argc != 3) {
        std::cerr
            << "usage: ./jvm_launcher [classpath] [class-name] [method-name]"
            << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string classpath(argv[0]);
    std::string className(argv[1]);
    std::string methodName(argv[2]);

    // # Init

    // ## Load environment names
    std::string prefix = std::string(std::getenv("FUZZER_BRIDGE_PREFIX"));

    // # Init fd_in, fd_out
    int fd_in = std::atoi(std::getenv("FUZZER_BRIDGE_FD_IN"));
    int fd_out = std::atoi(std::getenv("FUZZER_BRIDGE_FD_OUT"));

    // # Init shared memory
    std::string shm_output_name = prefix + "-output";
    int shm_output_size =
        std::atoi(std::getenv("FUZZER_BRIDGE_SHM_OUTPUT_SIZE"));

    int shm_output_fd =
        shm_open(shm_output_name.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_output_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    shm_out = static_cast<uint32_t*>(mmap(NULL, shm_output_size,
                                          PROT_READ | PROT_WRITE, MAP_SHARED,
                                          shm_output_fd, 0));

    if (shm_out == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    initJVM(classpath, className, methodName);

    uint32_t buf32 = 0;
    write(fd_out, &buf32, sizeof(buf32));

    while (true) {
        // Read input
        uint64_t Size;
        if (read(fd_in, &Size, sizeof(Size)) <= 0) {
            break;
        }

        // Clear out the shared memory before writing to it; just to be safe.
        memset(shm_out, 0, shm_output_size);

        uint8_t* Data = new uint8_t[Size];
        read(fd_in, Data, Size);

        shm_out[0] = run(Data, Size, (uint8_t*)&shm_out[1],
                         shm_output_size - sizeof(shm_out[0]));
        delete[] Data;

        if (shm_out[0] != 0) {
            shm_out[1] = 0;
        }

        uint64_t buf64 = 0;
        write(fd_out, &buf64, sizeof(buf64));
    }

    munmap(shm_out, shm_output_size);

    return EXIT_SUCCESS;
}
