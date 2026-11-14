#include "release.h"
#include <limits.h>

int banker_release(Banker *banker, size_t customer, const int *resources)
{
    const Input *input = banker->input;
    size_t n = input->resource_count, offset;
    if (customer >= input->customer_count || !resources)
        return 0;
    offset = customer * n;
    /* Uma liberacao invalida nao deve produzir atualizacoes parciais. */
    for (size_t j = 0; j < n; ++j)
        if (resources[j] < 0 || resources[j] > banker->allocation[offset + j] ||
            input->available[j] > INT_MAX - resources[j] ||
            banker->need[offset + j] > INT_MAX - resources[j])
            return 0;
    for (size_t j = 0; j < n; ++j) {
        input->available[j] += resources[j];
        banker->allocation[offset + j] -= resources[j];
        banker->need[offset + j] += resources[j];
    }
    return 1;
}
