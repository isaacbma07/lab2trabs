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

Fila f_cria(int tam_do_dado)
{
  Fila self = malloc(sizeof(*self));
  assert(self != NULL);
  nó *sent = malloc(sizeof(nó));
  assert(sent != NULL);
  sent->prox = sent;
  sent->ant = sent;
  sent->dado = NULL;
  self->sentinela = sent;
  self->tam_do_dado = tam_do_dado;
  self->proximo = sent;
  self->direcao = 1;

  return self;
}

bool f_tá_vazia(Fila self)
{
  return self->sentinela->prox == self->sentinela;
}

void f_destrói(Fila self)
{
  nó *atual = self->sentinela->prox;
  while (atual != self->sentinela) {
    nó *prox = atual->prox;
    free(atual->dado);
    free(atual);
    atual = prox;
  }
  free(self->sentinela);
  free(self);
}

void f_insere(Fila self, void *pdado)
{
  nó *novo = malloc(sizeof(nó));
  assert(novo != NULL);
  novo->dado = malloc(self->tam_do_dado);
  assert(novo->dado != NULL);
  memcpy(novo->dado, pdado, self->tam_do_dado);
  nó *x = self->sentinela;
  novo->prox = x;
  novo->ant = x->ant;
  x->ant->prox = novo;
  x->ant = novo;
}