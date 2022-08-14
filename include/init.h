#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct init_args {
  char *temp_dir;
  char *contents;
} INIT_ARGS;

char *init(char *);
INIT_ARGS *parse_init_args(char *);
