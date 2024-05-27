#include "keyboard.h"
#include "screen.h"
#include "timer.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define RESET   "\x1b[0m"
#define AZUL    "\033[1;36m"
#define VERDE   "\x1b[32m"
#define AMARELO "\x1b[33m"
#define LARANJA "\x1b[38;5;208m"
#define VERMELHO "\x1b[31m"

bool comeuComida = false;
bool iniciarJogo = false;
bool perdeuJogo = false;
int velocidadeCobra = 250000;
int contador = 0;

char comidaChar = '*';
char cobraChar = 'O';
char *corCobra = VERDE;
char *corComida = AMARELO;

FILE *fptr;
FILE *fp;

typedef struct Segmento {
  int x;
  int y;
  struct Segmento *proximo;
} Segmento;

typedef struct Cobra {
  Segmento *cabeca;
  int dx;
  int dy;
} Cobra;

typedef struct Comida {
  int x;
  int y;
} Comida;

typedef struct Ranking {
  char nome[50];
  int pontuacao;
} Ranking;


void moverCobra(Cobra *cobra, int dx, int dy, Comida *comida);
void gerarComidaEmLocalAleatorio(Comida *comida);
void exibirJogo(Cobra *cobra, Comida *comida);
bool colidiu(Cobra *cobra);
void configuracoes();
void salvarPontuacao(const char* nomeJogador, int contador);
void mostrarRanking();

int main() {
  keyboardInit();
  screenInit(0);
  char nomeJogador[50];
  printf("Por favor, insira seu nome: ");
  fgets(nomeJogador, 50, stdin);
  nomeJogador[strcspn(nomeJogador, "\n")] = 0;
  screenInit(0);
  printf("Escolha uma dificuldade: \n");
  printf("1 - Super fácil (1.0x) \n");
  printf("2 - Fácil (1.7x) \n");
  printf("3 - Médio (2.0x) \n");
  printf("4 - Difícil (2.4x) \n");
  printf("5 - Impossível (2.9x) \n");
  printf("6 - Configurações\n");
  printf("7 - Mostrar Ranking\n");
  printf("8 - Sair\n");
  printf("Pressione a tecla correspondente.\n");

  while (1) {
    if (keyhit()) {
      char tecla = readch();
      switch (tecla) {
      case '1':
        velocidadeCobra = 250000;
        iniciarJogo = true;
        break;
      case '2':
        velocidadeCobra = 170000;
        iniciarJogo = true;
        break;
      case '3':
        velocidadeCobra = 140000;
        iniciarJogo = true;
        break;
      case '4':
        velocidadeCobra = 100000;
        iniciarJogo = true;
        break;
      case '5':
        velocidadeCobra = 50000;
        iniciarJogo = true;
        break;
      case '6':
        configuracoes();
        break;
      case '7':
        mostrarRanking();
        break;
      case '8':
        keyboardDestroy();
        return 0;
      }
    }

    if (iniciarJogo) {
      srand(time(NULL));
      screenInit(1);

      Cobra cobra;
      Comida comida;

      cobra.cabeca = (Segmento*)malloc(sizeof(Segmento));
      cobra.cabeca->x = 10;
      cobra.cabeca->y = 10;
      cobra.cabeca->proximo = NULL;

      cobra.dx = 1; // -1 esquerda, 1 direita
      cobra.dy = 0; // -1 cima, 1 baixo

      gerarComidaEmLocalAleatorio(&comida);

      while (iniciarJogo) {
        if (keyhit() && !perdeuJogo) {
          int tecla = readch();
          int novo_dx = cobra.dx;
          int novo_dy = cobra.dy;

          switch (tecla) {
          case 119: // w
            novo_dx = 0;
            novo_dy = -1;
            break;
          case 97: // a
            novo_dx = -1;
            novo_dy = 0;
            break;
          case 115: // s
            novo_dx = 0;
            novo_dy = 1;
            break;
          case 100: // d
            novo_dx = 1;
            novo_dy = 0;
            break;
          case 27: // ESC
            iniciarJogo = false;
            break;
          }
          if (!(novo_dx == -cobra.dx && novo_dy == -cobra.dy)){
            cobra.dx = novo_dx;
            cobra.dy = novo_dy;
          }
        }

        moverCobra(&cobra, cobra.dx, cobra.dy, &comida);
        exibirJogo(&cobra, &comida);
        usleep(velocidadeCobra);

        if (colidiu(&cobra)){
          perdeuJogo = true;
          screenInit(0);
          fptr = fopen("ascii.txt", "r");
          char texto = fgetc(fptr);
          while (texto != EOF) {
            printf("%c", texto);
            texto = fgetc(fptr);
          }
          fclose(fptr);

          printf("\n\n Você deseja continuar jogando? (s/n)\n");

          while (1) {
            if (keyhit()) {
              char tecla = readch();
              screenInit(0);
              switch (tecla) {
              case 's':
                cobra.cabeca = (Segmento *)malloc(sizeof(Segmento));
                cobra.cabeca->x = 10;
                cobra.cabeca->y = 10;
                cobra.cabeca->proximo = NULL;

                cobra.dx = 1;
                cobra.dy = 0;

                gerarComidaEmLocalAleatorio(&comida);
                comeuComida = false;
                perdeuJogo = false;

                contador = 0;
                break;
              case 'n':
                perdeuJogo = false;
                iniciarJogo = false;
                break;
              }
              if (!perdeuJogo) {
                salvarPontuacao(nomeJogador, contador);
                break;
              }
            }
          }
        }
      }

      screenDestroy();
      keyboardDestroy();
      return 0;
    }
  }
}

