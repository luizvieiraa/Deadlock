#include "safety.h"
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int banker_is_safe(const Banker *banker)
{
    const Input *input = banker->input;
    size_t n = input->resource_count, m = input->customer_count;
    size_t completed = 0;
    int result = 0;
    int *work;
    unsigned char *finish;
    if (n > SIZE_MAX / sizeof(int))
        return -1;
    work = malloc(n * sizeof(int));
    finish = calloc(m, sizeof(*finish));
    if (!work || !finish) {
        free(work);
        free(finish);
        return -1;
    }
    memcpy(work, input->available, n * sizeof(int));
    while (completed < m) {
        int progress = 0;
        for (size_t i = 0; i < m; ++i) {
            size_t j;
            if (finish[i])
                continue;
            for (j = 0; j < n; ++j)
                if (banker->need[i * n + j] > work[j])
                    break;
            if (j != n)
                continue;
            /* Simula a conclusao do cliente e a devolucao dos recursos. */
            for (j = 0; j < n; ++j) {
                int allocated = banker->allocation[i * n + j];
                if (allocated < 0 || work[j] < 0 || work[j] > INT_MAX - allocated) {
                    result = -1;
                    goto done;
                }
                work[j] += allocated;
            }
            finish[i] = 1;
            ++completed;
            progress = 1;
        }
        if (!progress)
            break;
    }
    result = completed == m;
done:
    free(work);
    free(finish);
    return result;
}
