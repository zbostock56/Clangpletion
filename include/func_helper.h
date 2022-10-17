#include <clang-c/Index.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EQUAL (0)
#define LESS (1)
#define GREATER (2)
#define MAX_FUNC_HELPER_LEN (100)
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

typedef struct comparison_args {
  char *token;
  int token_len;
  CXCompletionResult *comp_results;
  unsigned int index;
} COMP_ARGS;

typedef struct add_args {
  char *token;
  CXCompletionResult *comp_results;
  unsigned int num_results;
  unsigned int cur_index;
  size_t *buffer_len;
} ADD_ARGS;


extern char g_func_helper[MAX_FUNC_HELPER_LEN];
char *func_helper(char *);

void binary_search(char *token, int token_len,
                   CXCompletionResult *comp_results,
                   unsigned int num_results, size_t *buffer_len,
                   int comp_func(COMP_ARGS *args),
                   void add_func(ADD_ARGS *args));
int func_helper_compare(COMP_ARGS *args);
void func_helper_add(ADD_ARGS *args);

COMPLETE_ARGS *parse_complete_args(char *);
char *offset_strncpy(char *, const char *, size_t, size_t);
