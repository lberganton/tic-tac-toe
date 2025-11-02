#include <arpa/inet.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 8080
#define PLAYER_ONE_CHAR 'O'
#define PLAYER_TWO_CHAR 'X'

static inline void help(void) {
  printf("Usage: tic-tac-toe <create/connect> [options]...\n");
  printf("options:\n");
  printf("  -h    Print this message.\n");
  printf("  -p    The port that will be used. Whether not provided, %d will be "
         "used.\n",
         DEFAULT_PORT);
}

static inline void print_board(char board[3][3]) {
  printf(" %c | %c | %c\n", board[0][0], board[0][1], board[0][2]);
  printf("---+---+---\n");
  printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
  printf("---+---+---\n");
  printf(" %c | %c | %c\n", board[2][0], board[2][1], board[2][2]);
}

static inline void get_play(char board[3][3], char ch) {
  char play;
  do {
    printf("Your play (1-9): ");
    scanf(" %c", &play);
    while (getchar() != '\n');
    play -= '1';
  } while (play < 0 || play > 8 || board[play / 3][play % 3] != ' ');
  board[play / 3][play % 3] = ch;
}

static inline bool check_win(char board[3][3], char ch) {
  return (board[0][0] == ch && board[0][1] == ch && board[0][2] == ch) ||
         (board[1][0] == ch && board[1][1] == ch && board[1][2] == ch) ||
         (board[2][0] == ch && board[2][1] == ch && board[2][2] == ch) ||
         (board[0][0] == ch && board[1][0] == ch && board[2][0] == ch) ||
         (board[0][1] == ch && board[1][1] == ch && board[2][1] == ch) ||
         (board[0][2] == ch && board[1][2] == ch && board[2][2] == ch) ||
         (board[0][0] == ch && board[1][1] == ch && board[2][2] == ch) ||
         (board[0][2] == ch && board[1][1] == ch && board[2][0] == ch);
}

static inline bool check_draw(char board[3][3]) {
  return board[0][0] != ' ' && board[0][1] != ' ' && board[0][2] != ' ' &&
         board[1][0] != ' ' && board[1][1] != ' ' && board[1][2] != ' ' &&
         board[2][0] != ' ' && board[2][1] != ' ' && board[2][2] != ' ';
}

static void tic_tac_toe(int other_player_socket, bool is_player_one) {
  const char my_symbol = is_player_one ? PLAYER_ONE_CHAR : PLAYER_TWO_CHAR;
  const char adversary_symbol = is_player_one ? PLAYER_TWO_CHAR : PLAYER_ONE_CHAR;
  char board[3][3] = {
    {' ', ' ', ' '},
    {' ', ' ', ' '},
    {' ', ' ', ' '}
  };

  printf("You are the %c\n", my_symbol);
  print_board(board);

  if (is_player_one) {
    get_play(board, PLAYER_ONE_CHAR);
    if (send(other_player_socket, board, sizeof(board), 0) == -1) {
      perror("send");
      return;
    }
    print_board(board);
  }

  while (true) {
    printf("Waiting for player %s...\n", is_player_one ? "two" : "one");
    if (recv(other_player_socket, board, sizeof(board), 0) == -1) {
      perror("recv");
      return;
    }
    print_board(board);
    if (check_win(board, adversary_symbol)) {
      printf("You lose!\n");
      return;
    }
    if (check_draw(board)) {
      printf("The game endup with a draw!\n");
      return;
    }
    get_play(board, my_symbol);
    if (send(other_player_socket, board, sizeof(board), 0) == -1) {
      perror("send");
      return;
    }
    print_board(board);
    if (check_win(board, my_symbol)) {
      printf("You win!\n");
      return;
    }
    if (check_draw(board)) {
      printf("The game endup with a draw!\n");
      return;
    }
  }
}

static int create_server(int port) {
  int server = socket(AF_INET, SOCK_STREAM, 0);
  if (server == -1) {
    perror("socket");
    return 1;
  }

  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (int[]) { 1 }, sizeof(int));

  struct sockaddr_in addr = {
    .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
    .sin_port = htons(port),
    .sin_family = AF_INET
  };

  if (bind(server, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
    perror("bind");
    close(server);
    return 1;
  }

  if (listen(server, 1) == -1) {
    perror("listen");
    close(server);
    return 1;
  }

  printf("Server started at port %d\n", port);

  int client = accept(server, (struct sockaddr*) &addr, (socklen_t[]) { sizeof(addr) });
  if (client == -1) {
    perror("accept");
    close(server);
    return 1;
  }

  tic_tac_toe(client, true);

  close(client);
  close(server);
  return 0;
}

static int connect_server(int port) {
  int server = socket(AF_INET, SOCK_STREAM, 0);
  if (server == -1) {
    perror("server");
    return 1;
  }

  struct sockaddr_in addr = {
    .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
    .sin_port = htons(port),
    .sin_family = AF_INET
  };

  if (connect(server, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
    perror("connect");
    close(server);
    return 1;
  }

  tic_tac_toe(server, false);

  close(server);
  return 0;
}

int main(int argc, char *argv[]) {
  int port = 0;

  int opt;
  bool error = false;

  while ((opt = getopt(argc, argv, ":hp:")) != -1) {
    char *ptr;
    switch (opt) {
    case 'p':
      port = strtol(optarg, &ptr, 0);
      if (!*optarg || *ptr || port < 1024 || port > 65535) {
        fprintf(stderr, "The port must be a value between 1024 and 65535\n");
        error = true;
      }
      break;
    case '?':
      fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
      error = true;
      break;
    case ':':
      fprintf(stderr, "Option -%c requires a operand\n", optopt);
      error = true;
      break;
    case 'h':
      help();
      return 0;
    }
  }

  if (error) {
    help();
    return 1;
  }

  if (argc - optind != 1) {
    help();
    return 1;
  }

  port = port == 0 ? DEFAULT_PORT : port;

  if (strcmp("create", argv[optind]) == 0) {
    return create_server(port);
  }
  if (strcmp("connect", argv[optind]) == 0) {
    return connect_server(port);
  }

  fprintf(stderr, "Invalid option '%s'\n", argv[optind]);
  help();

  return 1;
}
