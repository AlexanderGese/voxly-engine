#include "debugdraw.h"
#include "debugdraw_timed.h"
#include "../../util/darray.h"
#include "../../util/log.h"
#include "../../math/mat4.h"
#include <string.h>
i < DD_BUCKET_COUNT;
i++)
        darr_free(dd->buckets[i]);
darr_free(dd->scratch);
darr_free(dd->timed);
ddgl_destroy(&dd->gl);
ddbucket bk = line_bucket(dd);
darr_push(dd->buckets[bk], ddvert_make(a, ca));
darr_push(dd->buckets[bk], ddvert_make(b, cb));
;
