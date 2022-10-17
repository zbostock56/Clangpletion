#include <func_helper.h>

char g_func_helper[MAX_FUNC_HELPER_LEN];

/*
 * ========== FUNC_HELPER() ==========
 *
 * DESC
 * Populates a function helper given a function name
 *
 * ARGUMENTS
 * - char *args: String containing function arguments passed from vim-side code
 *   - structure:
 *     [<FILE_DIR>\n<TEMP_DIR>\n<FILE_NAME>\n<TOKEN>\n<ROW>\n<COL>]
 *     - FILE_DIR: Absolute directory of the file
 *     - TEMP_DIR: The absolute path of the directory where the unsaved version
 *       of the file is stored
 *     - FILE_NAME: Name of file being used for autocomplete
 *     - TOKEN: Name of function call currently being typed by the user
 *     - ROW: Current row of cursor
 *     - COL: Current column of cursor
 *
 * RETURNS
 * A char * containing containing a list of possible autocomplete options and
 * a function helper (if applicable)
 *
 * ===================================
 */
char *func_helper(char *args) {
  COMPLETE_ARGS *parsed_args = parse_complete_args(args);

  if (!parsed_args) {
    g_func_helper[0] = 'P';
    g_func_helper[1] = '_';
    g_func_helper[2] = 'E';
    g_func_helper[3] = 'R';
    g_func_helper[4] = 'R';
    g_func_helper[5] = '\0';
    return g_func_helper;
  }

  const char **include_list = malloc(INCLUDE_LIST_STARTING_LEN * sizeof(char *));
  char *cur_include_str = parsed_args->includes;
  int include_list_buffer_len = INCLUDE_LIST_STARTING_LEN;
  int include_list_len = 0;
  int pos = 0;
  while (parsed_args->includes[pos] != '\0') {
    if (parsed_args->includes[pos] == '\n') {
      parsed_args->includes[pos] = '\0';
      include_list[include_list_len] = cur_include_str;
      cur_include_str = parsed_args->includes + pos + 1;
      include_list_len++;

      if (include_list_len == include_list_buffer_len) {
        include_list = realloc(include_list, 2 * include_list_buffer_len * sizeof(char *));
        if (include_list == NULL) {
          free(parsed_args->temp_dir);
          free(parsed_args->include_dir);
          free(parsed_args);

          free(include_list);

          g_func_helper[0] = 'L';
          g_func_helper[1] = '_';
          g_func_helper[2] = 'E';
          g_func_helper[3] = 'R';
          g_func_helper[4] = 'R';
          g_func_helper[5] = '\0';
          return g_func_helper;
        }
        include_list_buffer_len *= 2;
      }
    }
    pos++;
  }

  FILE *temp_file = fopen(parsed_args->temp_dir, "r");
  if (temp_file == NULL) {
    free(parsed_args->file_dir);
    free(parsed_args->temp_dir);
    free(parsed_args);

    free(include_list);

    g_func_helper[0] = 'T';
    g_func_helper[1] = '_';
    g_func_helper[2] = 'E';
    g_func_helper[3] = 'R';
    g_func_helper[4] = 'R';
    g_func_helper[5] = '\0';
    return g_func_helper;
  }

  fseek(temp_file, 0, SEEK_END);
  long int temp_file_len = ftell(temp_file);
  fseek(temp_file, 0, SEEK_SET);
  char *temp_file_contents = malloc(temp_file_len);
  fread(temp_file_contents, temp_file_len, 1, temp_file);
  fclose(temp_file);

  struct CXUnsavedFile u_file = {
    parsed_args->file_dir,
    temp_file_contents,
    temp_file_len
  };

  CXIndex index = clang_createIndex(0, 0);

  CXTranslationUnit unit = clang_parseTranslationUnit(
      index,
      parsed_args->file_dir,
      include_list,
      include_list_len,
      &u_file,
      1,
      CXTranslationUnit_None
  );

  CXCodeCompleteResults *comp_results_obj = clang_codeCompleteAt(
      unit,
      parsed_args->file_dir,
      parsed_args->row,
      parsed_args->col,
      &u_file,
      1,
      clang_defaultCodeCompleteOptions()
  );

  if (comp_results_obj) {
    CXCompletionResult *comp_results = comp_results_obj->Results;
    unsigned int num_results = comp_results_obj->NumResults;
    clang_sortCodeCompletionResults(comp_results, num_results);

    size_t func_helper_len = 0;

    binary_search(parsed_args->token, parsed_args->token_len, comp_results,
                  num_results, &func_helper_len, func_helper_compare,
                  func_helper_add);

    g_func_helper[func_helper_len] = '\0';

    clang_disposeCodeCompleteResults(comp_results_obj);
  }

  free(parsed_args->include_dir);
  free(parsed_args->temp_dir);
  free(parsed_args);

  free(include_list);

  free(temp_file_contents);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  return g_func_helper;
}

int func_helper_compare(COMP_ARGS *args) {
  enum CXCursorKind cursor_kind = args->comp_results[args->index].CursorKind;
  if (cursor_kind != CXCursor_FunctionDecl) {
    return -1;
  }

  CXCompletionString comp_string = args->comp_results[args->index].CompletionString;
  CXString cx_chunk_text = clang_getCompletionChunkText(comp_string, 1);
  const char *chunk_text = clang_getCString(cx_chunk_text);

  int comp = strncmp(args->token, chunk_text, args->token_len);
  clang_disposeString(cx_chunk_text);

  if (comp == 0) {
    return EQUAL;
  } else if (comp < 0) {
    return LESS;
  } else {
    return GREATER;
  }
}

void func_helper_add(ADD_ARGS *args) {
  CXCompletionString comp_string = args->comp_results[args->cur_index].CompletionString;
  unsigned int num_chunks = clang_getNumCompletionChunks(comp_string);

  int adding_func = 1;
  for (int i = 1; i < num_chunks && adding_func; i++) {
    CXString cx_chunk_text = clang_getCompletionChunkText(comp_string, i);
    const char *chunk_text = clang_getCString(cx_chunk_text);
    size_t chunk_len = strlen(chunk_text);

    if ((*(args->buffer_len)) + chunk_len <= MAX_FUNC_HELPER_LEN - 4) {
      offset_strncpy(g_func_helper, chunk_text, chunk_len, *(args->buffer_len));
      (*(args->buffer_len)) += chunk_len;
    } else {
      g_func_helper[*(args->buffer_len)] = '.';
      g_func_helper[*(args->buffer_len) + 1] = '.';
      g_func_helper[*(args->buffer_len) + 2] = '.';
      adding_func = 0;
      (*(args->buffer_len)) += 3;
    }
    clang_disposeString(cx_chunk_text);
  }
}

