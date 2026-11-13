#ifndef BANKER_H
#define BANKER_H

#include "input.h"

typedef struct {
    const Input *input;
    int *allocation;
    int *need;
} Banker;

int banker_init(Banker *banker, const Input *input);
void banker_free(Banker *banker);

#endif
