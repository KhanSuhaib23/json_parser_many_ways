#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    size_t sz;
    const char* buff;
} String;

String string_new(const char* str, size_t sz);
void printf_tabs(uint16_t num);
bool string_equal(String s1, String s2);

#endif // UTIL_H_
