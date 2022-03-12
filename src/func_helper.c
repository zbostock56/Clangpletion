#include <clang-c/Index.h>
#include <stdio.h>
#include <stdlib.h>

#include <func_helper.h>
#include <globals.h>

FILE *helper_debug_log;

char *function_helper(char *args) {
  int pop_result = populate_function_helper_args(args);
  if (pop_result) {
    return "FAILED TO POPULATE ARGUMENTS";
  }

  helper_debug_log = fopen("helper_debug_log.txt", "w");
  if (helper_debug_log == NULL) {
    return "FAILED TO OPEN DEBUG LOG";
  }

  fprintf(helper_debug_log, "HELPER DEBUG INITIALIZED...\n\n");

  // Clang set-up
  if (g_index == NULL) {
    g_index = clang_createIndex(0, 0);
  }

  struct CXUnsavedFile unsaved_file = { FILENAME, CONTENTS, g_contents_len };

  if (g_unit == NULL) {
    g_unit = clang_parseTranslationUnit(
      g_index,
      FILENAME,
      NULL,
      0,
      &unsaved_file,
      1,
      CXTranslationUnit_None
    );

    if (g_unit == NULL) {
      return "NULL";
    }
  }

  //CXCursor cursor = clang_getTranslationUnitCursor(g_unit);

  CXCodeCompleteResults *comp_results = clang_codeCompleteAt(
    g_unit,
    FILENAME,
    ROW,
    COL,
    &unsaved_file,
    1,
    clang_defaultCodeCompleteOptions()
  );

  CXCompletionResult *results = comp_results->Results;

  for (int i = 0; i < comp_results->NumResults; i++) {
    CXCompletionResult result = results[i];
    if (result.CursorKind == CXCursor_FunctionDecl) {
      CXCompletionString comp_str = result.CompletionString;
      CXString func_name = clang_getCompletionChunkText(comp_str, 1);

      if (compare_strl(WORD, (char *) clang_getCString(func_name))) {
        gen_help_header((char *) clang_getCString(func_name));

        for (int j = 2; j < clang_getNumCompletionChunks(comp_str); j++) {
          CXString arg = clang_getCompletionChunkText(comp_str, j);

          gen_help_arg((char *) clang_getCString(arg));

          clang_disposeString(arg);
        }
        /*clang_visitChildren(
          cursor,
          visitor,
          WORD
        );*/

        break;
      }

      clang_disposeString(func_name);
    }
  }

  if (function_help[g_help_len - 2] == ',') {
    function_help[g_help_len - 2] = ')';
    function_help[g_help_len - 1] = '\0';
  } else if (function_help[g_help_len - 1] == '(') {
    function_help[g_help_len] = ')';
    g_help_len++;
    function_help[g_help_len] = '\0';
  }
  return function_help;
}

/*
enum CXChildVisitResult visitor(CXCursor c, CXCursor parent,
                                CXClientData client_data) {
  if (((c.kind == CXCursor_FunctionDecl) && (clang_isCursorDefinition(c))) ||
      ((c.kind == CXCursor_FunctionDecl) && (function_help[0] == '\0'))) {
    CXString func_name = clang_getCursorSpelling(c);
    if (compare_strl((char *) client_data, (char *) clang_getCString(func_name))) {
      gen_help_header((char *) client_data);

      for (int i = 0; i < clang_Cursor_getNumArguments(c); i++) {
        CXCursor arg = clang_Cursor_getArgument(c, i);
        CXString type = clang_getTypeSpelling(clang_getCursorType(arg));
        CXString name = clang_getCursorSpelling(arg);
        int res = gen_help_arg((char *) clang_getCString(type),
                               (char *) clang_getCString(name));
        clang_disposeString(type);
        clang_disposeString(name);

        if (res) {
          break;
        }
      }

      clang_disposeString(func_name);

      if (clang_isCursorDefinition(c)) {
        return CXChildVisit_Break;
      }
    }
  }

  return CXChildVisit_Continue;
}
*/

int gen_help_header(char *arg) {
  char next;
  int i = 0;
  while (((next = arg[i]) != '\0') && (i < (FUNC_HELP_MAX / 2))) {
    function_help[i] = next;
    i++;
  }

  if ((i == (FUNC_HELP_MAX / 2)) && (next != '\0')) {
    function_help[0] = '.';
    function_help[1] = '.';
    function_help[2] = '.';
    //function_help[3] = '(';
    //g_help_len = 4;
    g_help_len = 3;

    return 0;
  } else {
    //function_help[i] = '(';
    //i++;
    g_help_len = i;

    return 0;
  }
}

