// includes, constantes e declarações {{{1
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MIN_ALLOC 8    // alocação mínima

struct str {
  byte *dados;
  int nbytes;
  int alloc;
};

// A memória para conter os bytes de uma string deve ser alocada e/ou
//   realocada conforme a necessidade, cuidando para que a quantidade
//   de memória alocada seja sempre:
//   - nula (não alocada) se a string for vazia, ou
//   - não inferior ao necessário para armazenar os bytes da codificação utf8;
//   - não inferior à alocação mínima;
//   - não superior ao triplo do número de bytes necessários
//     (exceto quando for o mínimo);
//   - uma potência de 2.

// funções auxiliares {{{1

static void s_zera(Str s)
{
  s->dados = NULL;
  s->nbytes = 0;
  s->alloc = 0;
}

static void s_aloca_copia(Str s, char const *strC, int nbytes)
{
  int alloc = MIN_ALLOC;
  while (alloc < nbytes) {
    alloc = 2*alloc;
  }

  s->dados = malloc(alloc);
  assert(s->dados != NULL);

  memcpy(s->dados, strC, nbytes);

  s->nbytes = nbytes;
  s->alloc = alloc;
}

static int s_fixpos(int pos, int n)
{
  if (pos < 0) {
    pos = n + pos + 1;
  }
  return pos;
}

static byte *s_byte_de(Str_c s, int pos)
{
  int n = u8_conta_unichar_nos_bytes(s->nbytes, s->dados);
  pos = s_fixpos(pos, n);
  return u8_avanca_unichar(s-> dados, pos);
}

static int s_conta_bytes(Str_c sb){
  if (sb == NULL)
  {
   return 0;
  }
  return sb->nbytes;;
}

static void s_garante_espaco (Str s, int novo_byte)
{
  if(novo_byte == 0) {
  free (s->dados);
  s->dados = NULL;
  s->alloc = 0;
  return;
  }

  int alloc = MIN_ALLOC;
  while (alloc < novo_byte){
    alloc = 2*alloc;
  }

  if (alloc != s->alloc) {
    s->dados = realloc(s->dados, alloc);
    assert(s->dados != NULL);
    s->alloc = alloc;
  }

}

static void s_resolve_intervalo (Str_c s, int pos, int tam, int *offset_ini, int *offset_fim)
{
  int n = u8_conta_unichar_nos_bytes(s->nbytes, s->dados);
  pos = s_fixpos(pos, n);
  if (tam < 0)
  {
    tam = n - pos;
  }
  
  *offset_ini = s_byte_de(s, pos) - s->dados;
  *offset_fim = s_byte_de(s, pos + tam) - s->dados;
} 

// verifica se a string cad está de acordo com a especificação
// aborta o programa se não tiver
static void s_ok(Str_c s)
{ 
  assert(s != NULL);
  
  if (s->nbytes == 0) {
    assert(s->dados == NULL);
    assert(s->alloc == 0);
  }
  else {
    assert(s->dados != NULL);
    assert(s->alloc >= s->nbytes);  
    assert(s->alloc >= MIN_ALLOC);   
    assert((s->alloc <= 3*s->nbytes) || (s->alloc == MIN_ALLOC));
    assert ((s->alloc & (s->alloc - 1)) == 0);
  }
}

//...

// operações de criação e destruição {{{1

Str s_cria(char const *strC)
{
  Str s = malloc(sizeof(*s));
  assert(s != NULL);

  if (strC == NULL) {
   s_zera(s);
   s_ok(s);
   return s;
}

int nbytes = strlen(strC);
int nchars = u8_conta_unichar_nos_bytes(nbytes, (byte*) strC);

if (nchars == -1 || nbytes == 0) {
  s_zera(s);
}
else{
  s_aloca_copia(s,strC, nbytes);
}

s_ok(s);
return s;
}

void s_destroi(Str s)
{
  s_ok(s);
  free(s->dados);
  free(s);
}

Str s_cria_substring(Str_c s, int pos, int tam)
{
   Str nova = s_cria("");
   s_substring(nova, s, pos, tam);
   return nova;
}

Str s_cria_cópia(Str_c s)
{
   return s_cria_substring(s, 0, -1);
}

