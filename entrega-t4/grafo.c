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
  int consulta_nó;
  bool consulta_partem;
  int consulta_pos;
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
  self->consulta_nó = 0;
  self->consulta_partem = true;
  self->consulta_pos = 0;

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

static void remove_arestas_do_nó(Grafo self, int nó)
{
  for (int i = 0; i < self->n; i++) {
    g_altera_valor_da_aresta(self, nó, i, NULL);
    g_altera_valor_da_aresta(self, i, nó, NULL);
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

static int índice_antigo(int novo, int nó)
{
  if (novo < nó) {
    return novo;
  }
  return novo + 1;
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
        int o = índice_antigo(i, nó);
        int d = índice_antigo(j, nó);
        nova[i * novo_n + j] = self->arestas[o * n + d];
      }
    }
  }
  free(self->arestas);
  self->arestas = nova;
}

void g_remove_nó(Grafo self, int nó)
{
  assert(nó >= 0 && nó < self->n);
  remove_arestas_do_nó(self, nó);
  diminui_matriz(self, nó);
  free(self->nós[nó]);
  for (int i = nó; i < self->n - 1; i++) {
    self->nós[i] = self->nós[i + 1];
  }
  self->n--;
}

void g_arestas_que_partem(Grafo self, int nó_origem)
{
  assert(nó_origem >= 0 && nó_origem < self->n);
  self->consulta_nó = nó_origem;
  self->consulta_partem = true;
  self->consulta_pos = 0;
}

void g_arestas_que_chegam(Grafo self, int nó_destino)
{
  assert(nó_destino >= 0 && nó_destino < self->n);
  self->consulta_nó = nó_destino;
  self->consulta_partem = false;
  self->consulta_pos = 0;
}

static int índice_da_consulta(Grafo self)
{
  if (self->consulta_partem) {
    return self->consulta_nó * self->n + self->consulta_pos;
  }

  return self->consulta_pos * self->n + self->consulta_nó;
}

void *g_próxima_aresta(Grafo self, int *p_nó_vizinho)
{
  while (self->consulta_pos < self->n) {
    int pos = self->consulta_pos;

    int índice = índice_da_consulta(self);
    self->consulta_pos++;

    void *aresta = self->arestas[índice];

    if (aresta != NULL) {
      if (p_nó_vizinho != NULL) {
        *p_nó_vizinho = pos;
      }

      return aresta;
    }
  }

  return NULL;
}

bool g_tem_ciclo(Grafo self)
{
  return false;
}

Fila g_ordem_topológica(Grafo self)
{
  Fila fila = f_cria(sizeof(int));

  for (int i = 0; i < self->n; i++) {
    f_insere(fila, &i);
  }

  return fila;
}