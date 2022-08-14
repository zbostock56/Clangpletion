#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct init_args {
  char *temp_dir;
  char *contents;
} INIT_ARGS;

typedef struct complete_args {
  char *file_dir;
  char *temp_dir;
  char *include_dir;
  char *token;
  int token_len;
  int row;
  int col;
  char *includes;
} COMPLETE_ARGS;

INIT_ARGS *parse_init_args(char *);
COMPLETE_ARGS *parse_complete_args(char *);

char *parse_arg(char *, size_t *);
char *offset_strncpy(char *, const char *, size_t, size_t);
int begins_with(const char *, const char *);
