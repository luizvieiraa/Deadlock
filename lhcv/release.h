#ifndef RELEASE_H
#define RELEASE_H

#include "banker.h"

/* Retorna 1 ao liberar e 0 para uma liberacao invalida, sem alterar o estado. */
int banker_release(Banker *banker, size_t customer, const int *resources);

#endif
