#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    size_t sz;
    const char* buff;
} String;

typedef struct {
    uint32_t code;
    uint32_t sz;
} Rune;


String string_new(const char* str, size_t sz);
Rune get_rune(const char* pos);
void printf_tabs(uint16_t num);
bool string_equal(String s1, String s2);

#endif // UTIL_H_
