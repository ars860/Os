#include <iostream>
#include <dlfcn.h>
#include "hello.h"
#include "world.h"

int main() {
    void *handle = dlopen("libexclamation.so", RTLD_LAZY);
    if (!handle) {
        std::cout << dlerror() << '\n';
        exit(EXIT_FAILURE);
    }

    auto exclamation = reinterpret_cast<std::string (*)()>(dlsym(handle, "exclamationMark"));
    if (!exclamation) {
        std::cout << dlerror() << '\n';
        exit(EXIT_FAILURE);
    }

    std::cout << printHello() << " " << printWorld() << exclamation() << std::endl;

    if (dlclose(handle) != 0) {
        std::cout << dlerror() << '\n';
        exit(EXIT_FAILURE);
    }
}