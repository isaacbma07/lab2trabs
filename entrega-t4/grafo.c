#include "grafo.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct grafo {
  int tam_nó;
  int tam_aresta;
  int n;
  void **nós;
  void **arestas;
};

Grafo g_cria(int tam_nó, int tam_aresta)
{
  Grafo self = malloc(sizeof(*self));
  assert(self != NULL);

  self->tam_nó = tam_nó;
  self->tam_aresta = tam_aresta;
  self->n = 0;
  self->nós = NULL;
  self->arestas = NULL;

  return self;
}

int g_num_nós(Grafo self)
{
  return self->n;
}

static void aumenta_matriz(Grafo self)
{
  int n = self->n;
  void **nova = calloc((n + 1) * (n + 1), sizeof(void *));
  assert(nova != NULL);

  for (int o = 0; o < n; o++) {
    for (int d = 0; d < n; d++) {
      nova[o * (n + 1) + d] = self->arestas[o * n + d];
    }
  }

  free(self->arestas);
  self->arestas = nova;
}

int g_insere_nó(Grafo self, void *p_dado)
{
  int n = self->n;
  aumenta_matriz(self);

  self->nós = realloc(self->nós, (n + 1) * sizeof(void *));
  assert(self->nós != NULL);

  void *dado = malloc(self->tam_nó);
  assert(dado != NULL);

  memcpy(dado, p_dado, self->tam_nó);

  self->nós[n] = dado;
  self->n = n + 1;

  return n;
}

void *g_valor_do_nó(Grafo self, int nó)
{
  assert(nó >= 0 && nó < self->n);
  return self->nós[nó];
}

void g_altera_valor_do_nó(Grafo self, int nó, void *p_dado)
{
  assert(nó >= 0 && nó < self->n);
  memcpy(self->nós[nó], p_dado, self->tam_nó);
}

void *g_valor_da_aresta(Grafo self, int nó_origem, int nó_destino)
{
  assert(nó_origem >= 0 && nó_origem < self->n);
  assert(nó_destino >= 0 && nó_destino < self->n);
  return self->arestas[nó_origem * self->n + nó_destino];
}

void g_altera_valor_da_aresta(Grafo self, int nó_origem, int nó_destino,
                              void *p_dado)
{
  assert(nó_origem >= 0 && nó_origem < self->n);
  assert(nó_destino >= 0 && nó_destino < self->n);

  int pos = nó_origem * self->n + nó_destino;

  if (p_dado == NULL) {
    free(self->arestas[pos]);
    self->arestas[pos] = NULL;
  }
  else {
    if (self->arestas[pos] == NULL) {
      self->arestas[pos] = malloc(self->tam_aresta);
      assert(self->arestas[pos] != NULL);
    }

    memcpy(self->arestas[pos], p_dado, self->tam_aresta);
  }
}

void g_destrói(Grafo self)
{
  for (int i = 0; i < self->n; i++) {
    free(self->nós[i]);
  }

  for (int i = 0; i < self->n * self->n; i++) {
    free(self->arestas[i]);
  }

  free(self->nós);
  free(self->arestas);
  free(self);
}

static void diminui_matriz(Grafo self, int nó)
{
  int n = self->n;
  int novo_n = n - 1;
  void **nova = NULL;

  if (novo_n > 0) {
    nova = malloc(novo_n * novo_n * sizeof(void *));
    assert(nova != NULL);

    for (int i = 0; i < novo_n; i++) {
      for (int j = 0; j < novo_n; j++) {
        int o = (i < nó) ? i : i + 1;
        int d = (j < nó) ? j : j + 1;

        nova[i * novo_n + j] = self->arestas[o * n + d];
      }
    }
  }

  free(self->arestas);
  self->arestas = nova;
}