#include "lista.h"

#include <assert.h>
#include <stdlib.h>

typedef struct nó {
    dado_t dado;
    struct nó *ant;
    struct nó *prox;
} nó;

struct lista {
    nó sentinla;
    int tam;
};

// funções auxiliares

