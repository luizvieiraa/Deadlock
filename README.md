# Deadlock — Implementação 6

Implementação do algoritmo de Banker em C para a disciplina Infraestrutura de Software, da CESAR School.

**Aluno:** Luiz Henrique Carvalho Vieira — `lhcv@cesar.school`  
**Professor:** Marcelo Lima Arcoverde

O programa lê os recursos disponíveis pela linha de comando, os limites dos clientes em `customer.txt` e os comandos em `commands.txt`. As solicitações são aceitas somente quando respeitam os limites e mantêm o sistema em estado seguro. O resultado é gravado em `result.txt`.

## Requisitos e compilação

Use um terminal Linux, Unix ou macOS com compilador C compatível com C11 e GNU Make. No Windows, utilize o Ubuntu/WSL. A versão modular anterior foi confirmada no Ubuntu/WSL. A versão atual, em um único fonte, foi revalidada no Windows/GCC; o aluno também confirmou sua execução no WSL com retorno 0 e a saída esperada.

Na raiz do repositório:

```bash
cd lhcv
make
```

O comando gera o executável `banker`. O Makefile utiliza `gcc` com `-std=c11 -Wall -Wextra -Wpedantic -O2`. Para selecionar outro compilador compatível:

```bash
make CC=clang
```

## Arquivos de entrada

Antes de executar, coloque os anexos `customer.txt` e `commands.txt` fornecidos pelo professor dentro de `lhcv`, junto ao executável. Esses arquivos não são versionados; devem ser obtidos no Classroom após clonar o repositório.

### customer.txt

Cada linha representa um cliente, identificado a partir de zero, e contém seu limite máximo por tipo de recurso. Os valores são inteiros não negativos separados por vírgulas:

```text
7,5,3
3,2,2
9,0,2
2,2,2
4,3,3
```

A quantidade de linhas determina o número de clientes. A quantidade de valores por linha deve corresponder ao número de recursos informado na execução.

### commands.txt

Os comandos são processados na ordem do arquivo:

| Comando | Formato | Operação |
| --- | --- | --- |
| `RQ` | `RQ cliente r1 r2 ...` | Solicitar recursos para um cliente. |
| `RL` | `RL cliente r1 r2 ...` | Liberar recursos já alocados a um cliente. |
| `*` | `*` | Exibir `MAXIMUM`, `ALLOCATION`, `NEED` e `AVAILABLE` na saída. |

Exemplo de formato:

```text
RQ 0 0 2 1
RL 0 0 1 1
*
```

Em `RQ` e `RL`, deve existir exatamente um valor por tipo de recurso, além do identificador do cliente. Não há entrada interativa.

## Execução

Execute dentro de `lhcv`, onde estão os dois arquivos de entrada:

```bash
./banker 10 5 7
echo $?
cat result.txt
```

Cada argumento representa a quantidade inicial de instâncias de um tipo de recurso. No exemplo, existem três tipos, com 10, 5 e 7 instâncias. As quantidades de clientes e recursos são dinâmicas.

Inicialmente, `ALLOCATION` é zero e `NEED` é igual a `MAXIMUM`. Uma solicitação `RQ` é recusada se exceder a necessidade restante do cliente, não houver recursos disponíveis ou a alocação levar a um estado inseguro. A simulação de uma solicitação recusada é desfeita. A liberação `RL` atualiza a alocação, a necessidade e os recursos disponíveis.

O programa cria ou sobrescreve `result.txt`, registrando as alocações, liberações, recusas e consultas. As mensagens e a formatação do exemplo seguem o arquivo `result.txt` anexado pelo professor. Recusas normais de `RQ` são registradas na saída e o processamento continua.

O código de saída é `0` quando o processamento termina com sucesso e `1` quando ocorre erro. Mensagens de erro são escritas em `stderr`.

## Tratamento de erros

O programa valida os argumentos e os arquivos de entrada antes de executar os comandos. São rejeitados arquivos ausentes ou vazios, formatos inválidos, números negativos ou acima de `INT_MAX`, identificadores de clientes inexistentes e quantidades de recursos incompatíveis. Uma liberação superior à alocação do cliente também encerra a execução com erro. Falhas de alocação de memória e de abertura ou escrita da saída são tratadas.

Se um erro ocorrer durante o processamento ou a escrita, `result.txt` pode conter apenas os resultados produzidos até esse ponto; confira sempre o código de saída.

## Organização do código

A implementação está integralmente em `lhcv/banker.c`, o único arquivo `.c` do projeto. Não há cabeçalhos próprios. O arquivo reúne os tipos, a leitura e validação, as matrizes dinâmicas, a verificação de segurança, RQ/RL, a saída e `main`.

| Arquivo | Responsabilidade |
| --- | --- |
| `lhcv/banker.c` | Implementação completa do programa. |
| `lhcv/Makefile` | Compilar o único fonte e limpar os compilados. |
| `lhcv/evidencias.log` | Registrar os testes realizados. |

## Validação

Após a consolidação, `lhcv/evidencias.log` registra 72 casos aprovados no Windows/GCC: 52 casos de regressão e 20 sequências de 30 operações comparadas com um modelo independente em Python. Incluem comparação byte a byte com a referência, entradas malformadas, arquivos ausentes, dimensões dinâmicas, zero e valores extremos. O log preserva a execução anterior no Ubuntu/WSL, correspondente à versão modular. Após a consolidação, o aluno confirmou nova execução no WSL com retorno 0 e a mesma saída correta, sem fornecer uma nova transcrição integral da compilação.

Falhas de memória e permissões de leitura não foram injetadas nos testes. `make clean` foi executado antes da compilação no WSL; a ausência do executável imediatamente após a limpeza não foi verificada separadamente.

## Limpeza

Dentro de `lhcv`:

```bash
make clean
```

O comando remove `banker`, `banker.exe` e os arquivos `*.o`. Os arquivos de entrada, `result.txt` e as evidências são preservados.

## Arquivos versionados e entrega

O GitHub deve conter código, Makefile, README, `evidencias.log` e o relatório em PDF. O `.gitignore` exclui as entradas do professor, a saída gerada, os executáveis e os arquivos objeto.

Para o Classroom, o enunciado exige `lhcv.pdf` e `lhcv.tar`. O arquivo TAR deve conter a pasta `lhcv` com apenas `banker.c` e o `Makefile`. A preparação do relatório e do pacote de entrega corresponde às etapas seguintes.
