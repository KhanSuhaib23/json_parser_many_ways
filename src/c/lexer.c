#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "util.h"
#include "lexer.h"

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
    if (rune.code == '\n') {
        lexer->ln++;
        lexer->col = 1;
    } else {
        lexer->col++;
    }

    lexer->pos += rune.sz;
}

Json_Token json_lex(Json_Lexer* lexer) {
   #define next_rune() (consume_rune(lexer, rune), get_rune(lexer->pos))
   Json_Token token;
   int64_t sg = 1, iv;
   size_t sz;
   double fv, ml;
   Rune rune;
   rune = get_rune(lexer->pos);

   while (get_character_class(rune.code) == Character_Class_Whitespace) {
       rune = next_rune();
   }

   switch (get_character_class(rune.code)) {
       case Character_Class_Character:
       case Character_Class_Underscore: {
           token.tag = Json_Token_Ident;
           token.string.buff = lexer->pos;
           sz = 0;
           rune = next_rune();

           while (get_character_class(rune.code) == Character_Class_Character || get_character_class(rune.code) == Character_Class_Underscore) {
               sz += rune.sz;
               rune = next_rune();
           }

           token.string.sz = sz;
       } break;
        case Character_Class_Single_Quote: {
            token.tag = Json_Token_String;
            rune = next_rune();
            token.string.buff = lexer->pos;
            sz = 0;

            while (get_character_class(rune.code) != Character_Class_Single_Quote) {
                sz += rune.sz;
                if (get_character_class(rune.code) == Character_Class_End) {
                    fprintf(stderr, "[ERROR]: Lexer state error didn't find closing ' before END\n");
                    exit(-1);
                } else if (rune.code == '\\') {
                    rune = next_rune();
                    sz += rune.sz;
                    if (rune.code == 0) {
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
            fprintf(stderr, "[ERROR]: Lexer state error, encountered unknown character %c\n", rune.code);
            next_rune();
        }
   }

   return token;
}

#undef next_rune