/*
int gen_help_arg(char *type, char *name) {
  char next;
  int i = 0;
  while (((next = type[i]) != '\0') && ((i + g_help_len) < FUNC_HELP_MAX - 5)) {
    function_help[g_help_len + i] = next;
    i++;
  }

  if (((i + g_help_len) == (FUNC_HELP_MAX - 5)) && (next != '\0')) {
    function_help[g_help_len] = '.';
    function_help[g_help_len + 1] = '.';
    function_help[g_help_len + 2] = '.';
    function_help[g_help_len + 3] = ')';
    function_help[g_help_len + 4] = '\0';
    g_help_len = FUNC_HELP_MAX - 1;

    return 1;
  } else {
    function_help[g_help_len + i] = ' ';
    i++;
    g_help_len += i;
  }

  int k = 0;
  while (((next = name[k]) != '\0') && ((k + g_help_len) < FUNC_HELP_MAX - 5)) {
    function_help[g_help_len + k] = next;
    k++;
  }

  if (((g_help_len + k) == (FUNC_HELP_MAX - 5)) && (next != '\0')) {
    function_help[g_help_len - i] = '.';
    function_help[g_help_len - i + 1] = '.';
    function_help[g_help_len - i + 2] = '.';
    function_help[g_help_len - i + 3] = ')';
    function_help[g_help_len - i + 4] = '\0';
    g_help_len = FUNC_HELP_MAX - 1;

    return 1;
  } else {
    function_help[g_help_len + k] = ',';
    function_help[g_help_len + k + 1] = ' ';
    k += 2;
    g_help_len += k;

    return 0;
  }
}
*/

int gen_help_arg(char *arg) {
  char next;
  int i = 0;

  while (((next = arg[i]) != '\0') && ((i + g_help_len) < FUNC_HELP_MAX - 5)) {
    function_help[g_help_len + i] = next;
    i++;
  }

  if (next != '\0') {
    function_help[g_help_len] = '.';
    function_help[g_help_len + 1] = '.';
    function_help[g_help_len + 2] = '.';
    function_help[g_help_len + 3] = ')';
    function_help[g_help_len + 4] = '\0';
    g_help_len = FUNC_HELP_MAX - 1;

    return 1;
  } else {
    g_help_len += i;

    return 0;
  }
}
int populate_function_helper_args(char *arg_str) {
  // Reset the function helper string

  for (int j = 0; j < FUNC_HELP_MAX; j++) {
    function_help[j] = '\0';
  }

  int i = 0;

  // Populate g_filename
  g_filename_len = 0;
  if (g_filename == NULL) {
    g_filename = malloc(FILENAME_INIT_MAX);
    if (g_filename == NULL) {
      free_allocated_memory();
      return -1;
    }
  }
  while (arg_str[i] != '\n') {
    g_filename[g_filename_len] = arg_str[i];
    g_filename_len++;
    i++;
    if (g_filename_len >= g_filename_max) {
      g_filename_max *= 2;
      g_filename = realloc(g_filename, g_filename_max);

      if (g_filename == NULL) {
        free_allocated_memory();
        return -1;
      }
    }
  }
  g_filename[g_filename_len] = '\0';
  i++;

  // Populate g_row
  char row_buf[20];
  int row_len = 0;
  while (arg_str[i] != '\n') {
    row_buf[row_len] = arg_str[i];
    row_len++;
    i++;
  }
  row_buf[row_len] = '\0';
  g_row = parse_int(row_buf);
  i++;

  // Populate g_col
  char col_buf[20];
  int col_len = 0;
  while (arg_str[i] != '\n') {
    col_buf[col_len] = arg_str[i];
    col_len++;
    i++;
  }
  col_buf[col_len] = '\0';
  g_col = parse_int(col_buf);
  i++;

  // Populate g_token
  g_token_len = 0;
  if (g_token == NULL) {
    g_token = malloc(TOKEN_INIT_MAX);
    if (g_token == NULL) {
      free_allocated_memory();
      return -1;
    }
  }
  while (arg_str[i] != '\n') {
    g_token[g_token_len] = arg_str[i];
    g_token_len++;
    i++;
    if (g_token_len >= g_token_max) {
      g_token_max *= 2;
      g_token = realloc(g_token, g_token_max);

      if (g_token == NULL) {
        free_allocated_memory();
        return -1;
      }
    }
  }
  g_token[g_token_len] = '\0';
  i++;

  // Populate g_contents
  g_contents_len = 0;
  if (g_contents == NULL) {
    g_contents = malloc(CONTENTS_INIT_MAX);
    if (g_contents == NULL) {
      free_allocated_memory();
      return -1;
    }
  }
  while (arg_str[i] != '\0') {
    g_contents[g_contents_len] = arg_str[i];
    g_contents_len++;
    i++;
    if (g_contents_len >= g_contents_max) {
      g_contents_max *= 2;
      g_contents = realloc(g_contents, g_contents_max);

      if (g_contents == NULL) {
        free_allocated_memory();
        return -1;
      }
    }
  }
  g_contents[g_contents_len] = '\0';
  i++;

  return 0;

}

