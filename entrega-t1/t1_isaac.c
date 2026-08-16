#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define POSICOES_ATAQUE 10
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
    bool noturno;
    int numero_onda;
    int pontos;
    int inimigos_inativos;
    int tiros;
    crono ultimo_movimento;
    char ataques[POSICOES_ATAQUE];
    int num_ataques;
    double intervalo;
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
    for (int i = 0; i < POSICOES_ATAQUE; i++) {
        est->ataques[i] = ' ';
    }
}

void inicializa_escudos(estado_t *est)
{
    for (int i = 0; i < NUM_ESCUDOS; i++) {
        est->escudos[i] = ')';
    }
}

double calcula_intervalo_diurno(int numero_onda)
{
    double intervalo = 2.0;
    for (int i = 1; i < numero_onda; i++) {
        intervalo *= 0.9;
    }
    return intervalo;
}

bool sorteia_noturno(int numero_onda)
{
    int chances[] = {100, 80, 60, 40};
    int chance_diurna;
    if (numero_onda <= 4) {
        chance_diurna = chances[numero_onda - 1];
    } else {
        chance_diurna = 20;
    }
    return (rand() % 100) >= chance_diurna;
}

void configura_onda_diurna(estado_t *est)
{
    est->noturno = false;
    est->num_ataques = 10;
    est->inimigos_inativos = 20;
    est->arma = '0';
}

void configura_onda_noturna(estado_t *est)
{
    est->noturno = true;
    est->num_ataques = 5;
    est->inimigos_inativos = 15;
    est->arma = '0';
}

void sorteia_tipo_onda(estado_t *est)
{
    if (sorteia_noturno(est->numero_onda)) {
        configura_onda_noturna(est);
    } else {
        configura_onda_diurna(est);
    }
}

void inicia_onda(estado_t *est)
{
    sorteia_tipo_onda(est);
    est->intervalo = calcula_intervalo_diurno(est->numero_onda);
    if (est->noturno) {
        est->intervalo *= 3;
    }
    est->tiros = 30;
    inicializa_ataques(est);
    crono_inicia(&est->ultimo_movimento);
}

void inicializa_estado(estado_t *est)
{
    est->terminou_jogo = false;
    est->pontos = 0;
    est->numero_onda = 1;
    inicializa_escudos(est);
    inicia_onda(est);
}

bool ha_ataque_ativo(estado_t *est)
{
    for (int i = 0; i < est->num_ataques; i++) {
        if (est->ataques[i] != ' ') {
            return true;
        }
    }
    return false;
}

bool onda_terminou(estado_t *est)
{
    if (est->terminou_jogo) {
        return true;
    }
    return est->inimigos_inativos == 0 && !ha_ataque_ativo(est);
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

void proxima_arma(estado_t *est)
{
    char *sequencia = est->noturno ? "02468n" : "0123456789n";
    int tamanho = est->noturno ? 6 : 11;
    int i = 0;
    while (sequencia[i] != est->arma) {
        i++;
    }
    i = (i + 1) % tamanho;
    est->arma = sequencia[i];
}

int acha_alvo(estado_t *est)
{
    for (int i = 0; i < est->num_ataques; i++) {
        char tipo = est->ataques[i];
        if (tipo == est->arma) {
            return i;
        }
        if (est->arma == 'n' && tipo == 'N') {
            return i;
        }
    }
    return -1;
}

void atira_no_alvo(estado_t *est, int i)
{
    if (est->arma == 'n' && est->ataques[i] == 'N') {
        est->ataques[i] = 'n';
        return;
    }
    int pontos_base = est->num_ataques - i;
    int multiplicador;
    if (est->arma == 'n') {
        multiplicador = 2;
    } else {
        multiplicador = 1;
    }
    if (est->noturno) {
        multiplicador *= 2;
    }
    est->ataques[i] = ' ';
    est->pontos += pontos_base * multiplicador;
}

void atira(estado_t *est)
{
    if (est->tiros <= 0) {
        return;
    }
    est->tiros--;
    int i = acha_alvo(est);
    if (i != -1) {
        atira_no_alvo(est, i);
    }
}

void proxima_arma(estado_t *est)
{
    char *sequencia;
    int tamanho;
    if (est->noturno) {
        sequencia = "02468n";
        tamanho = 6;
    } else {
        sequencia = "0123456789n";
        tamanho = 11;
    }
    int i = 0;
    while (sequencia[i] != est->arma) {
        i++;
    }
    i = (i + 1) % tamanho;
    est->arma = sequencia[i];
}

bool deve_mover(estado_t *est)
{
    if (crono_parcial(&est->ultimo_movimento) < est->intervalo) {
        return false;
    }
    crono_inicia(&est->ultimo_movimento);
    return true;
}

void trata_colisao(estado_t *est)
{
    for (int i = NUM_ESCUDOS - 1; i >= 0; i--) {
        if (est->escudos[i] != ' ') {
            est->escudos[i] = ' ';
            return;
        }
    }
    est->terminou_jogo = true;
}

void move_ataques(estado_t *est)
{
    char saiu = est->ataques[0];
    for (int i = 0; i < est->num_ataques - 1; i++) {
        est->ataques[i] = est->ataques[i + 1];
    }
    est->ataques[est->num_ataques - 1] = ' ';
    if (saiu != ' ') {
        trata_colisao(est);
    }
}

char sorteia_tipo_ataque()
{
    int x = rand() % 11;
    if (x == 10) {
        return 'N';
    }
    return x + '0';
}

void nasce_ataque(estado_t *est)
{
    est->inimigos_inativos--;
    est->ataques[est->num_ataques - 1] = sorteia_tipo_ataque();
}

void processa_tempo(estado_t *est)
{
    if (!deve_mover(est)) {
        return;
    }
    move_ataques(est);
    if (est->inimigos_inativos > 0) {
        nasce_ataque(est);
    }
}

void desenha_escudos(estado_t *est)
{
    for (int i = 0; i < NUM_ESCUDOS; i++) {
        printf("%c", est->escudos[i]);
    }
}

void desenha_ataques(estado_t *est)
{
    for (int i = 0; i < est->num_ataques; i++) {
        printf("%c", est->ataques[i]);
    }
}

void apresenta(estado_t *est)
{
    if (est->noturno) {
        printf(" %d   \r", est->pontos);
        return;
    }
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

void aplica_bonus_fim_onda(estado_t *est)
{
    est->pontos += est->tiros * 2;
    for (int i = 0; i < NUM_ESCUDOS; i++) {
        if (est->escudos[i] != ' ') {
            est->pontos += 10;
        }
    }
}

void espera_reiniciar(estado_t *est)
{
    printf("\nFim da onda! Pontuação atualizada. Aperte 'r' para continuar.\r\n");
    char c;
    do {
        c = lechar();
        if (c == 27) {
            est->terminou_jogo = true;
            return;
        }
    } while (c != 'r' && c != 'R');
}

void reinicia_onda(estado_t *est)
{
    est->numero_onda++;
    inicia_onda(est);
}

void joga_partida(estado_t *est)
{
    while (!est->terminou_jogo) {
        joga_onda(est);
        if (!est->terminou_jogo) {
            aplica_bonus_fim_onda(est);
            espera_reiniciar(est);
            if (!est->terminou_jogo) {
                reinicia_onda(est);
            }
        }
    }
}

int main()
{
    srand(time(NULL));
    estado_t estado;
    inicializa_tela();
    inicializa_estado(&estado);
    while (!estado.terminou_jogo) {
        joga_partida(&estado);
    }
    desinicializa_tela();
}