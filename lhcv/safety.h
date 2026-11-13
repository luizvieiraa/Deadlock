#ifndef SAFETY_H
#define SAFETY_H

#include "banker.h"

/* 1: seguro; 0: inseguro; -1: falha de memoria ou estado inconsistente.
   A verificacao nao altera as matrizes nem os recursos disponiveis. */
int banker_is_safe(const Banker *banker);

#endif
