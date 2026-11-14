#ifndef REQUEST_H
#define REQUEST_H

#include "banker.h"

typedef enum {
    REQUEST_GRANTED,
    REQUEST_EXCEEDS_NEED,
    REQUEST_UNAVAILABLE,
    REQUEST_UNSAFE,
    REQUEST_INVALID,
    REQUEST_ERROR
} RequestResult;

RequestResult banker_request(Banker *banker, size_t customer, const int *resources);

#endif