void moverCobra(Cobra *cobra, int dx, int dy, Comida *comida) {
  Segmento *novoCabeca = (Segmento *)malloc(sizeof(Segmento));
  novoCabeca->x = cobra->cabeca->x + dx;
  novoCabeca->y = cobra->cabeca->y + dy;
  novoCabeca->proximo = cobra->cabeca;

  cobra->cabeca = novoCabeca;

  if (novoCabeca->x == comida->x && novoCabeca->y == comida->y) {
    comeuComida = true;
    gerarComidaEmLocalAleatorio(comida);
    contador++;
  }
  else {
    Segmento *atual = cobra->cabeca->proximo;
    Segmento *segmentoAnterior = cobra->cabeca;

    while (atual->proximo != NULL)
    {
      segmentoAnterior = atual;
      atual = atual->proximo;
    }

    segmentoAnterior->proximo = NULL;
    free(atual);
  }
}

void gerarComidaEmLocalAleatorio(Comida *comida) {
  comida->x = rand() % (MAXX - 3) + 2;
  comida->y = rand() % (MAXY - 2) + 2;
}

void exibirJogo(Cobra *cobra, Comida *comida) {
  screenInit(1);
  printf("  W - Frente   S - Trás   A - Esquerda   D - Direita   ESC - Sair\t Comida: %d", contador);

  Segmento *segmento = cobra->cabeca;
  while (segmento != NULL) {
    screenGotoxy(segmento->x, segmento->y);
    printf("%s%c%s", corCobra, cobraChar, RESET);
    segmento = segmento->proximo;
  }

  screenGotoxy(comida->x, comida->y);
  printf("%s%c%s", corComida, comidaChar, RESET);

  screenUpdate();
}

bool colidiu(Cobra *cobra) {
  if (cobra->cabeca->x <= 1 || cobra->cabeca->x >= MAXX - 1 || cobra->cabeca->y <= 1 || cobra->cabeca->y >= MAXY - 1)
  {
    return true;
  }

  Segmento *segmento = cobra->cabeca->proximo;
  while (segmento != NULL) {
    if (cobra->cabeca->x == segmento->x && cobra->cabeca->y == segmento->y) {
      return true;
    }
    segmento = segmento->proximo;
  }

  return false;
}

void configuracoes() {
  screenInit(0);
  int escolha;
  printf("Configurações do Jogo:\n");
  printf("1 - Caractere da Comida (Atual: %c)\n", comidaChar);
  printf("2 - Caractere da Cobra (Atual: %c)\n", cobraChar);
  printf("3 - Cor da Cobra (Atual: %s)\n", strcmp(corCobra, VERDE) == 0 ? "Verde" : (strcmp(corCobra, AZUL) == 0 ? "Azul" : "Vermelho"));
  printf("4 - Cor da Comida (Atual: %s)\n", strcmp(corComida, AMARELO) == 0 ? "Amarelo" : (strcmp(corComida, LARANJA) == 0 ? "Laranja" : "Azul"));
  printf("5 - Voltar ao menu principal\n");

  do {
    escolha = readch();
    switch (escolha) {
      case '1':
        printf("Escolha * , $ ou & para a comida: ");
        comidaChar = readch();
        break;
      case '2':
        printf("Escolha O, @ ou 0 para a cobra: ");
        cobraChar = readch();
        break;
      case '3':
        printf("Escolha a cor da cobra (1 - Verde, 2 - Azul, 3 - Vermelho): ");
        escolha = readch();
        corCobra = (escolha == '1') ? VERDE : (escolha == '2' ? AZUL : VERMELHO);
        break;
      case '4':
        printf("Escolha a cor da comida (1 - Amarelo, 2 - Laranja, 3 - Azul): ");
        escolha = readch();
        corComida = (escolha == '1') ? AMARELO : (escolha == '2' ? LARANJA : AZUL);
        break;
      case '5':
        return;
    }
    printf("Configuração atualizada. Pressione qualquer tecla para continuar.\n");
  } while (escolha != '5');
}

void salvarPontuacao(const char* nomeJogador, int contador) {
  FILE *fp = fopen("ranking.txt", "a");
  if (fp != NULL) {
    fprintf(fp, "%s %d\n", nomeJogador, contador);
    fclose(fp);
  } else {
    printf("Erro ao abrir arquivo de ranking.\n");
  }
}

void mostrarRanking() {
  screenInit(0);
  FILE *fp = fopen("ranking.txt", "r");
  char linha[100];

  if (fp != NULL) {
    printf("Ranking:\n");
    while (fgets(linha, sizeof(linha), fp)) {
      printf("%s", linha);
    }
    fclose(fp);
    printf("\nPressione ESC para voltar ao menu principal.\n");
    while (1) {
      if (keyhit()) {
        int tecla = readch();
        if (tecla == 27) {
          break;
        }
      }
    }
  } else {
    printf("Erro ao abrir arquivo de ranking.\n");
    printf("Pressione qualquer tecla para voltar.\n");
    while (!keyhit()) {
    }
    readch();
  }
}
