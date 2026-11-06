#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#define POSICOES_ATAQUE 10
#define NUM_ESCUDOS 3

typedef struct timespec crono;

//inicializa um cronometro
void crono_inicia(crono *c)
{
    clock_gettime(CLOCK_MONOTONIC, c);
}

//retorna tempo
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

//modo cru, leitura de tecla sem enter
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

//devolve o terminal ao modo normal
void desinicializa_tela()
{
    system("stty sane");
}

//limpa o vetor de ataques, marcando as posições como vazias
void inicializa_ataques(estado_t *est)
{
    for (int i = 0; i < POSICOES_ATAQUE; i++) {
        est->ataques[i] = ' ';
    }
}

//coloca os escudos intactos
void inicializa_escudos(estado_t *est)
{
    for (int i = 0; i < NUM_ESCUDOS; i++) {
        est->escudos[i] = ')';
    }
}

// calcula o intervalo diurno, reduzindo 10% a cada onda
double calcula_intervalo_diurno(int numero_onda)
{
    double intervalo = 2.0;
    for (int i = 1; i < numero_onda; i++) {
        intervalo *= 0.9;
    }
    return intervalo;
}

//sorteia se a onda será noturna
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

//configura os valores da onda diurna
void configura_onda_diurna(estado_t *est)
{
    est->noturno = false;
    est->num_ataques = 10;
    est->inimigos_inativos = 20;
    est->arma = '0';
}

//configura os valores da onda noturna
void configura_onda_noturna(estado_t *est)
{
    est->noturno = true;
    est->num_ataques = 5;
    est->inimigos_inativos = 15;
    est->arma = '0';
}

//decide e aplica se a onda é diurna ou noturna
void sorteia_tipo_onda(estado_t *est)
{
    if (sorteia_noturno(est->numero_onda)) {
        configura_onda_noturna(est);
    } else {
        configura_onda_diurna(est);
    }
}

//prepara uma nova onda: tipo, intervalo, tiros, ataques e cronômetro
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

//prepara o estado inicial de uma partida nova
void inicializa_estado(estado_t *est)
{
    est->terminou_jogo = false;
    est->pontos = 0;
    est->numero_onda = 1;
    inicializa_escudos(est);
    inicia_onda(est);
}

//verifica se existe algum ataque ativo no vetor de ataques
bool ha_ataque_ativo(estado_t *est)
{
    for (int i = 0; i < est->num_ataques; i++) {
        if (est->ataques[i] != ' ') {
            return true;
        }
    }
    return false;
}

//diz se a onda atual terminou, fim de jogo ou sem ataque
bool onda_terminou(estado_t *est)
{
    if (est->terminou_jogo) {
        return true;
    }
    return est->inimigos_inativos == 0 && !ha_ataque_ativo(est);
}

//lê um caracter do teclado, ou 0 se nada foi digitado
char lechar()
{
    fflush(stdout);
    char c;
    if (fread(&c, 1, 1, stdin) == 1) {
        return c;
    }
    return 0;
}

//traduz o símbolo mostrado na tela para o nome do som
char *nome_som(char simbolo)
{
    static char digito[2];
    if (simbolo == ')') {
        return "12";
    }
    if (simbolo == ' ') {
        return "x";
    }
    if (simbolo == 'N' || simbolo == 'n') {
        return "11";
    }
    digito[0] = simbolo;
    digito[1] = '\0';
    return digito;
}

//monta e executa o comando aplay para tocar o som de nome dado
void toca_som(char *nome)
{
    char comando[48];
    sprintf(comando, "aplay -q Sons/%s.3.wav &", nome);
    system(comando);
}

//avança a arma
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
    toca_som(nome_som(est->arma));
}

//acha o índice do ataque ativo mais próximo da base que bate com a arma
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

//destrói o ataque na posição i, somando pontos
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

//dispara um tiro, buscando um alvo válido
void atira(estado_t *est)
{
    if (est->tiros <= 0) {
        return;
    }
    est->tiros--;
    int i = acha_alvo(est);
    if (i != -1) {
        atira_no_alvo(est, i);
        toca_som(nome_som(est->arma));
    } else {
        toca_som("x");
    }
}

// converte a posição combinada no símbolo que está ali
char simbolo_sonar(estado_t *est, int posicao)
{
    if (posicao < NUM_ESCUDOS) {
        return est->escudos[posicao];
    }
    int indice_ataque = posicao - NUM_ESCUDOS;
    return est->ataques[indice_ataque];
}

// monta a parte do comando aplay referente a um som e acrescenta
void acrescenta_som(char *comando, char simbolo)
{
    char *nome = nome_som(simbolo);
    strcat(comando, "Sons/");
    strcat(comando, nome);
    strcat(comando, ".3.wav ");
}

// aciona o sonar, tocando o som de cada posição (escudos e ataques)
void aciona_sonar(estado_t *est)
{
    char comando[250] = "aplay -q ";
    int total = NUM_ESCUDOS + est->num_ataques;
    for (int i = 0; i < total; i++) {
        char simbolo = simbolo_sonar(est, i);
        acrescenta_som(comando, simbolo);
    }
    strcat(comando, "&");
    system(comando);
}

// lê uma tecla e executa o comando correspondente
void processa_teclado(estado_t *est)
{
    char c = lechar();
    if (c == 27) {
        est->terminou_jogo = true;
    } else if (c == '\t') {
        proxima_arma(est);
    } else if (c == '\r' || c == '\n') {
        atira(est);
    } else if (c == ' ') {
        aciona_sonar(est);
    }
}

