#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>

typedef struct {
    size_t sz;
    const char* buff;
} String;

String string_new(const char* str, size_t sz) {
    return (String) {
        .sz = sz,
        .buff = str
    };
}
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

typedef struct {
    uint32_t code;
    uint32_t sz;
} Rune;

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



typedef enum {
    Json_Node_Integer,
    Json_Node_Number,
    Json_Node_String,
    Json_Node_Object,
    Json_Node_Unknown,
    Json_Node_Array,
    Json_Node_Float,
    Json_Node_Boolean,
    Json_Node_Null

} Json_Node_Enum;

typedef uint32_t Json_Node_Tag;

typedef struct Json_Node Json_Node;

typedef struct {
    size_t sz;
    size_t cap;
    String* keys;
    Json_Node* nodes;
} Json_Object;

typedef struct {
    size_t sz;
    size_t cap;
    Json_Node* nodes;
} Json_Array;

struct Json_Node {
    Json_Node_Tag tag;
    union {
        int64_t integer;
        double number;
        bool boolean;
        String string;
        Json_Object object;
        Json_Array array;
    };
};

const size_t ARRAY_MIN_CAP = 4;

Json_Object json_object_new() {
    Json_Object array;
    array.sz = 0;
    array.cap = ARRAY_MIN_CAP;
    array.keys = (String*) malloc(sizeof(String) * ARRAY_MIN_CAP);
    array.nodes = (Json_Node*) malloc(sizeof(Json_Node) * ARRAY_MIN_CAP);

    return array;
}

void json_object_put(Json_Object* object, String key, Json_Node node) {
    if (object->sz >= object->cap) {
        object->keys = (String*) realloc(object->keys, sizeof(String) * object->cap * 2);
        object->nodes = (Json_Node*) realloc(object->nodes, sizeof(Json_Node) * object->cap * 2);
        object->cap *= 2;
    }
    object->keys[object->sz] = key;
    object->nodes[object->sz] = node;
    object->sz++;
}

Json_Array json_array_new() {
    Json_Array array;
    array.sz = 0;
    array.cap = ARRAY_MIN_CAP;
    array.nodes = (Json_Node*) malloc(sizeof(Json_Node) * ARRAY_MIN_CAP);

    return array;
}

