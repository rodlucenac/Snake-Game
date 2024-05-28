#include "keyboard.h"
#include "screen.h"
#include "timer.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define RESET          "\x1b[0m"
#define PRETO          "\x1b[30m"
#define VERMELHO       "\x1b[31m"
#define VERDE          "\x1b[32m"
#define AMARELO        "\x1b[33m"
#define AZUL           "\x1b[34m"
#define MAGENTA        "\x1b[35m"
#define CIANO          "\x1b[36m"
#define BRANCO         "\x1b[37m"
#define CINZA          "\x1b[90m"
#define VERMELHO_CLARO "\x1b[91m"

typedef struct {
  int posX;
  int posY;
} Alimento;

typedef struct Nodo {
  int posX;
  int posY;
  struct Nodo *prox;
} Nodo;

typedef struct {
  Nodo *inicio;
  int dirX;
  int dirY;
} Serpente;

typedef struct {
  char nome[50];
  int pontuacao;
} Ranking;

bool comecarJogo = false;
bool jogoPerdido = false;
int velocidade = 250000;
bool comeuAlimento = false;
int pontos = 0;

char charAlimento = '*';
char charSerpente = 'O';
char *corSerpente = VERDE;
char *corAlimento = AMARELO;

void limparTela() {
  printf("\033[H\033[J");
}

void mostrarMenu() {
  screenInit(0);
  printf("SNAKE GAME\n");
  printf("1 - Muito fácil\n");
  printf("2 - Fácil\n");
  printf("3 - Médio\n");
  printf("4 - Difícil\n");
  printf("5 - Muito dificil\n");
  printf("6 - Impossivel\n");
  printf("7 - Configurações\n");
  printf("8 - Mostrar Ranking\n");
  printf("9 - Sair\n");
}

void adicionarSegmento(Serpente *serpente, int x, int y) {
  Nodo *novoNodo = (Nodo *)malloc(sizeof(Nodo));
  novoNodo->posX = x;
  novoNodo->posY = y;
  novoNodo->prox = serpente->inicio;
  serpente->inicio = novoNodo;
}

void inicializarSerpente(Serpente *serpente) {
  serpente->inicio = NULL;
  serpente->dirX = 1;
  serpente->dirY = 0;
  adicionarSegmento(serpente, 10, 10);
}

void criarAlimento(Alimento *alimento) {
  alimento->posX = rand() % (MAXX - 3) + 2;
  alimento->posY = rand() % (MAXY - 2) + 2;
}

void moverSerpente(Serpente *serpente, Alimento *alimento) {
  int novaPosY = serpente->inicio->posY + serpente->dirY;
  int novaPosX = serpente->inicio->posX + serpente->dirX;

  adicionarSegmento(serpente, novaPosX, novaPosY);

  if (novaPosX == alimento->posX && novaPosY == alimento->posY) {
    comeuAlimento = true;
    criarAlimento(alimento);
    pontos++;
  } else {
    Nodo *atual = serpente->inicio;
    while (atual->prox->prox != NULL) {
      atual = atual->prox;
    }
    free(atual->prox);
    atual->prox = NULL;
  }
}

void desenharJogo(Serpente *serpente, Alimento *alimento) {
  screenInit(1);
  printf("  W-Frente  S-Trás  A-Esquerda  D-Direita  ESC-Sair\t Pontuação: %d", pontos);

  Nodo *nodo = serpente->inicio;
  while (nodo != NULL) {
    screenGotoxy(nodo->posX, nodo->posY);
    printf("%s%c%s", corSerpente, charSerpente, RESET);
    nodo = nodo->prox;
  }

  screenGotoxy(alimento->posX, alimento->posY);
  printf("%s%c%s", corAlimento, charAlimento, RESET);

  screenUpdate();
}

bool verificarColisao(Serpente *serpente) {
  Nodo *nodo = serpente->inicio->prox;
  while (nodo != NULL) {
    if (serpente->inicio->posX == nodo->posX && serpente->inicio->posY == nodo->posY) {
      return true;
    }
    nodo = nodo->prox;
  }
  if (serpente->inicio->posX <= 1 || serpente->inicio->posX >= MAXX - 1 || serpente->inicio->posY <= 1 || serpente->inicio->posY >= MAXY - 1) {
    return true;
  }
  return false;
}

