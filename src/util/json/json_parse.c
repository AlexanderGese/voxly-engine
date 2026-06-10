#include "json_parse.h"
#include "json_token.h"
#include "json_number.h"
#include "json_string.h"
#include "json_value.h"
#include "json_array.h"
#include "json_object.h"
#include <stdlib.h>
#include <string.h>
typedef struct {
    json_lexer lx;
    json_token cur;     // current (already lexed) token
    int        depth;
    json_err   err;
    json_loc  *loc;     // where to stash a failure, may be NULL
} parser;
