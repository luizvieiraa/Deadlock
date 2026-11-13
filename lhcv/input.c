#include "input.h"
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int error(const char *file, size_t line, const char *message)
{
    if (file)
        fprintf(stderr, "Erro em %s, linha %zu: %s\n", file, line, message);
    else
        fprintf(stderr, "Erro: %s\n", message);
    return 0;
}

static void spaces(const char **cursor)
{
    while (isspace((unsigned char)**cursor))
        ++*cursor;
}

/* Aceita somente inteiros decimais nao negativos representaveis por int. */
static int number(const char **cursor, int *value)
{
    const char *p = *cursor;
    int result = 0;
    if (*p < '0' || *p > '9')
        return 0;
    while (*p >= '0' && *p <= '9') {
        int digit = *p++ - '0';
        if (result > (INT_MAX - digit) / 10)
            return 0;
        result = result * 10 + digit;
    }
    *cursor = p;
    *value = result;
    return 1;
}

static void *grow(void *old, size_t count, size_t size)
{
    if (size && count > SIZE_MAX / size)
        return NULL;
    return realloc(old, count * size);
}

/* Le linhas sem limite fixo; distingue EOF de falhas de leitura/memoria. */
static int line_read(FILE *file, char **line)
{
    size_t length = 0, capacity = 128;
    char *buffer = malloc(capacity);
    int ch;
    if (!buffer)
        return -1;
    while ((ch = fgetc(file)) != EOF && ch != '\n') {
        if (ch == '\0') {
            free(buffer);
            return -1;
        }
        if (length == capacity - 1) {
            char *next;
            if (capacity > SIZE_MAX / 2) {
                free(buffer);
                return -1;
            }
            capacity *= 2;
            next = realloc(buffer, capacity);
            if (!next) {
                free(buffer);
                return -1;
            }
            buffer = next;
        }
        buffer[length++] = (char)ch;
    }
    if (ferror(file)) {
        free(buffer);
        return -1;
    }
    if (!length && ch == EOF) {
        free(buffer);
        return 0;
    }
    buffer[length] = '\0';
    *line = buffer;
    return 1;
}

static int customer_line(const char *p, Input *input)
{
    size_t n = input->resource_count;
    size_t rows = input->customer_count;
    int *next;
    if (rows == SIZE_MAX || rows + 1 > SIZE_MAX / n)
        return 0;
    next = grow(input->maximum, (rows + 1) * n, sizeof(int));
    if (!next)
        return 0;
    input->maximum = next;
    for (size_t j = 0; j < n; ++j) {
        spaces(&p);
        if (!number(&p, &next[rows * n + j]))
            return 0;
        spaces(&p);
        if (j + 1 < n && *p++ != ',')
            return 0;
    }
    if (*p)
        return 0;
    ++input->customer_count;
    return 1;
}

static int command_line(const char *p, Input *input)
{
    Command command = {0};
    Command *next;
    int customer;
    spaces(&p);
    if (*p == '*') {
        command.type = *p++;
    } else {
        if (strncmp(p, "RQ", 2) == 0)
            command.type = 'Q';
        else if (strncmp(p, "RL", 2) == 0)
            command.type = 'L';
        else
            return 0;
        p += 2;
        if (!isspace((unsigned char)*p))
            return 0;
        spaces(&p);
        if (!number(&p, &customer) || (size_t)customer >= input->customer_count)
            return 0;
        command.customer = (size_t)customer;
        command.resources = grow(NULL, input->resource_count, sizeof(int));
        if (!command.resources)
            return 0;
        for (size_t j = 0; j < input->resource_count; ++j) {
            if (!isspace((unsigned char)*p))
                goto invalid;
            spaces(&p);
            if (!number(&p, &command.resources[j]))
                goto invalid;
        }
    }
    spaces(&p);
    if (*p || input->command_count == SIZE_MAX)
        goto invalid;
    next = grow(input->commands, input->command_count + 1, sizeof(Command));
    if (!next)
        goto invalid;
    input->commands = next;
    next[input->command_count++] = command;
    return 1;
invalid:
    free(command.resources);
    return 0;
}

static int read_file(const char *name, Input *input, int customers)
{
    FILE *file = fopen(name, "r");
    size_t line_number = 0;
    int status, valid = 1;
    char *line = NULL;
    if (!file)
        return error(name, 0, "nao foi possivel abrir o arquivo.");
    while ((status = line_read(file, &line)) == 1) {
        ++line_number;
        valid = customers ? customer_line(line, input) : command_line(line, input);
        free(line);
        line = NULL;
        if (!valid) {
            error(name, line_number, customers
                  ? "linha invalida: esperado um inteiro nao negativo por recurso, separado por virgulas."
                  : "comando invalido: use RQ/RL, cliente existente e um inteiro nao negativo por recurso, ou apenas *.");
            break;
        }
    }
    if (valid && status < 0)
        valid = error(name, line_number + 1, "falha de leitura, byte nulo ou memoria insuficiente.");
    if (fclose(file) != 0 && valid)
        valid = error(name, line_number, "falha ao fechar o arquivo.");
    if (valid && !line_number)
        valid = error(name, 0, "arquivo vazio.");
    return valid;
}

int read_input(int argc, char **argv, Input *input)
{
    if (argc < 2)
        return error(NULL, 0, "uso: ./banker recurso1 [recurso2 ...]");
    input->resource_count = (size_t)(argc - 1);
    input->available = grow(NULL, input->resource_count, sizeof(int));
    if (!input->available)
        return error(NULL, 0, "memoria insuficiente.");
    for (int i = 1; i < argc; ++i) {
        const char *p = argv[i];
        if (!number(&p, &input->available[i - 1]) || *p)
            return error(NULL, 0, "cada argumento deve ser um inteiro nao negativo entre 0 e INT_MAX.");
    }
    return read_file("customer.txt", input, 1) && read_file("commands.txt", input, 0);
}

void free_input(Input *input)
{
    for (size_t i = 0; i < input->command_count; ++i)
        free(input->commands[i].resources);
    free(input->commands);
    free(input->maximum);
    free(input->available);
    memset(input, 0, sizeof(*input));
}
