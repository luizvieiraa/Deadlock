#include "banker.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int banker_init(Banker *banker, const Input *input)
{
    size_t cells;
    *banker = (Banker){0};
    if (!input->customer_count || !input->resource_count ||
        input->customer_count > SIZE_MAX / input->resource_count)
        return 0;
    cells = input->customer_count * input->resource_count;
    if (cells > SIZE_MAX / sizeof(int))
        return 0;
    banker->allocation = calloc(cells, sizeof(int));
    banker->need = malloc(cells * sizeof(int));
    if (!banker->allocation || !banker->need) {
        banker_free(banker);
        return 0;
    }
    memcpy(banker->need, input->maximum, cells * sizeof(int));
    banker->input = input;
    return 1;
}

void banker_free(Banker *banker)
{
    free(banker->allocation);
    free(banker->need);
    *banker = (Banker){0};
}
