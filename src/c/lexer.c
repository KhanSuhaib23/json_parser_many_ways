#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "util.h"
#include "lexer.h"

String keywords[Json_Keyword_PastEnd];

void json_lex_init() {
    keywords[Json_Keyword_Null] = (String) { .buff = "null", .sz = 4};
    keywords[Json_Keyword_False] = (String) { .buff = "false", .sz = 5};
    keywords[Json_Keyword_True] = (String) { .buff = "true", .sz = 4};
}

#define ERROR_LINES 10

void json_report_error(Json_Lexer* lexer, const char* error_pos) {
    const char* last_n_line[ERROR_LINES] = {0};
    size_t error_line_index = 0, i, l;
    size_t current_line = 1;
    const char* pos = lexer->start;

    last_n_line[error_line_index] = pos;
    error_line_index = (error_line_index + 1) % 10;

    for (; pos[0] && pos != error_pos; ++pos) {
        if (pos[0] == '\n') {
            last_n_line[error_line_index] = pos + 1;
            error_line_index = (error_line_index + 1) % 10;

            current_line++;
        }
    }

    l = current_line > 10 ? current_line - 10 + 1 : 1;

    for (i = 0; i < ERROR_LINES; ++i, error_line_index = (error_line_index + 1) % 10) {
        if (last_n_line[error_line_index]) {
            fprintf(stderr, "%4zd  | ", l);
            pos = last_n_line[error_line_index];
            for (; pos[0] && pos[0] != '\n'; ++pos) {
                fprintf(stderr, "%c", pos[0]);
            }
            fprintf(stderr, "\n");
            l++;
        }
    }

    fprintf(stderr, "        ");
    pos = last_n_line[error_line_index == 0 ? ERROR_LINES - 1 : error_line_index - 1];

    for (; pos[0] && pos[0] != '\n' && pos != error_pos; ++pos) {
        if (pos[0] == '\t') {
            fprintf(stderr, "--------");
        } else {
            fprintf(stderr, "-");
        }
    }
    fprintf(stderr, "^");

    for (; pos[0] && pos[0] != '\n'; ++pos) {
        fprintf(stderr, "-");
    }
    fprintf(stderr, "\n");
}

Character_Class get_character_class(uint8_t ch) {
    switch (ch) {
        case 0:
            return Character_Class_End;

        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y':
        case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y':
        case 'Z':
            return Character_Class_Character;

        case ' ': case '\n': case '\r': case '\t':
            return Character_Class_Whitespace;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return Character_Class_Digit;

        case '.': case ',': case '\"': case '{': case '}':
        case '[': case ']': case '-': case ':': case '\\':
            return (Character_Class) ch;

        default: {
            if (ch <= 127) {
                return Character_Class_Unknown;
            } else {
                return Character_Class_Character;
            }
        } break;
    }
}

Json_Keyword get_keyword(String ident) {
    for (size_t i = Json_Keyword_Start; i < Json_Keyword_PastEnd; ++i) {
        if (string_equal(ident, keywords[i])) {
            return i;
        }
    }
    return Json_Keyword_None;
}

Json_Token json_lex(Json_Lexer* lexer) {
   Json_Token token;
   int64_t sg = 1, iv;
   uint8_t* b = (uint8_t*) lexer->pos;
   token.st = lexer->pos;
   size_t sz = 0;
   double fv, ml;

   while (get_character_class(b[0]) == Character_Class_Whitespace) {
       ++b;
   }


   switch (get_character_class(b[0])) {
       case Character_Class_Character: {
           token.tag = Json_Token_String;
           token.string.buff = (const char*) b;

           for (; get_character_class(b[0]) == Character_Class_Character; ++b, ++sz);

           token.string.sz = sz;

           switch (get_keyword(token.string)) {
               case Json_Keyword_Null: {
                   token.tag = Json_Token_Null;
               } break;
               case Json_Keyword_False: {
                   token.tag = Json_Token_Boolean;
                   token.boolean = false;
               } break;
               case Json_Keyword_True: {
                   token.tag = Json_Token_Boolean;
                   token.boolean = true;
               } break;
               default: {
                   json_report_error(lexer, token.st);
                   fprintf(stderr, "[ERROR]: Lexer error found unknown identifier\n");
                   exit(-1);
               } break;
           }

       } break;
        case Character_Class_Double_Quote: {
            token.tag = Json_Token_String;
            ++b;
            token.string.buff = (const char*) b;
            sz = 0;

            for (; get_character_class(b[0]) != Character_Class_Double_Quote; ) {
                if (get_character_class(b[0]) == Character_Class_End) {
                    json_report_error(lexer, lexer->pos);
                    fprintf(stderr, "[ERROR]: Lexer state error didn't find closing ' before END\n");
                    exit(-1);
                } else if (b[0] == '\\') {
                    ++b;
                    ++sz;
                    if (b[0] == 0) {
                        json_report_error(lexer, lexer->pos);
                        fprintf(stderr, "[ERROR]: Lexer state error encountered \\ before END\n");
                        exit(-1);
                    }
                }
                ++b;
                ++sz;
            }
            ++b;
            token.string.sz = sz;
        } break;
       case Character_Class_LBrace:
       case Character_Class_RBrace:
       case Character_Class_LBracket:
       case Character_Class_RBracket:
       case Character_Class_Colon:
       case Character_Class_Comma:
       {
           token.tag = (Json_Token_Tag) get_character_class(b[0]);
           ++b;
       } break;
       case Character_Class_Hyphen:
           sg = -1;
           ++b;
       case Character_Class_Digit: {
           token.tag = Json_Token_Integer;

           iv = 0;

           if (get_character_class(b[0]) != Character_Class_Digit) {
               json_report_error(lexer, lexer->pos);
               fprintf(stderr, "[ERROR]: Lexer error, found - then didn't see a digit\n");
               exit(-1);
           }

           while (get_character_class(b[0]) == Character_Class_Digit) {
               iv = 10 * iv + (b[0] - '0');
               ++b;
           }
           if (get_character_class(b[0]) == Character_Class_Dot) {
               token.tag = Json_Token_Number;
               ++b;
               if (get_character_class(b[0]) != Character_Class_Digit) {
                   json_report_error(lexer, lexer->pos);
                   fprintf(stderr, "[ERROR]: Lexer error, found . then didn't see a digit\n");
                   exit(-1);
               }
               fv = (double) iv;
               ml = 0.1;
               while (get_character_class(b[0]) == Character_Class_Digit) {
                   fv += ml * (b[0] - '0');
                   ml *= 0.1;
                   ++b;
               }
               token.number = sg * fv;
           } else {
               token.integer = sg * iv;
           }
       } break;
        default: {
            json_report_error(lexer, lexer->pos);
            fprintf(stderr, "[ERROR]: Lexer state error, encountered unknown character %c\n", b[0]);
            ++b;
        }
   }

   lexer->pos = (char*) b;

   return token;
}
