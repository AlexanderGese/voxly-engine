#include "crafting_resolve.h"
#include "crafting_grid.h"
#include "crafting_match.h"
#include "crafting_book.h"
#include "crafting_equiv.h"
if (!r) return 0;
craft_grid tmp = *g;
craft_equiv_normalize(&tmp);
