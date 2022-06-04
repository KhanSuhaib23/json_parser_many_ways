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

Character_Class get_character_class(uint32_t rune) {
    switch (rune) {
        case 0: {
            return Character_Class_End;
        } break;
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
        case 'Z': {
            return Character_Class_Character;
        } break;
        case '_': {
            return Character_Class_Underscore;
        } break;
        case ' ': case '\n': case '\r': case '\t': {
            return Character_Class_Whitespace;
        } break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            return Character_Class_Digit;
        } break;
        case '.': {
            return Character_Class_Dot;
        } break;
        case ',': {
            return Character_Class_Comma;
        } break;
        case '\'': {
            return Character_Class_Single_Quote;
        } break;
        case '\"': {
            return Character_Class_Double_Quote;
        } break;
        case '{': {
            return Character_Class_LBrace;
        } break;
        case '}': {
            return Character_Class_RBrace;
        } break;
        case '[': {
            return Character_Class_LBracket;
        } break;
        case ']': {
            return Character_Class_RBracket;
        } break;
        case '-': {
            return Character_Class_Hyphen;
        } break;
        case ':': {
            return Character_Class_Colon;
        } break;
        default: {
            if (rune >= 0 && rune <= 127) {
                return Character_Class_Unknown;
            } else {
                return Character_Class_Character; // TODO(suhaibnk): there may be invalid utf8 chars in this get rid of them
            }
        } break;
    }
}

void consume_rune(Json_Lexer* lexer, Rune rune) {
    lexer->pos += rune.sz;
}

Json_Keyword get_keyword(String ident) {
    for (size_t i = Json_Keyword_Start; i < Json_Keyword_PastEnd; ++i) {
        if (string_equal(ident, keywords[i])) {
            return i;
        }
    }
    return Json_Keyword_None;
}

#define next_rune() (consume_rune(lexer, rune), get_rune(lexer->pos))

Json_Token json_lex(Json_Lexer* lexer) {
   Json_Token token;
   int64_t sg = 1, iv;
   size_t sz;
   double fv, ml;
   Rune rune;
   rune = get_rune(lexer->pos);

   while (get_character_class(rune.code) == Character_Class_Whitespace) {
       rune = next_rune();
   }

   token.st = lexer->pos;

   switch (get_character_class(rune.code)) {
       case Character_Class_Character:
       case Character_Class_Underscore: {
           token.tag = Json_Token_Ident;
           token.string.buff = lexer->pos;
           sz = rune.sz;
           rune = next_rune();

           while (get_character_class(rune.code) == Character_Class_Character || get_character_class(rune.code) == Character_Class_Underscore) {
               sz += rune.sz;
               rune = next_rune();
           }

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

               } break;
           }


       } break;
        case Character_Class_Single_Quote: {
            token.tag = Json_Token_String;
            rune = next_rune();
            token.string.buff = lexer->pos;
            sz = 0;

            while (get_character_class(rune.code) != Character_Class_Single_Quote) {
                sz += rune.sz;
                if (get_character_class(rune.code) == Character_Class_End) {
                    json_report_error(lexer, lexer->pos);
                    fprintf(stderr, "[ERROR]: Lexer state error didn't find closing ' before END\n");
                    exit(-1);
                } else if (rune.code == '\\') {
                    rune = next_rune();
                    sz += rune.sz;
                    if (rune.code == 0) {
                        json_report_error(lexer, lexer->pos);
                        fprintf(stderr, "[ERROR]: Lexer state error encountered \\ before END\n");
                        exit(-1);
                    }
                    rune = next_rune();
                } else {
                    rune = next_rune();
                }
            }
            next_rune(); // consume the ending single quote
            token.string.sz = sz;
        } break;
        case Character_Class_LBrace: {
            token.tag = Json_Token_LBrace;
            next_rune();
        } break;
        case Character_Class_RBrace: {
            token.tag = Json_Token_RBrace;
            next_rune();
        } break;
        case Character_Class_LBracket: {
            token.tag = Json_Token_LBracket;
            next_rune();
        } break;
        case Character_Class_RBracket: {
            token.tag = Json_Token_RBracket;
            next_rune();
        } break;
       case Character_Class_Hyphen:
           sg = -1;
           rune = next_rune();
       case Character_Class_Digit: {
           token.tag = Json_Token_Integer;

           iv = 0;

           if (get_character_class(rune.code) != Character_Class_Digit) {
               json_report_error(lexer, lexer->pos);
               fprintf(stderr, "[ERROR]: Lexer error, found - then didn't see a digit\n");
               exit(-1);
           }

           while (get_character_class(rune.code) == Character_Class_Digit) {
               iv = 10 * iv + (rune.code - '0');
               rune = next_rune();
           }
           if (get_character_class(rune.code) == Character_Class_Dot) {
               token.tag = Json_Token_Number;
               rune = next_rune();
               if (get_character_class(rune.code) != Character_Class_Digit) {
                   json_report_error(lexer, lexer->pos);
                   fprintf(stderr, "[ERROR]: Lexer error, found . then didn't see a digit\n");
                   exit(-1);
               }
               fv = (double) iv;
               ml = 0.1;
               while (get_character_class(rune.code) == Character_Class_Digit) {
                   fv += ml * (rune.code - '0');
                   ml *= 0.1;
                   rune = next_rune();
               }

               token.number = fv;
           } else {
               token.integer = iv;
           }
       } break;
       case Character_Class_Colon: {
           token.tag = Json_Token_Colon;
           next_rune();
       } break;
       case Character_Class_Comma: {
           token.tag = Json_Token_Comma;
           next_rune();
       } break;
        default: {
            json_report_error(lexer, lexer->pos);
            fprintf(stderr, "[ERROR]: Lexer state error, encountered unknown character %c\n", rune.code);
            next_rune();
        }
   }

   return token;
}

#undef next_rune
