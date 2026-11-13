#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

typedef struct {
    char type; /* Q: RQ, L: RL, *: consulta. */
    size_t customer;
    int *resources;
} Command;

typedef struct {
    size_t resource_count;
    size_t customer_count;
    int *available;
    int *maximum; /* Matriz linear: cliente * resource_count + recurso. */
    size_t command_count;
    Command *commands;
} Input;

int read_input(int argc, char **argv, Input *input);
void free_input(Input *input);

#endif
