#include "lista.h"

#include <assert.h>
#include <stdlib.h>

typedef struct nó 
{
  dado_t dado;
  struct nó *ant;
  struct nó *prox;
} nó;

struct lista 
{
  nó *sentinela;
  int tam;
};

Lista l_cria()
{
  Lista l = malloc(sizeof(*l));
  assert(l != NULL);

  nó *sent = malloc(sizeof(nó));
  assert(sent != NULL);

  sent->prox = sent;
  sent->ant = sent;

  l->sentinela = sent;
  l->tam = 0;

  return l;
}

void l_destroi(Lista l)
{
nó*atual = l->sentinela->prox;
while (atual != l->sentinela) {
  nó *prox = atual ->prox;
  free(atual);
  atual = prox;
}
free(l->sentinela);
free(l);
}

// funções auxiliares (as static)
static nó *acha_nó(Lista l, int p)
{
  nó *atual = l->sentinela;
  for (int i = 0; i <= p; i++) {
    atual = atual->prox;
  }
  return atual;
}

static void insere_antes(nó *x, dado_t d)
{
  nó *novo = malloc(sizeof(nó));
  assert(novo != NULL);
  novo->dado = d;

  novo->prox = x;
  novo->ant = x->ant;
  x->ant->prox = novo;
  x->ant = novo;
}

void l_insere_pos (Lista l, dado_t d, int p)
{
  nó *x = acha_nó (l, p);
  insere_antes(x, d);
  l->tam++;
}

void l_insere_inicio(Lista l, dado_t d)
{
  l_insere_pos(l, d, 0);
}

void l_insere_fim(Lista l, dado_t d)
{
  l_insere_pos(l, d, l_tam(l));
}

int l_tam(Lista l)
{
  return l->tam;
}

bool l_vazia(Lista l)
{
  return l->tam == 0;
}

bool l_cheia(Lista l)
{
  return false;
}

dado_t l_dado_pos(Lista l, int pos)
{
  nó *x = acha_nó(l, pos);
  return x->dado;
}

dado_t l_dado_inicio(Lista l)
{
  return l->sentinela->prox->dado;
}

dado_t l_dado_fim(Lista l)
{
  return l->sentinela->ant->dado;
}

static dado_t remove_no(nó *vitima)
{
  dado_t d = vitima->dado;

  vitima->ant->prox = vitima->prox;
  vitima->prox->ant = vitima->ant;

  free(vitima);

  return d;
}

dado_t l_remove_pos(Lista l, int p)
{
  nó *x = acha_nó(l, p);
  dado_t d = remove_no(x);
  l->tam--;
  return d;
}

dado_t l_remove_inicio(Lista l)
{
  return l_remove_pos(l, 0);
}

dado_t l_remove_fim(Lista l)
{
  return l_remove_pos(l, l_tam(l) - 1);
}
