#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_POSICOES 13
#define NUM_ESCUDOS 3

typedef struct timespec crono;

void crono_inicia(crono *c)
{
    clock_gettime(CLOCK_MONOTONIC, c);
}

double crono_parcial(crono *c)
{
    crono agora;
    clock_gettime(CLOCK_MONOTONIC, &agora);
    double segundos = agora.tv_sec - c->tv_sec;
    double nanosegundos = agora.tv_nsec - c->tv_nsec;
    return segundos + 1e-9 * nanosegundos;
}

typedef struct {
    bool terminou_jogo;
    int pontos;
    int inimigos_inativos;
    int tiros;
    crono ultimo_movimento;
    char ataques[MAX_POSICOES];
    int num_posicoes;
    char arma;
    char escudos[NUM_ESCUDOS];
} estado_t;

void inicializa_tela()
{
    if (system("stty raw opost -echo min 0 time 1") != 0) {
        perror("erro na execução de system(\"stty\")");
        fprintf(stderr, "você tem o programa stty instalado?\n");
        exit(1);
    }
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0) {
        perror("erro na execução de setvbuf()");
        exit(1);
    }
}

void desinicializa_tela()
{
    system("stty sane");
}

void inicializa_ataques(estado_t *est)
{
    for (int i = 0; i < MAX_POSICOES; i++) {
        est->ataques[i] = ' ';
    }
}

void inicializa_escudos(estado_t *est)
{
    for (int i = 0; i < NUM_ESCUDOS; i++) {
        est->escudos[i] = ')';
    }
}

void inicializa_estado(estado_t *est)
{
    est->terminou_jogo = false;
    est->pontos = 0;
    est->tiros = 30;
    est->num_posicoes = 13;
    est->arma = '0';
    inicializa_ataques(est);
    inicializa_escudos(est);
    est->ataques[0] = '5';
    crono_inicia(&est->ultimo_movimento);
}

bool onda_terminou(estado_t *est)
{
    return est->terminou_jogo;
}

char lechar()
{
    fflush(stdout);
    char c;
    if (fread(&c, 1, 1, stdin) == 1) {
        return c;
    }
    return 0;
}

void processa_teclado(estado_t *est)
{
    char c = lechar();
    if (c == 27) {
        est->terminou_jogo = true;
    }
}

void processa_tempo(estado_t *est)
{
}

void desenha_escudos(estado_t *est)
{
    for (int i = 0; i < NUM_ESCUDOS; i++) {
        printf("%c", est->escudos[i]);
    }
}

void desenha_ataques(estado_t *est)
{
    for (int i = 0; i < est->num_posicoes; i++) {
        printf("%c", est->ataques[i]);
    }
}

void apresenta(estado_t *est)
{
    printf(" %d %d %c", est->pontos, est->tiros, est->arma);
    desenha_escudos(est);
    desenha_ataques(est);
    printf("   \r");
}

void joga_onda(estado_t *est)
{
    while (!onda_terminou(est)) {
        processa_teclado(est);
        processa_tempo(est);
        apresenta(est);
    }
}

void joga_partida(estado_t *est)
{
    while (!est->terminou_jogo) {
        joga_onda(est);
    }
}

int main()
{
    estado_t estado;
    inicializa_tela();
    inicializa_estado(&estado);
    while (!estado.terminou_jogo) {
        joga_partida(&estado);
    }
    desinicializa_tela();
}