#include "calc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

static bool é_espaço(unichar c)
{
  return c == ' ' || c == '\t' || c == '\n';
}

static bool é_dígito(unichar c)
{
  return c >= '0' && c <= '9';
}

static bool é_letra(unichar c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool é_continuação_de_número(unichar c)
{
  return é_dígito(c) || c == '.';
}

static bool é_continuação_de_id(unichar c)
{
  return é_letra(c) || c == '_' || c == '$' || é_dígito(c);
}

static bool é_operador(unichar c)
{
  return c == '+' || c == '-' || c == '*' || c == '/' ||
         c == '^' || c == '(' || c == ')' || c == '=';
}

typedef enum { OPERANDO, OPERADOR, ERRO } tipo_token;

static tipo_token classifica(Str_c token)
{
  unichar c = s_ch(token, 0);
  if (s_tam(token) == 1 && é_operador(c)) {
    return OPERADOR;
  }

  if (é_dígito(c) || c == '.' || é_letra(c) || c == '_' || c == '$') {
    return OPERANDO;
  }

  return ERRO;
}

typedef enum { ACAO_TERMINA, ACAO_EMPILHA, ACAO_ERRO, ACAO_OPERA, ACAO_DESCARTA } acao;

static acao acao_pilha_vazia(unichar entrada)
{
  switch (entrada) {
    case 0:   return ACAO_TERMINA;   // F
    case ')': return ACAO_ERRO;
    default:  return ACAO_EMPILHA;
  }
}

static acao acao_mais_menos(unichar entrada)
{
  switch (entrada) {
    case 0: case ')': case '+': case '-':
      return ACAO_OPERA;
    default:
      return ACAO_EMPILHA;
  }
}

static acao acao_vezes_div(unichar entrada)
{
  switch (entrada) {
    case 0: case ')': case '+': case '-': case '*': case '/':
      return ACAO_OPERA;
    default:
      return ACAO_EMPILHA;
  }
}

static acao acao_potencia(unichar entrada)
{
  switch (entrada) {
    case 0: case ')': case '+': case '-': case '*': case '/': case '^':
      return ACAO_OPERA;
    default:
      return ACAO_EMPILHA;
  }
}

static acao acao_abre_parenteses(unichar entrada)
{
  switch (entrada) {
    case 0:   return ACAO_ERRO;
    case ')': return ACAO_DESCARTA;
    default:  return ACAO_EMPILHA;
  }
}

static acao decide_acao(unichar topo, unichar entrada)
{
  if (topo == 0) {
    return acao_pilha_vazia(entrada);
  }
  if (topo == '+' || topo == '-') {
    return acao_mais_menos(entrada);
  }
  if (topo == '*' || topo == '/') {
    return acao_vezes_div(entrada);
  }
  if (topo == '^') {
    return acao_potencia(entrada);
  }
  if (topo == '(') {
    return acao_abre_parenteses(entrada);
  }
  return ACAO_ERRO;
}

Lista tokeniza(Str txt)
{
  Lista l = l_cria();
  int n = s_tam(txt);
  int i = 0;
  while (i < n) {
    unichar c = s_ch(txt, i);
    if (é_espaço(c)) {
      i++;
      continue;
    }
    int ini = i;
    if (é_dígito(c) || c == '.') {
      while (i < n && é_continuação_de_número(s_ch(txt, i))) {
        i++;
      }
    }
    else if (é_letra(c) || c == '_' || c == '$') {
      while (i < n && é_continuação_de_id(s_ch(txt, i))) {
        i++;
      }
    }
    else {
      i++;
    }
    Str token = s_cria_substring(txt, ini, i - ini);
    l_insere_fim(l, token);
  }

  return l;
}

static bool operar(unichar op, Lista operandos)
{
  if (l_tam(operandos) < 2) {
    return false;
  }
  Str b = l_desempilha(operandos);
  Str a = l_desempilha(operandos);
  double va = s_número(a);
  double vb = s_número(b);
  double resultado = 0;
  switch (op) {
    case '+': resultado = va + vb; break;
    case '-': resultado = va - vb; break;
    case '*': resultado = va * vb; break;
    case '/': resultado = va / vb; break;
    case '^': resultado = pow(va, vb); break;
  }
  s_destroi(a);
  s_destroi(b);
  l_empilha(operandos, s_cria_número(resultado));
  return true;
}

typedef enum { PASSO_SEGUE, PASSO_CONTINUA, PASSO_ERRO } passo_result;

static passo_result trata_operando(Lista tokens, Lista operandos, Str *erro)
{
  if (l_vazia(tokens)) {
    return PASSO_SEGUE;
  }

  Str tok = l_dado_inicio(tokens);
  tipo_token tipo = classifica(tok);
  if (tipo == ERRO) {
    *erro = s_cria("#ERRO token inválido");
    return PASSO_ERRO;
  }
  if (tipo == OPERANDO) {
    l_empilha(operandos, l_remove_inicio(tokens));
    return PASSO_CONTINUA;
  }
  return PASSO_SEGUE;
}

static void processa_tabela(Lista tokens, Lista operandos, Lista operadores,
                             Str *erro, bool *terminou)
{
  unichar entrada = l_vazia(tokens) ? 0 : s_ch(l_dado_inicio(tokens), 0);
  unichar topo = l_vazia(operadores) ? 0 : s_ch(l_topo(operadores), 0);
  switch (decide_acao(topo, entrada)) {
    case ACAO_TERMINA:
      *terminou = true;
      break;
    case ACAO_ERRO:
      *erro = s_cria("#ERRO expressão mal formada");
      break;
    case ACAO_EMPILHA:
      l_empilha(operadores, l_remove_inicio(tokens));
      break;
    case ACAO_DESCARTA:
      s_destroi(l_desempilha(operadores));
      s_destroi(l_remove_inicio(tokens));
      break;
    case ACAO_OPERA: {
      Str op_tok = l_desempilha(operadores);
      unichar op = s_ch(op_tok, 0);
      s_destroi(op_tok);
      if (!operar(op, operandos)) {
        *erro = s_cria("#ERRO operandos insuficientes");
      }
      break;
    }
  }
}

static Str finaliza(Lista tokens, Lista operandos, Lista operadores, Str erro)
{
  if (erro == NULL && l_tam(operandos) != 1) {
    erro = s_cria("#ERRO expressão incompleta");
  }
  Str resultado = (erro != NULL) ? erro : l_desempilha(operandos);
  while (!l_vazia(tokens))     s_destroi(l_remove_inicio(tokens));
  while (!l_vazia(operandos))  s_destroi(l_desempilha(operandos));
  while (!l_vazia(operadores)) s_destroi(l_desempilha(operadores));
  l_destroi(tokens);
  l_destroi(operandos);
  l_destroi(operadores);

  return resultado;
}

Str calculadora(Str expressão)
{
  Lista tokens = tokeniza(expressão);
  Lista operandos = l_cria();
  Lista operadores = l_cria();
  Str erro = NULL;
  bool terminou = false;
  while (!terminou && erro == NULL) {
    passo_result r = trata_operando(tokens, operandos, &erro);
    if (r == PASSO_ERRO) {
      break;
    }
    if (r == PASSO_CONTINUA) {
      continue;
    }
    processa_tabela(tokens, operandos, operadores, &erro, &terminou);
  }

  return finaliza(tokens, operandos, operadores, erro);
}