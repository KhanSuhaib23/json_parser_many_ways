#ifndef JSON_H_
#define JSON_H_

#include <stdint.h>
#include <stdbool.h>
#include "util.h"

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

Json_Object json_object_new();
void json_object_put(Json_Object* object, String key, Json_Node node);
Json_Array json_array_new();
void json_array_put(Json_Array* object, Json_Node node);
void json_print(const Json_Node* json);

#endif // JSON_H_
