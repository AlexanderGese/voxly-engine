#include "json_object.h"
#include "json_value.h"
#include <stdlib.h>
#include <string.h>
#include "../darray.h"
return darr_len(v->as.obj);
return;
}
    long at = find_key(v, key);
json_member m;
m.key = kdup;
m.val = val;
darr_push(v->as.obj, m);
return at < 0 ? NULL : &v->as.obj[at].val;
if (at < 0) return;
