#include <clang-c/Index.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FUNC_HELPER_LEN (100)
#define MAX_COMPLETION_LIST_LEN (200)
#define INCLUDE_LIST_STARTING_LEN (10)

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

extern char g_func_helper[MAX_FUNC_HELPER_LEN];
extern char g_completion_list[MAX_COMPLETION_LIST_LEN];

char *code_complete(char *);
char *func_helper(char *);
COMPLETE_ARGS *parse_complete_args(char *);
char *offset_strncpy(char *, const char *, size_t, size_t);
int begins_with(const char *, const char *);