// verifica se já passou tempo suficiente para o próximo movimento
bool deve_mover(estado_t *est)
{
    if (crono_parcial(&est->ultimo_movimento) < est->intervalo) {
        return false;
    }
    crono_inicia(&est->ultimo_movimento);
    return true;
}

//destrói o escudo mais próximo, ou termina o jogo se não houver
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

//move todos os ataques uma posição para a esquerda, tratando colisão
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

//sorteia tipo de ataque, entre os dígitos e N
char sorteia_tipo_ataque()
{
    int x = rand() % 11;
    if (x == 10) {
        return 'N';
    }
    return x + '0';
}

//coloca um novo ataque na última posição, se ainda houver inativos
void nasce_ataque(estado_t *est)
{
    est->inimigos_inativos--;
    char tipo = sorteia_tipo_ataque();
    est->ataques[est->num_ataques - 1] = tipo;
    toca_som(nome_som(tipo));
}

//processa a passagem do tempo e move os ataques e sorteia novos
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

// desenha os 3 escudos na tela
void desenha_escudos(estado_t *est)
{
    for (int i = 0; i < NUM_ESCUDOS; i++) {
        printf("%c", est->escudos[i]);
    }
}

// desenha os ataques na tela
void desenha_ataques(estado_t *est)
{
    for (int i = 0; i < est->num_ataques; i++) {
        printf("%c", est->ataques[i]);
    }
}

// desenha o estado atual do jogo na tela
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

//executa uma onda até que ela termine
void joga_onda(estado_t *est)
{
    while (!onda_terminou(est)) {
        processa_teclado(est);
        processa_tempo(est);
        apresenta(est);
    }
}

// aplica o bônus de pontos do fim de onda (tiros e escudos restantes)
void aplica_bonus_fim_onda(estado_t *est)
{
    est->pontos += est->tiros * 2;
    for (int i = 0; i < NUM_ESCUDOS; i++) {
        if (est->escudos[i] != ' ') {
            est->pontos += 10;
        }
    }
}

//monta e toca, em sequência, uma lista de sons dada
void toca_sequencia(char *nomes[], int quantidade)
{
    char comando[100] = "aplay -q ";
    for (int i = 0; i < quantidade; i++) {
        strcat(comando, "Sons/");
        strcat(comando, nomes[i]);
        strcat(comando, ".3.wav ");
    }
    strcat(comando, "&");
    system(comando);
}

// toca a sequência de sons do fim de uma onda
void toca_som_fim_onda()
{
    char *sons[] = {"12", "11", "12"};
    toca_sequencia(sons, 3);
}

//toca som do fim da partida, cogumelo do mario :)
void toca_som_fim_partida()
{
    system("aplay -q Sons/vitoria.wav &");
}

//mostra o resumo de fim de onda e espera o jogador digitar r
void espera_reiniciar(estado_t *est)
{
    toca_som_fim_onda();
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

// avança para a próxima onda
void reinicia_onda(estado_t *est)
{
    est->numero_onda++;
    inicia_onda(est);
}

// executa a partida, onda após onda, até o jogo terminar
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

// lê as 3 maiores pontuações do arquivo ou zera se não tiver
void le_pontuacoes(int pontuacoes[3])
{
    for (int i = 0; i < 3; i++) {
        pontuacoes[i] = 0;
    }
    FILE *arq;
    arq = fopen("pontuacoes.txt", "r");
    if (arq == NULL) {
        return;
    }
    for (int i = 0; i < 3; i++) {
        fscanf(arq, "%d", &pontuacoes[i]);
    }
    fclose(arq);
}

// salva as 3 maiores pontuações no arquivo
void salva_pontuacoes(int pontuacoes[3])
{
    FILE *arq;
    arq = fopen("pontuacoes.txt", "w");
    if (arq == NULL) {
        return;
    }
    for (int i = 0; i < 3; i++) {
        fprintf(arq, "%d\n", pontuacoes[i]);
    }
    fclose(arq);
}

// insere uma nova pontuação no lugar certo entre as 3 maiores
void insere_pontuacao(int pontuacoes[3], int nova)
{
    for (int i = 0; i < 3; i++) {
        if (nova > pontuacoes[i]) {
            for (int j = 2; j > i; j--) {
                pontuacoes[j] = pontuacoes[j - 1];
            }
            pontuacoes[i] = nova;
            return;
        }
    }
}

// atualiza o arquivo de recordes com a pontuação da partida atual
void atualiza_recordes(estado_t *est)
{
    int pontuacoes[3];
    le_pontuacoes(pontuacoes);
    insere_pontuacao(pontuacoes, est->pontos);
    salva_pontuacoes(pontuacoes);
}

// mostra o resumo final e pergunta se quer jogar de novo
bool pergunta_jogar_de_novo(estado_t *est)
{
    toca_som_fim_partida();
    printf("\nFim de jogo! Pontuação final: %d\r\n", est->pontos);
    printf("Jogar de novo? (s/n)\r\n");
    char c;
    do {
        c = lechar();
    } while (c != 's' && c != 'S' && c != 'n' && c != 'N');
    return (c == 's' || c == 'S');
}

int main()
{
    srand(time(NULL));
    estado_t estado;
    inicializa_tela();
    bool jogar_de_novo = true;
    while (jogar_de_novo) {
        inicializa_estado(&estado);
        joga_partida(&estado);
        atualiza_recordes(&estado);
        jogar_de_novo = pergunta_jogar_de_novo(&estado);
    }
    desinicializa_tela();
}