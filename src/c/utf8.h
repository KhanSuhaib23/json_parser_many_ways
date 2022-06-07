#ifndef UTF8_H_
#define UTF8_H_

#include <stdint.h>

void* utf8_decode_dfa(void* buff, uint32_t* c, uint32_t* e);
void* utf8_validate_dfa(uint32_t* state, void* buff);

#endif // UTF8_H_
