#include "fila.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct nó {
  void *dado;
  struct nó *ant;
  struct nó *prox;
} nó;

struct fila {
  nó *sentinela;
  int tam_do_dado;
  nó *proximo;
  int direcao;
};