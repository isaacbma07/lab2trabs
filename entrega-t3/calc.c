#include "calc.h"
#include "dicionario.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

static bool é_espaço(unichar c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
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
    case 0: case ')': case '+': case '-': case '*': case '/':
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

static acao acao_igual(unichar entrada)
{
  switch (entrada) {
    case 0: case ')':
      return ACAO_OPERA;
    default:
      return ACAO_EMPILHA;
  }
}

static acao decide_acao(unichar topo, unichar entrada)
{
  if (topo == 0) {
    return acao_pilha_vazia(entrada);
  }
  if (topo == '=') {
    return acao_igual(entrada);
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

//variaveis dicionario
static bool chave_igual(chave_t a, chave_t b)
{
  return s_igual((Str_c) a, (Str_c) b);
}

static bool chave_menor(chave_t a, chave_t b)
{
  return false;
}

static Dicionário obtem_dicionário()
{
  static Dicionário d = NULL;
  if (d == NULL) {
    d = dic_cria(chave_menor, chave_igual);
  }
  return d;
}

static bool é_número_token(Str_c tok)
{
  unichar c = s_ch(tok, 0);
  return é_dígito(c) || c == '.' || c == '-';
}

static bool é_número_válido(Str_c tok)
{
  int n = s_tam(tok);
  int pontos = 0;
  int dígitos = 0;
  for (int i = 0; i < n; i++) {
    unichar c = s_ch(tok, i);
    if (c == '.') {
      pontos++;
    }
    else if (é_dígito(c)) {
      dígitos++;
    }
    else {
      return false;
    }
  }
  return pontos <= 1 && dígitos > 0;
}

static bool valor_de(Str_c operando, double *valor)
{
  if (é_número_token(operando)) {
    *valor = s_número(operando);
    return true;
  }

  Dicionário d = obtem_dicionário();
  valor_t v = dic_busca(d, (chave_t) operando);
  if (v == VALOR_NÃO_EXISTE) {
    return false; 
  }

  *valor = s_número((Str) v);
  return true;
}

static Str valor_como_string(Str_c operando)
{
  if (é_número_token(operando)) {
    return s_cria_cópia(operando);
  }

  Dicionário d = obtem_dicionário();
  valor_t v = dic_busca(d, (chave_t) operando);
  if (v == VALOR_NÃO_EXISTE) {
    return NULL;
  }
  return s_cria_cópia((Str) v);
}

static bool atribui(Lista operandos, Str *erro)
{
  if (l_tam(operandos) < 2) {
    *erro = s_cria("#ERRO operandos insuficientes");
    return false;
  }

  Str b = l_desempilha(operandos);
  Str a = l_desempilha(operandos);
  if (é_número_token(a)) {
    s_destroi(a);
    s_destroi(b);
    *erro = s_cria("#ERRO atribuição a um número");
    return false;
  }

  Str valor_str = valor_como_string(b);
  s_destroi(b);
  if (valor_str == NULL) {
    s_destroi(a);
    *erro = s_cria("#ERRO variável não existe");
    return false;
  }

  Dicionário d = obtem_dicionário();
  valor_t antigo = dic_insere(d, (chave_t) a, (valor_t) valor_str);
  if (antigo != VALOR_NÃO_EXISTE) {
    s_destroi(a);
    s_destroi((Str) antigo);
  }

  l_empilha(operandos, s_cria_cópia(valor_str));
  return true;
}

static bool operar(unichar op, Lista operandos, Str *erro)
{
  if (l_tam(operandos) < 2) {
    *erro = s_cria("#ERRO operandos insuficientes");
    return false;
  }
  Str b = l_desempilha(operandos);
  Str a = l_desempilha(operandos);
  double va = 0;
  double vb = 0;
  double resultado = 0;
  bool ok = valor_de(a, &va) && valor_de(b, &vb);
  if (!ok) {
    *erro = s_cria("#ERRO variável não existe");
  }
  else if (op == '/' && vb == 0) {
    *erro = s_cria("#ERRO divisão por zero");
    ok = false;
  }
  else {
    switch (op) {
      case '+': resultado = va + vb; break;
      case '-': resultado = va - vb; break;
      case '*': resultado = va * vb; break;
      case '/': resultado = va / vb; break;
      case '^': resultado = pow(va, vb); break;
    }
    if (!isfinite(resultado)) {
      *erro = s_cria("#ERRO resultado inválido");
      ok = false;
    }
  }
  s_destroi(a);
  s_destroi(b);
  if (!ok) {
    return false;
  }
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
    if (é_número_token(tok) && !é_número_válido(tok)) {
      *erro = s_cria("#ERRO número inválido");
      return PASSO_ERRO;
    }
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
     *erro = s_cria(entrada == ')' ? "#ERRO falta (" : "#ERRO falta )");
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
      Str erro_específico = NULL;
      bool ok;
      if (op == '=') {
        ok = atribui(operandos, &erro_específico);
      }
      else {
        ok = operar(op, operandos, &erro_específico);
      }
      if (!ok) {
        *erro = erro_específico;
      }
      break;
    }
  }
} 

static Str resolve_resultado_final(Str_c operando, Str *erro)
{
  if (é_número_token(operando)) {
    return s_cria_cópia(operando);
  }
  Str valor = valor_como_string(operando);
  if (valor == NULL) {
    *erro = s_cria("#ERRO variável não existe");
  }
  return valor;
}

static Str finaliza(Lista tokens, Lista operandos, Lista operadores, Str erro)
{
  if (erro == NULL && l_tam(operandos) != 1) {
    erro = s_cria("#ERRO expressão incompleta");
  }

  Str resultado;
  if (erro != NULL) {
    resultado = erro;
  }
  else {
    Str op = l_desempilha(operandos);
    resultado = resolve_resultado_final(op, &erro);
    s_destroi(op);
    if (resultado == NULL) {
      resultado = erro;
    }
  }

  while (!l_vazia(tokens)) {
    s_destroi(l_remove_inicio(tokens));
  }

  while (!l_vazia(operandos)) {
    s_destroi(l_desempilha(operandos));
  }

  while (!l_vazia(operadores)) {
    s_destroi(l_desempilha(operadores));
  }

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
