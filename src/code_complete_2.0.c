#include <code_complete_2.0.h>

char g_completion_list[MAX_COMPLETION_LIST_LEN];

/*
 * ========== CODE_COMPLETE() ==========
 *
 * DESC
 * Returns a code completion list for a specific file at a specific locationi,
 * based on the token currently being typed by the user.
 *
 * ARGUMENTS
 * - char *args: String containing function arguments passed from vim-side code
 *   - structure:
 *     [<FILE_DIR>\n<TEMP_DIR>\n<FILE_NAME>\n<TOKEN>\n<ROW>\n<COL>]
 *     - FILE_DIR: Absolute directory of the file
 *     - TEMP_DIR: The absolute path of the directory where the unsaved version
 *       of the file is stored
 *     - FILE_NAME: Name of file being used for autocomplete
 *     - TOKEN: Value currently being typed by the user
 *     - ROW: Current row of cursor
 *     - COL: Current column of cursor
 *
 * RETURNS
 * A char * containing containing a list of possible autocomplete options
 * - structure:
 *   [<OPTION 1>\n<OPTION 2>\n...]
 *
 * =====================================
 */
char *code_complete(char *args) {
  COMPLETE_ARGS *parsed_args = parse_complete_args(args);

  if (!parsed_args) {
    g_completion_list[0] = 'P';
    g_completion_list[1] = '_';
    g_completion_list[2] = 'E';
    g_completion_list[3] = 'R';
    g_completion_list[4] = 'R';
    g_completion_list[5] = '\0';
    return g_completion_list;
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

          g_completion_list[0] = 'L';
          g_completion_list[1] = '_';
          g_completion_list[2] = 'E';
          g_completion_list[3] = 'R';
          g_completion_list[4] = 'R';
          g_completion_list[5] = '\0';
          return g_completion_list;
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

    g_completion_list[0] = 'T';
    g_completion_list[1] = '_';
    g_completion_list[2] = 'E';
    g_completion_list[3] = 'R';
    g_completion_list[4] = 'R';
    g_completion_list[5] = '\0';
    return g_completion_list;
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

    size_t completion_list_len = 0;

    if (num_results < RESULT_THRESHOLD) {
      // Linear search
      linear_search(parsed_args->token, comp_results, num_results, &completion_list_len);
    } else {
      // Binary search
      binary_search(parsed_args->token, parsed_args->token_len, comp_results,
                    num_results, &completion_list_len, comp_list_compare,
                    comp_list_add);
    }
    g_completion_list[completion_list_len] = '\0';

    clang_disposeCodeCompleteResults(comp_results_obj);
    /*unsigned int num_results = comp_results_obj->NumResults;
    sprintf(g_completion_list, "%d", num_results);*/
  }

  free(parsed_args->include_dir);
  free(parsed_args->temp_dir);
  free(parsed_args);

  free(include_list);

  free(temp_file_contents);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  return g_completion_list;
}

int comp_list_compare(COMP_ARGS *args) {
  CXCompletionString comp_string = args->comp_results[args->index].CompletionString;

  int typed_text_pos = get_typed_text(&comp_string);
  if (typed_text_pos == -1) {
    return -1;
  }

  CXString cx_chunk_text = clang_getCompletionChunkText(comp_string, typed_text_pos);
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

void comp_list_add(ADD_ARGS *args) {
  int buffer_full = 0;

  // Above
  unsigned int above = args->cur_index - 1;
  int adding = 1;
  while (buffer_full == 0 && above >= 0 && adding) {
    CXCompletionString comp_string = args->comp_results[above].CompletionString;
    int typed_text_pos = get_typed_text(&comp_string);
    if (typed_text_pos != -1) {
      CXString cx_chunk_text = clang_getCompletionChunkText(comp_string,
                               typed_text_pos);
      const char *chunk_text = clang_getCString(cx_chunk_text);

      if (begins_with(chunk_text, args->token) == 0) {
        size_t chunk_len = strlen(chunk_text);

        if (*(args->buffer_len) + chunk_len + 1 <= MAX_COMPLETION_LIST_LEN - 1) {
          offset_strncpy(g_completion_list, chunk_text, chunk_len, *(args->buffer_len));
          g_completion_list[*(args->buffer_len) + chunk_len] = '\n';
          (*(args->buffer_len)) += (chunk_len + 1);
        } else {
          buffer_full = 1;
        }
      } else {
        adding = 0;
      }

      clang_disposeString(cx_chunk_text);
    }
    above--;
  }

  adding = 1;
  while (buffer_full == 0 && args->cur_index < args->num_results && adding) {
    CXCompletionString comp_string = args->comp_results[args->cur_index].CompletionString;
    int typed_text_pos = get_typed_text(&comp_string);
    if (typed_text_pos != -1) {
      CXString cx_chunk_text = clang_getCompletionChunkText(comp_string,
                                                            typed_text_pos);
      const char *chunk_text = clang_getCString(cx_chunk_text);

      if (begins_with(chunk_text, args->token) == 0) {
        size_t chunk_len = strlen(chunk_text);

        if (*(args->buffer_len) + chunk_len + 1 <= MAX_COMPLETION_LIST_LEN - 1) {
          offset_strncpy(g_completion_list, chunk_text, chunk_len, *(args->buffer_len));
          g_completion_list[*(args->buffer_len) + chunk_len] = '\n';
          (*(args->buffer_len)) += (chunk_len + 1);
        } else {
          buffer_full = 1;
        }
      } else {
        adding = 0;
      }

      clang_disposeString(cx_chunk_text);
    }
    (args->cur_index)++;
  }
}

void linear_search(char *token, CXCompletionResult *comp_results,
                unsigned int num_results, size_t *comp_list_len) {
  int adding_completions = 1;
  unsigned int cur_result = 0;
  while (adding_completions && cur_result < num_results) {
    CXCompletionString comp_string = comp_results[cur_result].CompletionString;
    //unsigned int numChunks = clang_getNumCompletionChunks(comp_string);
    int typed_text_pos = get_typed_text(&comp_string);
    if (typed_text_pos != -1) {
      CXString cx_chunk_text = clang_getCompletionChunkText(comp_string, typed_text_pos);
      const char *chunk_text = clang_getCString(cx_chunk_text);

      /*if (numChunks > 0) {
        unsigned int cur_chunk = 0;
        enum CXCompletionChunkKind cx_chunk_kind = clang_getCompletionChunkKind(comp_string,
            cur_chunk);
        while (cx_chunk_kind != CXCompletionChunk_TypedText && cur_chunk < numChunks) {
          cur_chunk++;
          cx_chunk_kind = clang_getCompletionChunkKind(comp_string, cur_chunk);
        }

        if (cx_chunk_kind == CXCompletionChunk_TypedText) {
          CXString cx_chunk_text = clang_getCompletionChunkText(comp_string, cur_chunk);
          const char *chunk_text = clang_getCString(cx_chunk_text);
      */

      if (begins_with(chunk_text, token) == 0) {
        size_t chunk_len = strlen(chunk_text);

        if (*comp_list_len + chunk_len + 1 <= MAX_COMPLETION_LIST_LEN - 1) {
          offset_strncpy(g_completion_list, chunk_text, chunk_len, *comp_list_len);
          g_completion_list[*comp_list_len + chunk_len] = '\n';
          (*comp_list_len) += (chunk_len + 1);
        } else {
          adding_completions = 0;
        }
      }

      clang_disposeString(cx_chunk_text);
    }

    cur_result++;
  }
}
