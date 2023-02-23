#include "storm.h"
#include <stdlib.h>
#include "format.h"
#include <stdio.h>

extern g0 new_scalar_i64(i64 value)
{
    g0 scalar = malloc(sizeof(struct s0));
    scalar->type = -TYPE_I64;
    scalar->i64_value = value;
    return scalar;
}

extern g0 new_vector_i64(i64 *ptr, i64 len)
{
    g0 vector = malloc(sizeof(struct s0));
    vector->type = TYPE_I64;
    vector->list_value.ptr = ptr;
    vector->list_value.len = len;
    return vector;
}
