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

void f_remove(Fila self, void *pdado)
{
  assert(!f_tá_vazia(self));
  nó *vítima = self->sentinela->prox;
  if (pdado != NULL) {
    memcpy(pdado, vítima->dado, self->tam_do_dado);
  }
  if (self->proximo == vítima) {
    if (self->direcao == 1) {
      self->proximo = vítima->prox;
    }
    else {
      self->proximo = vítima->ant;
    }
  }
  vítima->ant->prox = vítima->prox;
  vítima->prox->ant = vítima->ant;
  free(vítima->dado);
  free(vítima);
}

void f_inicia_percurso(Fila self, int pos_inicial)
{
  nó *n = self->sentinela;

  if (pos_inicial >= 0) {
    self->direcao = 1;
    for (int i = 0; i <= pos_inicial; i++) {
      n = n->prox;
      if (n == self->sentinela) {
        break;
      }
    }
  }
  else {
    self->direcao = -1;
    for (int i = 0; i > pos_inicial; i--) {
      n = n->ant;
      if (n == self->sentinela) {
        break;
      }
    }
  }

  self->proximo = n;
}

bool f_próximo(Fila self, void *pdado)
{
  if (self->proximo == self->sentinela) {
    return false;
  }
  if (pdado != NULL) {
    memcpy(pdado, self->proximo->dado, self->tam_do_dado);
  }
  if (self->direcao == 1) {
    self->proximo = self->proximo->prox;
  }
  else {
    self->proximo = self->proximo->ant;
  }

  return true;
}