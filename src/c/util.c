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

#define push_bits(l, r, n) (l) <<= (n), (l) |= ((r) & ((1 >> (n + 1)) - 1))

Rune get_rune(const char* pos) {
    uint32_t rune = 0;
    uint32_t successive_bytes = 0, i;
    uint8_t ch = pos[0];

    if (ch & 0x80) {
        if (((ch ^ 0xf0) >> 3) == 0) {
            push_bits(rune, ch, 3);
            successive_bytes = 3;
        } else if (((ch ^ 0xe0) >> 4) == 0) {
            push_bits(rune, ch, 4);
            successive_bytes = 2;
        } else if (((ch ^ 0xa0) >> 5) == 0) {
            push_bits(rune, ch, 5);
            successive_bytes = 1;
        } else {
            fprintf(stderr, "[ERROR]: Parsing UTF-8 posing, expected a header byte got something else\n");
            exit(-1);
        }

    } else {
        push_bits(rune, ch, 7);
    }

    ++pos;
    ch = pos[0];

    for (i = 0; i < successive_bytes; ++i) {
        if (((ch ^ 0x80) >> 6) != 0) {
            fprintf(stderr, "[ERROR]: Parsing UTF-8 posing, expected successive byte got something else");
            exit(-1);
        }
        push_bits(rune, ch, 6);
        ++pos;
        ch = pos[0];
    }

    return (Rune) { .code = rune, .sz = successive_bytes + 1 };
}

#undef push_bits

void printf_tabs(uint16_t num) {
    for (; num; --num) {
        printf("    ");
    }
}
