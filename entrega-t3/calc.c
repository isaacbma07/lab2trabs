#include "calc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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