void json_array_put(Json_Array* object, Json_Node node) {
    if (object->sz >= object->cap) {
        object->nodes = (Json_Node*) realloc(object->nodes, sizeof(Json_Node) * object->cap * 2);
        object->cap *= 2;
    }
    object->nodes[object->sz] = node;
    object->sz++;
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

Json_Node json_parse_node(Json_Lexer* lexer) {
    Json_Token token = json_lex(lexer);
    Json_Node node;

    switch (token.tag) {
        case Json_Token_LBrace: {
            Json_Object json_object = json_object_new();

            while (1) {
                token = json_lex(lexer);
                if (token.tag != Json_Token_String && token.tag != Json_Token_Ident) {
                    fprintf(stderr, "[ERROR]: Parser state error. Expected a key string or identifier\n");
                    exit(-1);
                }
                String key = token.string;
                token = json_lex(lexer);

                if (token.tag != Json_Token_Colon) {
                    fprintf(stderr, "[ERROR]: Parser state error. Expected a colon\n");
                    exit(-1);
                }

                Json_Node node = json_parse_node(lexer);

                json_object_put(&json_object, key, node);
                token = json_lex(lexer);

                if (token.tag == Json_Token_RBrace) {
                    break;
                }

                if (token.tag != Json_Token_Comma) {
                    fprintf(stderr, "[ERROR]: Parser state error. Expected a commua\n");
                    exit(-1);
                }
            }
            node.tag = Json_Node_Object;
            node.object = json_object;
        } break;
        case Json_Token_LBracket: {
            Json_Array json_array = json_array_new();

            while (1) {
                Json_Node node = json_parse_node(lexer);

                json_array_put(&json_array, node);
                token = json_lex(lexer);

                if (token.tag == Json_Token_RBracket) {
                    break;
                }

                if (token.tag != Json_Token_Comma) {
                   fprintf(stderr, "[ERROR]: Parser state error. Expected a comma\n");
                   exit(-1);
                }

            }
            node.tag = Json_Node_Array;
            node.array = json_array;
        } break;

        case Json_Token_Integer: {
            node.tag = Json_Node_Integer;
            node.integer = token.integer;
        } break;
        case Json_Token_Number: {
            node.tag = Json_Node_Number;
            node.number = token.number;
        } break;
        case Json_Token_String: {
            node.tag = Json_Node_String;
            node.string = token.string;
        } break;
        default: {
            fprintf(stderr, "[ERROR]: Unknown token encountered %u at line %zd and col %zd\n'", token.tag, lexer->ln, lexer->col);
            for (char* st = lexer->start; st != lexer->pos; ++st) {
                fprintf(stderr, "%c", st[0]);
            }
            fprintf(stderr, "%c'\n", lexer->pos[0]);

            exit(-1);
        }
    }
    return node;
}

Json_Node json_parse(const char* str) {
    Json_Lexer lexer = (Json_Lexer) {
        .start = (char*) str,
        .pos = (char*) str,
        .ln = 1,
        .col = 1
    };

    return json_parse_node(&lexer);
}

void printf_tabs(uint16_t num) {
    for (; num; --num) {
        printf("    ");
    }
}

void json_print_indented(const Json_Node* node, uint16_t indentation) {
    switch (node->tag) {
        case Json_Node_Object: {
            printf("{\n");
            for (size_t i = 0; i < node->object.sz; ++i) {
                printf_tabs(indentation + 1);
                printf("'%.*s': ", (int) node->object.keys[i].sz, node->object.keys[i].buff);
                json_print_indented(&node->object.nodes[i], indentation + 1);
                printf("%s\n", i < node->object.sz - 1 ? "," : "");
            }
            printf_tabs(indentation);
            printf("}");

        } break;
        case Json_Node_Array: {
            printf("[\n");
            for (size_t i = 0; i < node->array.sz; ++i) {
                printf_tabs(indentation + 1);
                json_print_indented(&node->array.nodes[i], indentation + 1);
                printf("%s\n", i < node->array.sz - 1 ? "," : "");
            }
            printf_tabs(indentation);
            printf("]");

        } break;
        case Json_Node_Integer: {
            printf("%lld", node->integer);
        } break;
        case Json_Node_Number: {
            printf("%lf", node->number);
        } break;
        case Json_Node_String: {
            printf("'%.*s'", (int) node->string.sz, node->string.buff);
        } break;
        default: {
            printf("<UNKNOWN>");
        }
    }
}

void json_print(const Json_Node* json) {
    json_print_indented(json, 0);
}

int main(int argc, char** argv) {
    const char* str =
        "{\n"
"	'students': [{\n"
"			'id': 101,\n"
"			'name': 'ABC',\n"
"			'perc': 94.32\n"
"		},\n"
"		{\n"
"			'id': 102,\n"
"			'name': 'CSE',\n"
"			'perc': 92.99\n"
"		}],\n"
"'teacher': [{\n"
"			'id': 201,\n"
"			'name': '\xe0\xa4\xb9',\n"
/* "			'name': 'CSE',\n" */
"			'salaray': 9432\n"
"		},\n"
"		{\n"
"			'id': 202,\n"
"			'name': 'XYZ',\n"
"			'perc': 9299\n"
"		}],\n"
"'staff': [{\n"
"			id: 301,\n"
"			name: 'NII',\n"
"			perc: 94.32\n"
"		},\n"
"		{\n"
"			'id': 202,\n"
"			'name': 'XYZ',\n"
"			'perc': 92.99\n"
"		}]\n"

"}\n";

    printf("%s\n\n\n---------------------------\n\n\n", str);
    Json_Node node = json_parse(str);
    json_print(&node);

    return 0;
}
