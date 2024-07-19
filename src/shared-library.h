#ifndef SHARED_LIBRARY_H
#define SHARED_LIBRARY_H

#include <cstdlib>
#include <dlfcn.h>
#include <stdio.h>

template <typename F> struct SharedLibrary {
    void* handle;
    F fn;
};

template <typename F>
int get_interface_fn(SharedLibrary<F>* lib, const char* path,
                     const char* fnname)
{
    char* error;

    lib->handle = dlopen(path, RTLD_LAZY);
    if (!lib->handle) {
        fprintf(stderr, "cannot load library: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    lib->fn = (F)dlsym(lib->handle, fnname);
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "cannot resolve function: %s\n", error);

        if (dlclose(lib->handle) != 0) {
            if ((error = dlerror()) != NULL) {
                fprintf(stderr, "cannot close library: %s\n", error);
            } else {
                fprintf(stderr, "cannot close library for unknown reason\n");
            }
        }

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif