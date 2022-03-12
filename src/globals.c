#include <clang-c/Index.h>
#include <stdio.h>
#include <stdlib.h>

#include <globals.h>

char *g_plugin_loc = NULL;
size_t g_plugin_loc_max = PLUGIN_LOC_INIT_MAX;
size_t g_plugin_loc_len = 0;

char *g_filename = NULL;
size_t g_filename_max = FILENAME_INIT_MAX;
size_t g_filename_len = 0;

int g_row = -1;
int g_col = -1;

char *g_token = NULL;
size_t g_token_max = TOKEN_INIT_MAX;
size_t g_token_len = 0;

char *g_contents = NULL;
size_t g_contents_max = CONTENTS_INIT_MAX;
size_t g_contents_len = 0;

char recommendations[COMP_MAX];

char function_help[FUNC_HELP_MAX];
int g_help_len = 0;

CXIndex g_index = NULL;
CXTranslationUnit g_unit = NULL;

char *free_memory(char *args) {
  free_allocated_memory();

  return "SUCCESS";
}

int free_allocated_memory(void) {
  if (g_plugin_loc != NULL) {
    free(g_plugin_loc);
    g_plugin_loc = NULL;
    g_plugin_loc_len = 0;
  }
  if (g_filename != NULL) {
    free(g_filename);
    g_filename = NULL;
    g_filename_len = 0;
  }
  if (g_token != NULL) {
    free(g_token);
    g_token = NULL;
    g_token_len = 0;
  }
  if (g_contents != NULL) {
    free(g_contents);
    g_contents = NULL;
    g_contents_len = 0;
  }
  if (g_index != NULL) {
    clang_disposeIndex(g_index);
    g_index = NULL;
  }
  if (g_unit != NULL) {
    clang_disposeTranslationUnit(g_unit);
    g_unit = NULL;
  }

  return 0;
}

int parse_int(char *str) {
  int num = 0;
  char next;
  int i = 0;
  while ((next = str[i]) != '\0') {
    if (next < '0' || next > '9') {
      return -1;
    }
    num *= 10;
    num += (next - '0');
    i++;
  }
  return num;
}

int compare_str(char *str1, char *str2) {
  char next;
  int i = 0;
  while ((next = str1[i]) != '\0') {
    if (str1[i] != str2[i]) {
      return 0;
    }
    i++;
  }
  return 1;
}

int compare_strl(char *str1, char *str2) {
  char next;
  int i = 0;
  while ((next = str1[i]) != '\0') {
    if (str1[i] != str2[i]) {
      return 0;
    }
    i++;
  }
  if (str1[i] != str2[i]) {
    return 0;
  }

  return 1;
}
