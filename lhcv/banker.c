/* Implementacao 6 - Deadlock: algoritmo de Banker. */

#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct {
    const Input *input;
    int *allocation;
    int *need;
} Banker;

int banker_init(Banker *banker, const Input *input);
void banker_free(Banker *banker);

/* 1: seguro; 0: inseguro; -1: falha de memoria ou estado inconsistente.
   A verificacao nao altera as matrizes nem os recursos disponiveis. */
int banker_is_safe(const Banker *banker);

typedef enum {
    REQUEST_GRANTED,
    REQUEST_EXCEEDS_NEED,
    REQUEST_UNAVAILABLE,
    REQUEST_UNSAFE,
    REQUEST_INVALID,
    REQUEST_ERROR
} RequestResult;

RequestResult banker_request(Banker *banker, size_t customer, const int *resources);

/* Retorna 1 ao liberar e 0 para uma liberacao invalida, sem alterar o estado. */
int banker_release(Banker *banker, size_t customer, const int *resources);

int write_results(Banker *banker);

/* Leitura e validacao */
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

/* Inicializacao das matrizes */
int banker_init(Banker *banker, const Input *input)
{
    size_t cells;
    *banker = (Banker){0};
    if (!input->customer_count || !input->resource_count ||
        input->customer_count > SIZE_MAX / input->resource_count)
        return 0;
    cells = input->customer_count * input->resource_count;
    if (cells > SIZE_MAX / sizeof(int))
        return 0;
    banker->allocation = calloc(cells, sizeof(int));
    banker->need = malloc(cells * sizeof(int));
    if (!banker->allocation || !banker->need) {
        banker_free(banker);
        return 0;
    }
    memcpy(banker->need, input->maximum, cells * sizeof(int));
    banker->input = input;
    return 1;
}

void banker_free(Banker *banker)
{
    free(banker->allocation);
    free(banker->need);
    *banker = (Banker){0};
}

/* Verificacao de seguranca */
int banker_is_safe(const Banker *banker)
{
    const Input *input = banker->input;
    size_t n = input->resource_count, m = input->customer_count;
    size_t completed = 0;
    int result = 0;
    int *work;
    unsigned char *finish;
    if (n > SIZE_MAX / sizeof(int))
        return -1;
    work = malloc(n * sizeof(int));
    finish = calloc(m, sizeof(*finish));
    if (!work || !finish) {
        free(work);
        free(finish);
        return -1;
    }
    memcpy(work, input->available, n * sizeof(int));
    while (completed < m) {
        int progress = 0;
        for (size_t i = 0; i < m; ++i) {
            size_t j;
            if (finish[i])
                continue;
            for (j = 0; j < n; ++j)
                if (banker->need[i * n + j] > work[j])
                    break;
            if (j != n)
                continue;
            /* Simula a conclusao do cliente e a devolucao dos recursos. */
            for (j = 0; j < n; ++j) {
                int allocated = banker->allocation[i * n + j];
                if (allocated < 0 || work[j] < 0 || work[j] > INT_MAX - allocated) {
                    result = -1;
                    goto done;
                }
                work[j] += allocated;
            }
            finish[i] = 1;
            ++completed;
            progress = 1;
        }
        if (!progress)
            break;
    }
    result = completed == m;
done:
    free(work);
    free(finish);
    return result;
}

/* Solicitacao de recursos */
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

/* Liberacao de recursos */
int banker_release(Banker *banker, size_t customer, const int *resources)
{
    const Input *input = banker->input;
    size_t n = input->resource_count, offset;
    if (customer >= input->customer_count || !resources)
        return 0;
    offset = customer * n;
    /* Uma liberacao invalida nao deve produzir atualizacoes parciais. */
    for (size_t j = 0; j < n; ++j)
        if (resources[j] < 0 || resources[j] > banker->allocation[offset + j] ||
            input->available[j] > INT_MAX - resources[j] ||
            banker->need[offset + j] > INT_MAX - resources[j])
            return 0;
    for (size_t j = 0; j < n; ++j) {
        input->available[j] += resources[j];
        banker->allocation[offset + j] -= resources[j];
        banker->need[offset + j] += resources[j];
    }
    return 1;
}

/* Processamento e arquivo de resultados */
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

/* Programa principal */
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
