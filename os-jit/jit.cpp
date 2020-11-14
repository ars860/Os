#include <iostream>
#include <sys/mman.h>
#include <cstring>
#include <vector>

//return 1_000_000 + 1_000_000
unsigned char code[] = {0x55,
                        0x48, 0x89, 0xe5,
                        0xc7, 0x45, 0xf8, /*first  value*/0x40, 0x42, 0x0f, 0x00,
                        0xc7, 0x45, 0xfc, /*second value*/0x40, 0x42, 0x0f, 0x00,
                        0x8b, 0x55, 0xf8,
                        0x8b, 0x45, 0xfc,
                        0x01, 0xd0,
                        0x5d,
                        0xc3};

const size_t len = sizeof(code);
const size_t first_arg_begin = 7;
const size_t second_arg_begin = 14;

void exec(void *f) {
    if (mprotect(f, len, PROT_EXEC) == -1) {
        perror("Cant exec mapped function");
        exit(EXIT_FAILURE);
    }

    std::cout << reinterpret_cast<int (*)()>(f)() << '\n';
}

std::vector<unsigned char> get_chars(unsigned int x) {
    std::vector<unsigned char> res(4);
    for (int i = 0; i < res.size(); i++) {
        res[i] = static_cast<unsigned char>(x);
        x = (x >> 8);
    }
    return res;
}

void set_args(unsigned int first, unsigned int second, void *f) {
    std::vector<unsigned char> first_chars = get_chars(first);
    std::vector<unsigned char> second_chars = get_chars(second);

    if (mprotect(f, len, PROT_WRITE) == -1) {
        perror("Cant write to function memory");
        exit(EXIT_FAILURE);
    }

    memcpy(static_cast<unsigned char *>(f) + first_arg_begin, first_chars.data(), first_chars.size());
    memcpy(static_cast<unsigned char *>(f) + second_arg_begin, second_chars.data(), second_chars.size());
}

void clear_memory(void *f, size_t len) {
    if (munmap(f, len) == -1) {
        perror("Cant clear memory");
        exit(EXIT_FAILURE);
    }
}

int main() {
    void *f = mmap(nullptr, len, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (f == MAP_FAILED) {
        perror("Cant map memory");
        exit(EXIT_FAILURE);
    }
    memcpy(f, code, sizeof(code));

    //default arguments
    exec(f);

    unsigned int x, y;
    std::cin >> x >> y;

    set_args(x, y, f);

    //user input
    exec(f);

    clear_memory(f, len);
}