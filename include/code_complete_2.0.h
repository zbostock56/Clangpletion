#include <clang-c/Index.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EQUAL (0)
#define LESS (1)
#define GREATER (2)
#define MAX_FUNC_HELPER_LEN (100)
#define MAX_COMPLETION_LIST_LEN (200)
#define INCLUDE_LIST_STARTING_LEN (10)
#define RESULT_THRESHOLD (1000)

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

extern char g_completion_list[MAX_COMPLETION_LIST_LEN];

char *code_complete(char *);
void binary_search(char *token, int token_len,
                   CXCompletionResult *comp_results,
                   unsigned int num_results, size_t *buffer_len,
                   int comp_func(COMP_ARGS *args),
                   void add_func(ADD_ARGS *args));
int comp_list_compare(COMP_ARGS *args);
void comp_list_add(ADD_ARGS *args);
void linear_search(char *token, CXCompletionResult *comp_results,
                unsigned int num_results, size_t *comp_list_len);

COMPLETE_ARGS *parse_complete_args(char *);
char *offset_strncpy(char *, const char *, size_t, size_t);
int begins_with(const char *, const char *);
int get_typed_text(CXCompletionString *);
