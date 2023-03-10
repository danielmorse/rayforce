#include <string.h>
#include "string.h"
#include "storm.h"
#include "alloc.h"

string_t string_create(str_t str, u64_t len)
{
    string_t string = {
        .len = len,
        .str = str,
    };

    return string;
}

str_t string_clone(string_t string)
{
    str_t res = (str_t)storm_malloc(string.len + 1);
    strncpy(res, string.str, string.len + 1);
    return res;
}
