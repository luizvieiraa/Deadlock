#include "input.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
    Input input = {0};
    int valid = read_input(argc, argv, &input);
    /* As proximas etapas processarao os comandos ja validados. */
    free_input(&input);
    return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