// Retorna uma nova string com o conteúdo do arquivo chamado nome.
// Retorna uma string vazia em caso de erro.
Str s_cria_de_arquivo(char *nome)
{
  Str s = s_cria("");
  //...
  return s;
}

// operações de acesso {{{1

int s_tam(Str_c s)
{
  s_ok(s);
  int nchars = u8_conta_unichar_nos_bytes(s->nbytes, s->dados);
  return nchars;
}

char *s_strc(Str_c s)
{
  s_ok(s);
  
  char *strC = malloc(s->nbytes + 1);
  assert(strC != NULL);

  memcpy(strC, s->dados, s->nbytes);

  strC[s->nbytes] = '\0';
  
  return strC;
}

unichar s_ch(Str_c s, int pos)
{
  s_ok(s);
  
  int n = u8_conta_unichar_nos_bytes(s->nbytes, s->dados);
  pos = s_fixpos(pos, n);

  if (pos < 0 || pos >= n) {
    return UNI_INV;
  }
  
  byte *ptr = u8_avanca_unichar(s->dados, pos);

  unichar uni;
  u8_unichar_nos_bytes(s->nbytes, ptr, &uni);
  
  return uni;
}


// operações de busca e comparação {{{1

bool s_igual(Str_c s, Str_c sb)
{
  s_ok(s);
  s_ok(sb);

  if (s->nbytes != sb->nbytes)
  {
    return false;
  }
  
  return memcmp(s->dados, sb->dados, s->nbytes) == 0;
}

int s_busca_c(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  //...
  return -1;
}

int s_busca_nc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  //...
  return -1;
}

int s_busca_rc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  //...
  return -1;
}

int s_busca_rnc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  //...
  return -1;
}

int s_busca_s(Str_c s, int pos, Str_c buscada)
{
  s_ok(s);
  s_ok(buscada);
  //...
  return -1;
}


// operações de alteração {{{1

void s_substitui(Str s, int pos, int tam, Str_c sb)
{
  s_ok(s);
  if (sb != NULL){
   s_ok(sb);
  }
   
  int offset_ini, offset_fim;
  s_resolve_intervalo(s, pos, tam, &offset_ini, &offset_fim);

  int bytes_sb = s_conta_bytes(sb);
  int novo_bytes = s->nbytes - (offset_fim - offset_ini) + bytes_sb;

  s_garante_espaco(s, novo_bytes);

  byte *ini = s->dados + offset_ini;
  byte *fim = s->dados + offset_fim;
  int tam_cauda = s->nbytes - offset_fim;

  memmove(ini + bytes_sb, fim, tam_cauda);
  if (sb != NULL)
  {
    memcpy(ini, sb->dados, bytes_sb);
  }

  s->nbytes = novo_bytes;
}

void s_substring(Str s, Str_c sb, int pos, int tam)
{
  s_ok(s);
  if (sb != NULL) {
    s_ok(sb);
  }

  int ini = 0, fim = 0;
  if (sb != NULL) {
    s_resolve_intervalo(sb, pos, tam, &ini, &fim);
  }
  int nbytes = fim - ini;

  free(s->dados);
  if (nbytes == 0) {
    s_zera(s);
  }
  else {
    s_aloca_copia(s, (char *) sb->dados + ini, nbytes);
  }
}

void s_copia(Str s, Str_c sb)
{
  s_substring(s, sb, 0, -1);
}

void s_insere(Str s, int pos, Str_c sb)
{
  s_substitui(s, pos, 0, sb);
}

void s_insere_c(Str s, int pos, unichar c)
{
  s_ok(s);
  //...
}

void s_anexa(Str s, Str_c sb)
{
  s_substitui(s, -1, 0, sb);
}

void s_anexa_c(Str s, unichar c)
{
  s_insere_c(s, -1, c);
}

void s_remove(Str s, int pos, int tam)
{
  s_substitui(s, pos, tam, NULL);
}

void s_apara(Str s, Str_c sobras)
{
  s_ok(s);
  s_ok(sobras);
  //...
}

// operações de E/S {{{1

void s_imprime(Str_c s)
{
  s_ok(s);
  //...
}

void s_grava_arquivo(Str_c s, char *nome)
{
  s_ok(s);
  //...
}


// vim: foldmethod=marker shiftwidth=2

