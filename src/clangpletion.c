#include <clang-c/Index.h>
#include <stdio.h>
#include <stdlib.h>
#include "clangpletion.h"

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

char *complete(char *args) {
  int pop_result = populate_completion_args(args);
  if (pop_result) {
    return "FAILED TO POPULATE ARGUMENTS";
  }

  char *debug_path = (char *) malloc(g_plugin_loc_len + 19);
  sprintf(debug_path, "%s/src/debug_log.txt", PLUGIN_LOC);

  FILE *debug_log = fopen(debug_path, "w");
  free((void *) debug_path);
  if (debug_log == NULL) {
    return "DEBUG FAILED";
  }
/*
  fprintf(debug_log, "DEGUG INITIALIZED\n\n");

  fprintf(debug_log, "Plugin Location: %s Buf Size: %lld\nFilename: %s Buf Size: %lld\nRow: %d\nCol: %d\nWord: %s Buf Size: %lld\n"
          "========== CONTENTS ==========\n%s\n=========== CONTENTS END ==========\nBuf Size: %lld\n",
          PLUGIN_LOC, g_plugin_loc_max, FILENAME, g_filename_max, ROW, COL, WORD, g_token_max, CONTENTS, g_contents_max);
*/
  fclose(debug_log);

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

  // Code Complete at the specified location
  CXCodeCompleteResults *comp_results = clang_codeCompleteAt(
    g_unit,
    FILENAME,
    ROW,
    COL,
    &unsaved_file,
    1,
    clang_defaultCodeCompleteOptions()
  );

  if (comp_results == NULL) {
    return "Null";
  }

  int position = 0;
  int num_results = comp_results->NumResults;
  int result = 0;
  int adding = 1;

  if (num_results) {
    while (adding){
      // Keep track of where a result's chunks begin
      int beginning = position;


      // Each result has a completion string
      CXCompletionString comp_str = (comp_results->Results)[result]
                                      .CompletionString;
      /*
        The completion string of each result is divided into a number of
        'chunks' each representing a piece of info about the completion
        string. Here, we iterate through each chunk of the completion string
        and add any 'TypedText' or 'Text' chunks to the recommendations
        string.

        'TypedText' and 'Text' chunks are actual code recommendations that
        the user may intend to type at their respective location.
      */
      int num_chunks = clang_getNumCompletionChunks(comp_str);
      for (int j = 0; j < num_chunks && adding; j++) {
        enum CXCompletionChunkKind chunk_type = clang_getCompletionChunkKind(comp_str, j);

        // Check Chunk Type
        if (chunk_type == CXCompletionChunk_TypedText ||
            chunk_type == CXCompletionChunk_Text) {
        //if (chunk_typ == CXCompletionChunk_Informative) {
          CXString chunk_str = clang_getCompletionChunkText(comp_str, j);
          const char *chunk_txt = clang_getCString(chunk_str);

          // Check if chunk matches current token
          if(compare_str(WORD, (char *) chunk_txt)) {

            // Find length of current chunk
            int chunk_len = 0;
            int next;
            while ((next = chunk_txt[chunk_len]) != '\0') {
              chunk_len++;
            }

            // Check if recommendations has the space for the chunk
            if (position + (chunk_len + 1) < COMP_MAX) {
              // if so, add chunk to recommendation
              for (int k = 0; k < chunk_len; k++) {
                recommendations[position] = chunk_txt[k];
                position++;
              }
              // add space after chunk incase current result has more
              // 'Text' or 'TypedText' chunks
              recommendations[position] = ' ';
              position++;
            } else {
            /*
               if not, end adding further chunks and results to the
               recommendation string and place a null character after
               the end of the last recommendation to denote the end
               of the string
            */
              recommendations[beginning] = '\0';
              adding = 0;
            }
          }
          clang_disposeString(chunk_str);
        }
      }
      // Replace the final space of the result with a newline to denote
      // the end of an individual recommendation
      if (adding) {
        recommendations[position - 1] = '\n';
        result++;

        if (result == num_results) {
          adding = 0;
        }
      }
    }
  }

  clang_disposeCodeCompleteResults(comp_results);
  return recommendations;
}

char *free_memory(char *args) {
  free_allocated_memory();

  return "SUCCESS";
}

//FILE *helper_debug_log;

char *function_helper(char *args) {
  int pop_result = populate_function_helper_args(args);
  if (pop_result) {
    return "FAILED TO POPULATE ARGUMENTS";
  }

  /*helper_debug_log = fopen("helper_debug_log.txt", "w");

  fprintf(helper_debug_log, "DEGUG INITIALIZED\n\n");

  fprintf(helper_debug_log, "Filename: %s Buf Size: %ld\nWord: %s Buf Size: %ld\n"
          "========== CONTENTS =========\n%s\n========= CONTENTS END ========\n"
          "Buf Size: %ld\n",
          FILENAME, g_filename_max, WORD, g_token_max, "", g_contents_max);
  */
  //fclose(debug_log);

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

  CXCursor cursor = clang_getTranslationUnitCursor(g_unit);

  clang_visitChildren(
      cursor,
      visitor,
      WORD
  );

  //fclose(helper_debug_log);

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

int populate_completion_args(char *arg_str) {
  // Reset the recommendations string

  for (int j = 0; j < COMP_MAX; j++) {
    recommendations[j] = '\0';
  }

  int i = 0;

  // Populate g_plugin_loc
  g_plugin_loc_len = 0;
  if (g_plugin_loc == NULL) {
    g_plugin_loc = malloc(PLUGIN_LOC_INIT_MAX);
    if (g_plugin_loc == NULL) {
      free_allocated_memory();
      return -1;
    }
  }
  while (arg_str[i] != '\n') {
    g_plugin_loc[g_plugin_loc_len] = arg_str[i];
    g_plugin_loc_len++;
    i++;
    if (g_plugin_loc_len >= g_plugin_loc_max) {
      g_plugin_loc_max *= 2;
      g_plugin_loc = realloc(g_plugin_loc, g_plugin_loc_max);

      if (g_plugin_loc == NULL) {
        free_allocated_memory();
        return -1;
      }
    }
  }
  g_plugin_loc[g_plugin_loc_len] = '\0';
  i++;

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
    function_help[3] = '(';
    g_help_len = 4;

    return 0;
  } else {
    function_help[i] = '(';
    i++;
    g_help_len = i;

    return 0;
  }
}

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
