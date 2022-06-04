#ifndef LEXER_H_
#define LEXER_H_
#include <stddef.h>
#include <stdint.h>
#include "util.h"

typedef enum {
    Character_Class_End = 0,
    Character_Class_Whitespace = 0x1,
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
    Character_Class_Unknown,
} Character_Class;

typedef enum {
    Json_Token_String,
    Json_Token_Ident,
    Json_Token_Integer,
    Json_Token_Number,
    Json_Token_Boolean,
    Json_Token_Null,
    Json_Token_LBrace,
    Json_Token_RBrace,
    Json_Token_RBracket,
    Json_Token_LBracket,
    Json_Token_Comma,
    Json_Token_Colon
} Json_Token_Enum;

typedef uint32_t Json_Token_Tag;

typedef enum {
    Json_Keyword_None = 0,
    Json_Keyword_Start = 1,
    Json_Keyword_Null = 1,
    Json_Keyword_True,
    Json_Keyword_False,
    Json_Keyword_PastEnd
} Json_Keyword;


typedef struct {
    Json_Token_Tag tag;
    const char* st;
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
} Json_Lexer;

void json_lex_init();
Json_Token json_lex(Json_Lexer* lexer);
void json_report_error(Json_Lexer* lexer, const char* error_pos);

#endif // LEXER_H_
