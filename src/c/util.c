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

const uint32_t utf8_len[] = {
    4,
    3,
    2, 2,
    0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1
};


Rune get_rune(const char* pos) {
    #define get_utf8_len_index(b) (((b) >> 4) ^ 0x0f)
    #define push_bits(l, r, n) (l) <<= (n), (l) |= ((r) & ((1 << (n + 1)) - 1))
    #define verify_and_get(b, pos, n) (b) = (pos)[(n)]; if (utf8_len[get_utf8_len_index(b)]) { goto short_utf8; }


    Rune rune = { .sz = 0, .code = 0};
    uint32_t successive_bytes = 0, i;
    uint8_t b1 = pos[0];
    uint32_t range_max, range_min;
    uint8_t b2, b3, b4;

    switch (utf8_len[get_utf8_len_index(b1)]) {
        case 0: {
            fprintf(stderr, "[ERROR]: Parsing UTF-8, got 10xx xxxx byte when a header byte was expectec\n");
            rune.code = 0xff; // TODO(suhaibnk): put more appropriate error code
            return rune;
        } break;
        case 1: {
            rune.sz = 1;
            rune.code = (uint32_t) b1;
            range_min = 0x00;
            range_max = 0x7f;

            goto check_range;
        } break;
        case 2: {
            rune.sz = 2;
            push_bits(rune.code, b1, 5);

            verify_and_get(b2, pos, 1);
            push_bits(rune.code, b2, 6);

            range_min = 0x0080;
            range_max = 0x07ff;

            goto check_range;
        } break;
        case 3: {
            rune.sz = 3;
            push_bits(rune.code, b1, 4);

            verify_and_get(b2, pos, 1);
            push_bits(rune.code, b2, 6);

            verify_and_get(b3, pos, 2);
            push_bits(rune.code, b3, 6);

            range_min = 0x0800;
            range_max = 0xffff;

            goto check_range;
        } break;
        case 4: {
            rune.sz = 4;
            push_bits(rune.code, b1, 3);

            verify_and_get(b2, pos, 1);
            push_bits(rune.code, b2, 6);

            verify_and_get(b3, pos, 2);
            push_bits(rune.code, b3, 6);

            verify_and_get(b4, pos, 3);
            push_bits(rune.code, b4, 6);

            range_min = 0x010000;
            range_max = 0x10ffff;

            goto check_range;
        } break;
        default: {
            exit(-1);
            // TODO(suhaibnk): assest unreachable
        }


    }

    short_utf8:

    fprintf(stderr, "[ERROR]: Parsing UTF-8, expected more continuation bytes.");
    rune.sz = 0;
    rune.code = 0xff; // TODO(suhaibnk): put more appropriate error code

    goto end;

    check_range:

    if (rune.code < range_min || rune.code > range_max) {
        fprintf(stderr, "[ERROR]: Parsing UTF-8, found utf-8 value out of range of the enconding standards\n");
    }

    end:

    return rune;
}

#undef push_bits

void printf_tabs(uint16_t num) {
    for (; num; --num) {
        printf("    ");
    }
}
