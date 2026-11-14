#include "output.h"
#include "request.h"
#include "release.h"
#include <stdio.h>

static void vector(FILE *file, const int *values, size_t count)
{
    for (size_t j = 0; j < count; ++j)
        fprintf(file, "%s%d", j ? " " : "", values[j]);
}

static size_t vector_width(const int *values, size_t count)
{
    size_t width = count ? count - 1 : 0;
    for (size_t j = 0; j < count; ++j)
        width += (size_t)snprintf(NULL, 0, "%d", values[j]);
    return width;
}

static void column(FILE *file, const int *values, size_t count, size_t width)
{
    size_t used = vector_width(values, count);
    vector(file, values, count);
    while (used++ < width)
        fputc(' ', file);
    fputs("|   ", file);
}

static void state(FILE *file, const Banker *banker)
{
    const Input *input = banker->input;
    size_t n = input->resource_count;
    size_t maximum_width = 10, allocation_width = 12;
    for (size_t i = 0; i < input->customer_count; ++i) {
        size_t w = vector_width(input->maximum + i * n, n) + 1;
        if (w > maximum_width)
            maximum_width = w;
        w = vector_width(banker->allocation + i * n, n) + 1;
        if (w > allocation_width)
            allocation_width = w;
    }
    fputs("MAXIMUM", file);
    for (size_t j = 7; j < maximum_width; ++j)
        fputc(' ', file);
    fputs("|   ALLOCATION", file);
    for (size_t j = 10; j < allocation_width; ++j)
        fputc(' ', file);
    fputs("|   NEED\n", file);
    for (size_t i = 0; i < input->customer_count; ++i) {
        column(file, input->maximum + i * n, n, maximum_width);
        column(file, banker->allocation + i * n, n, allocation_width);
        vector(file, banker->need + i * n, n);
        fputs(" \n", file);
    }
    fputs("AVAILABLE ", file);
    vector(file, input->available, n);
    fputs(" \n", file);
}

static int request(FILE *file, Banker *banker, const Command *command)
{
    size_t n = banker->input->resource_count;
    RequestResult result = banker_request(banker, command->customer, command->resources);
    if (result == REQUEST_ERROR || result == REQUEST_INVALID) {
        fprintf(stderr, "Erro: falha ao processar RQ do cliente %zu.\n", command->customer);
        return 0;
    }
    if (result == REQUEST_GRANTED) {
        fprintf(file, "Allocate to customer %zu the resources ", command->customer);
        vector(file, command->resources, n);
        fputc('\n', file);
    } else if (result == REQUEST_UNAVAILABLE) {
        fputs("The resources ", file);
        vector(file, banker->input->available, n);
        fprintf(file, " are not enough to customer %zu request ", command->customer);
        vector(file, command->resources, n);
        fputs(" \n", file);
    } else {
        fprintf(file, "The customer %zu request ", command->customer);
        vector(file, command->resources, n);
        fputs(result == REQUEST_UNSAFE
              ? " was denied because result in an unsafe state\n"
              : " was denied because exceed its maximum allocation\n", file);
    }
    return 1;
}

int write_results(Banker *banker)
{
    const Input *input = banker->input;
    /* Modo binario mantem as quebras LF do arquivo de referencia no Windows. */
    FILE *file = fopen("result.txt", "wb");
    int valid = 1;
    if (!file) {
        fprintf(stderr, "Erro: nao foi possivel abrir result.txt para escrita.\n");
        return 0;
    }
    for (size_t i = 0; i < input->command_count && valid; ++i) {
        const Command *command = &input->commands[i];
        if (command->type == '*')
            state(file, banker);
        else if (command->type == 'Q')
            valid = request(file, banker, command);
        else if (command->type == 'L') {
            if (!banker_release(banker, command->customer, command->resources)) {
                fprintf(stderr, "Erro: liberacao invalida do cliente %zu no comando %zu.\n",
                        command->customer, i + 1);
                valid = 0;
            } else {
                fprintf(file, "Release from customer %zu the resources ", command->customer);
                vector(file, command->resources, input->resource_count);
                fputc('\n', file);
            }
        } else {
            fprintf(stderr, "Erro: comando desconhecido na posicao %zu.\n", i + 1);
            valid = 0;
        }
        if (ferror(file)) {
            fprintf(stderr, "Erro: falha de escrita em result.txt.\n");
            valid = 0;
        }
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Erro: falha ao finalizar result.txt.\n");
        valid = 0;
    }
    return valid;
}
