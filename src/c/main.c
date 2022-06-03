#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>
#include "lexer.h"
#include "json.h"
#include "parser.h"

int main(int argc, char** argv) {
    const char* str =
        "{\n"
"	'students': [{\n"
"			'id': 101,\n"
"			'name': 'ABC',\n"
"			'perc': 94.32,\n"
"           is_cat: false,\n"
"           cat: null\n"
"		},\n"
"		{\n"
"			'id': 102,\n"
"			'name': 'CSE',\n"
"			'perc': 92.99,\n"
"           is_cat: true,\n"
"           cat: { kitty_name: 'Cat', is_cute: true}\n"
"		}],\n"
"'teacher': [{\n"
"			'id': 201,\n"
"			'name': '\xe0\xa4\xb9',\n"
/* "			'name': 'CSE',\n" */
"			'salary': 9432\n"
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

    json_lex_init();
    printf("%s\n\n\n---------------------------\n\n\n", str);
    Json_Node node = json_parse(str);
    json_print(&node);

    return 0;
}
