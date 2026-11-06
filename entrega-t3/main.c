#include "calc.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
  char *nome_entrada = (argc > 1) ? argv[1] : "entrada.txt";
  char *nome_saida = (argc > 2) ? argv[2] : "saida.txt";
  Str texto = s_cria_de_arquivo(nome_entrada);
  Str quebra_linha = s_cria("\n");
  Lista linhas = l_cria_separando(texto, quebra_linha);
  Lista resultados = l_cria();
  int n = l_tam(linhas);
  for (int i = 0; i < n; i++) {
    Str linha = l_dado_pos(linhas, i);
    Str resultado = calculadora(linha);
    l_insere_fim(resultados, resultado);
  }

  Str texto_saida = s_cria_unindo(resultados, quebra_linha);
  s_grava_arquivo(texto_saida, nome_saida);

  // limpeza
  s_destroi(texto);
  s_destroi(quebra_linha);
  s_destroi(texto_saida);
  while (!l_vazia(linhas))      s_destroi(l_remove_inicio(linhas));
  while (!l_vazia(resultados))  s_destroi(l_remove_inicio(resultados));
  l_destroi(linhas);
  l_destroi(resultados);

  return 0;
}