#ifndef PARSER_H_
#define PARSER_H_

#include "json.h"
#include "lexer.h"

Json_Node json_parse(const char* str);

#endif // PARSER_H_