void configuracoes() {
  screenInit(0);
  int escolha;
  printf("Configurações do Jogo:\n");
  printf("1 - Caractere da Comida (Atual: %c)\n", charAlimento);
  printf("2 - Caractere da Serpente (Atual: %c)\n", charSerpente);
  printf("3 - Cor da Serpente (Atual: %s%c%s)\n", corSerpente, 'O', RESET);
  printf("4 - Cor da Comida (Atual: %s%c%s)\n", corAlimento, '*', RESET);
  printf("5 - Voltar ao menu principal\n");

  while (1) {
    escolha = readch();
    switch (escolha) {
      case '1':
        printf("Escolha * , $ ou & para a comida: ");
        charAlimento = readch();
        break;
      case '2':
        printf("Escolha O, @ ou 0 para a serpente: ");
        charSerpente = readch();
        break;
      case '3':
        printf("Escolha a cor da serpente:\n");
        printf("1 - Preto\n2 - Vermelho\n3 - Verde\n4 - Amarelo\n5 - Azul\n6 - Magenta\n7 - Ciano\n8 - Branco\n9 - Cinza\n0 - Vermelho Claro\n");
        escolha = readch();
        switch (escolha) {
          case '1': corSerpente = PRETO; break;
          case '2': corSerpente = VERMELHO; break;
          case '3': corSerpente = VERDE; break;
          case '4': corSerpente = AMARELO; break;
          case '5': corSerpente = AZUL; break;
          case '6': corSerpente = MAGENTA; break;
          case '7': corSerpente = CIANO; break;
          case '8': corSerpente = BRANCO; break;
          case '9': corSerpente = CINZA; break;
          case '0': corSerpente = VERMELHO_CLARO; break;
        }
        break;
      case '4':
        printf("Escolha a cor da comida:\n");
        printf("1 - Preto\n2 - Vermelho\n3 - Verde\n4 - Amarelo\n5 - Azul\n6 - Magenta\n7 - Ciano\n8 - Branco\n9 - Cinza\n0 - Vermelho Claro\n");
        escolha = readch();
        switch (escolha) {
          case '1': corAlimento = PRETO; break;
          case '2': corAlimento = VERMELHO; break;
          case '3': corAlimento = VERDE; break;
          case '4': corAlimento = AMARELO; break;
          case '5': corAlimento = AZUL; break;
          case '6': corAlimento = MAGENTA; break;
          case '7': corAlimento = CIANO; break;
          case '8': corAlimento = BRANCO; break;
          case '9': corAlimento = CINZA; break;
          case '0': corAlimento = VERMELHO_CLARO; break;
        }
        break;
      case '5':
        return;
    }
    printf("Configuração atualizada. Pressione qualquer tecla para continuar.\n");
    readch(); // Wait for any key press before returning to the menu
  }
}


void gravarPontuacao(const char* nomeJogador, int pontuacao) {
  FILE *fp = fopen("ranking.txt", "a");
  if (fp != NULL) {
    fprintf(fp, "%s %d\n", nomeJogador, pontuacao);
    fclose(fp);
  } else {
    printf("Erro ao abrir arquivo de ranking.\n");
  }
}

void exibirRanking() {
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
          return;
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

int main() {
  keyboardInit();
  screenInit(0);
  char nomeJogador[50];
  limparTela();
  printf("Por favor, insira seu nome: ");
  fgets(nomeJogador, 50, stdin);
  nomeJogador[strcspn(nomeJogador, "\n")] = 0;

  limparTela();
  printf("Bem-vindo, %s!\n", nomeJogador);
  sleep(2);

  mostrarMenu();

  while (1) {
    if (keyhit()) {
      char tecla = readch();
      switch (tecla) {
      case '1':
        velocidade = 150000;
        comecarJogo = true;
        break;
      case '2':
        velocidade = 100000;
        comecarJogo = true;
        break;
      case '3':
        velocidade = 80000;
        comecarJogo = true;
        break;
      case '4':
        velocidade = 50000;
        comecarJogo = true;
        break;
      case '5':
        velocidade = 30000;
        comecarJogo = true;
        break;
      case '6':
        velocidade = 20000;
        comecarJogo = true;
        break;
      case '7':
        configuracoes();
        mostrarMenu();
        break;
      case '8':
        exibirRanking();
        mostrarMenu();
        break;
      case '9':
        keyboardDestroy();
        return 0;
      }
    }

    if (comecarJogo) {
      srand(time(NULL));
      screenInit(1);

      Alimento alimento;
      Serpente serpente;
      criarAlimento(&alimento);
      inicializarSerpente(&serpente);

      while (comecarJogo) {
        if (keyhit() && !jogoPerdido) {
          int tecla = readch();
          int novo_dx = serpente.dirX;
          int novo_dy = serpente.dirY;

          switch (tecla) {
          case 115:
            novo_dx = 0;
            novo_dy = 1;
            break;
          case 119:
              novo_dx = 0;
              novo_dy = -1;
              break;
          case 97:
              novo_dx = -1;
              novo_dy = 0;
              break;
          case 27:
              comecarJogo = false;
              gravarPontuacao(nomeJogador, pontos);
              pontos = 0;
              mostrarMenu();
              break;
          case 100:
              novo_dx = 1;
              novo_dy = 0;
              break;
          }
          if (!(novo_dy == -serpente.dirY && novo_dx == -serpente.dirX)) {
              serpente.dirY = novo_dy;
              serpente.dirX = novo_dx;
          }
        }

        moverSerpente(&serpente, &alimento);
        desenharJogo(&serpente, &alimento);
        usleep(velocidade);

        if (verificarColisao(&serpente)) {
          jogoPerdido = true;
          screenInit(0);
          printf("\n\n Você deseja continuar jogando? (s/n)\n");
          while (1) {
            if (keyhit()) {
              char tecla = readch();
              if (tecla == 's') {
                inicializarSerpente(&serpente);
                criarAlimento(&alimento);
                comeuAlimento = false;
                jogoPerdido = false;
                pontos = 0;
                break;
              } else if (tecla == 'n') {
                jogoPerdido = false;
                comecarJogo = false;
                gravarPontuacao(nomeJogador, pontos);
                pontos = 0;
                mostrarMenu();
                break;
              }
            }
          }
        }
      }
      screenDestroy();
      mostrarMenu();
    }
  }
  keyboardDestroy();
  return 0;
}
