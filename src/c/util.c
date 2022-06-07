#include "util.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

String string_new(const char* str, size_t sz) {
    return (String) {
        .sz = sz,
        .buff = str
    };
}

bool string_equal(String s1, String s2) {
    if (s1.sz != s2.sz) return false;

    for (size_t i = 0; i < s1.sz; ++i) {
        if (s1.buff[i] != s2.buff[i]) {
            return false;
        }
    }
    return true;
}

void printf_tabs(uint16_t num) {
    for (; num; --num) {
        printf("    ");
    }
}
