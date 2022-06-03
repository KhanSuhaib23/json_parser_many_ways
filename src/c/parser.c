#include "parser.h"
#include <stdio.h>

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
