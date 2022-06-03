#ifndef LEXER_H_
#define LEXER_H_
#include <stdint.h>
#include "util.h"

typedef enum {
    Character_Class_Unknown,
    Character_Class_Whitespace,
    Character_Class_Single_Quote,
    Character_Class_Double_Quote,
    Character_Class_Comma,
    Character_Class_Character,
    Character_Class_Underscore,
    Character_Class_Digit,
    Character_Class_Dot,
    Character_Class_LBrace,
    Character_Class_RBrace,
    Character_Class_LBracket,
    Character_Class_RBracket,
    Character_Class_Hyphen,
    Character_Class_Colon,
    Character_Class_End
} Character_Class;

typedef enum {
    Json_Token_String,
    Json_Token_Ident,
    Json_Token_Integer,
    Json_Token_Number,
    Json_Token_LBrace,
    Json_Token_RBrace,
    Json_Token_RBracket,
    Json_Token_LBracket,
    Json_Token_Comma,
    Json_Token_Colon
} Json_Token_Enum;

typedef uint32_t Json_Token_Tag;

typedef struct {
    Json_Token_Tag tag;
    union {
        String string;
        int64_t integer;
        double number;
        bool boolean;
    };
} Json_Token;

typedef struct {
    char* start;
    char* pos;
    size_t ln, col;
} Json_Lexer;

Json_Token json_lex(Json_Lexer* lexer);

#endif // LEXER_H_