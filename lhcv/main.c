#include "input.h"
#include "banker.h"
#include "output.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    Input input = {0};
    Banker banker = {0};
    int valid = read_input(argc, argv, &input);
    if (valid && !banker_init(&banker, &input)) {
        fprintf(stderr, "Erro: nao foi possivel inicializar as matrizes do banker.\n");
        valid = 0;
    }
    if (valid)
        valid = write_results(&banker);
    banker_free(&banker);
    free_input(&input);
    return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
