#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "../core/storm.h"
#include "../core/format.h"

int main()
{
    int run = 1;
    while (run)
    {
        printf(">");

        char *line = NULL;
        size_t len = 0;
        size_t l = getline(&line, &len, stdin);
        UNUSED(l);

        i64 *vec = (i64 *)malloc(12 * sizeof(i64));
        for (i64 i = 0; i < 12; i++)
        {
            vec[i] = i;
        }
        g0 value = new_vector_i64(vec, 12);
        // g0 value = new_scalar_i64(9223372036854775807);
        str buffer;
        Result res = g0_fmt(&buffer, value);
        if (res == Ok)
        {
            printf("%s\n", buffer);
        }
        result_fmt(&buffer, res);
        printf("Result: %s\n", buffer);

        free(line);
    }

    return 0;
}
