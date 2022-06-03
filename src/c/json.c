#include "json.h"
#include <stdio.h>
#include <malloc.h>

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
