#include "request.h"
#include "safety.h"

RequestResult banker_request(Banker *banker, size_t customer, const int *resources)
{
    const Input *input = banker->input;
    size_t n = input->resource_count;
    size_t offset;
    int safe;
    if (customer >= input->customer_count || !resources)
        return REQUEST_INVALID;
    offset = customer * n;
    /* Valida o vetor inteiro antes de modificar qualquer recurso. */
    for (size_t j = 0; j < n; ++j)
        if (resources[j] < 0)
            return REQUEST_INVALID;
    for (size_t j = 0; j < n; ++j)
        if (resources[j] > banker->need[offset + j])
            return REQUEST_EXCEEDS_NEED;
    for (size_t j = 0; j < n; ++j)
        if (resources[j] > input->available[j])
            return REQUEST_UNAVAILABLE;
    for (size_t j = 0; j < n; ++j) {
        input->available[j] -= resources[j];
        banker->allocation[offset + j] += resources[j];
        banker->need[offset + j] -= resources[j];
    }
    safe = banker_is_safe(banker);
    if (safe == 1)
        return REQUEST_GRANTED;
    /* Desfaz integralmente a simulacao, inclusive em falhas de memoria. */
    for (size_t j = 0; j < n; ++j) {
        input->available[j] += resources[j];
        banker->allocation[offset + j] -= resources[j];
        banker->need[offset + j] += resources[j];
    }
    return safe == 0 ? REQUEST_UNSAFE : REQUEST_ERROR;
}
