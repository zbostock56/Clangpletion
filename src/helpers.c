#include <helpers.h>

/*
 * ========== PARSE_INIT_ARGS() ==========
 *
 * DESC
 * Parses the given function argument list into a populated INIT_ARGS struct
 * for the init() function
 *
 * ARGUMENTS
 * - char *args: String containing function args to be parsed
 *   - structure:
 *     [<TEMP_DIR>\n<FILENAME>\n<CONTENTS>]
 *     - TEMP_DIR: The absolute path of the directory where the unsaved version
 *       of the file is stored
 *     - FILENAME: Name of file being used for autocomplete
 *     - CONTENTS: Contents of file being used for autocomplete
 *
 * RETURNS
 * - A pointer to a populated INIT_ARGS struct
 *   - structure:
 *     { char *temp_dir, char *contents}
 *     - temp_dir: (MUST BE FREED) Path where file representing file being
 *       analyzed will be created
 *     - contents: Contents of the file being analyzed
 *   - MUST BE FREED:
 *     - parsed_args
 *     - parsed_args->temp_dir
 * - NULL if unsuccessful
 *
 * ===========================================
 */
INIT_ARGS *parse_init_args(char *args) {
  INIT_ARGS *parsed_args = malloc(sizeof(INIT_ARGS));

  size_t pos = 0;

  char *temp_dir = parse_arg(args, &pos);
  if (temp_dir == NULL) {
    free(parsed_args);
    return NULL;
  }

  char *filename = parse_arg(args, &pos);
  if (filename == NULL) {
    free(parsed_args);
    return NULL;
  }

  parsed_args->temp_dir = malloc(strlen(temp_dir) + strlen(filename) + 4);
  if (parsed_args->temp_dir == NULL) {
    free(parsed_args);
    return NULL;
  }
  sprintf(parsed_args->temp_dir, "%s/u_%s", temp_dir, filename);

  parsed_args->contents = args + pos;

  return parsed_args;
}

/*
 * ========== PARSE_COMPLETE_ARGS() ==========
 *
 * DESC
 * Parses the given function argument list into a COMPLETE_ARGS struct
 * for the code_complete() function
 *
 * ARGUMENTS
 * - char *args: String containing function args to be parsed
 *   - structure:
 *     [<FILE_DIR>\n<TEMP_DIR>\n<FILE_NAME>\n<TOKEN>\n<ROW>\n<COL>
 *      \n<INCLUDE_1>\n<INCLUDE_2>\n...<INCLUDE_N>]
 *     - FILE_DIR: Absolute directory of the file
 *     - TEMP_DIR: The absolute path of the directory where the unsaved version
 *       of the file is stored
 *     - FILE_NAME: Name of file being used for autocomplete
 *     - TOKEN: Token currently being typed by user
 *     - ROW: Current row of cursor
 *     - COL: Current column of cursor
 *     - INCLUDE_N: Path of include path to use in code completion
 *
 * RETURNS
 * - A pointer (MUST BE FREED) to a populated COMPLETE_ARGS struct if successful
 *   - structure:
 *     { char *file_dir, char *temp_dir, char *token, int token_len,
 *       int row, int col }
 *     - char *file_dir: String of absolute path to the file being manipulated
 *       by the user
 *     - char *temp_dir: (MUST BE FREED) String of absolute path to the file to
 *       be analyzed by the autocomplete engine
 *     - char *include_dir: (MUST BE FREED) String of absolute path to file
 *       containing the list of include paths that will be used when compiling
 *       the current file
 *     - char *token: String of the token currently being typed by the user
 *     - int token_len: Length of token string
 *     - int row: Row where the autocompletion will be done
 *     - int col: column where the autocompletion will be done
 *     - char *includes: array of include paths to be used in
 *       code completion
 *   - MUST BE FREED:
 *     - parsed_args
 *     - parsed_args->temp_dir
 *     - parsed_args->include_dir
 * - NULL if unsuccessful
 *
 * ===========================================
 */
COMPLETE_ARGS *parse_complete_args(char *args) {
  COMPLETE_ARGS *parsed_args = malloc(sizeof(COMPLETE_ARGS));
  if (parsed_args == NULL) {
    return NULL;
  }

  size_t pos = 0;

  parsed_args->file_dir = parse_arg(args, &pos);
  if (parsed_args->file_dir == NULL) {
    free(parsed_args);
    return NULL;
  }

  char *temp_dir = parse_arg(args, &pos);
  if (temp_dir == NULL) {
    free(parsed_args);
    return NULL;
  }

  char *filename = parse_arg(args, &pos);
  if (filename == NULL) {
    free(parsed_args);
    return NULL;
  }

  parsed_args->temp_dir = malloc(strlen(temp_dir) + strlen(filename) + 4);
  if (parsed_args->temp_dir == NULL) {
    free(parsed_args);
    return NULL;
  }
  sprintf(parsed_args->temp_dir, "%s/u_%s", temp_dir, filename);

  parsed_args->include_dir = malloc(strlen(temp_dir) + strlen(filename) + 4);
  if (parsed_args->include_dir == NULL) {
    free(parsed_args->temp_dir);
    free(parsed_args);
    return NULL;
  }
  sprintf(parsed_args->include_dir, "%s/i_%s", temp_dir, filename);

  parsed_args->token = parse_arg(args, &pos);
  if (parsed_args->token == NULL) {
    free(parsed_args->temp_dir);
    free(parsed_args->include_dir);
    free(parsed_args);
    return NULL;
  }

  parsed_args->token_len = strlen(parsed_args->token);

  char *row_buffer = parse_arg(args, &pos);
  if (row_buffer == NULL) {
    free(parsed_args->temp_dir);
    free(parsed_args->include_dir);
    free(parsed_args);
    return NULL;
  }

  parsed_args->row = atoi(row_buffer);

  char *col_buffer = parse_arg(args, &pos);
  if(col_buffer == NULL) {
    free(parsed_args->temp_dir);
    free(parsed_args->include_dir);
    free(parsed_args);
    return NULL;
  }

  parsed_args->col = atoi(col_buffer);

  parsed_args->includes = args + pos;

  return parsed_args;
}

/*
 * ========== PARSE_ARG() ==========
 *
 * DESC
 * Parse a portion of the source string into a parsed string
 *
 * ARGS
 * - const char *source: Buffer being parsed
 * - size_t start_size: Size the parsed buffer will be initially set to
 * - size_t *offset: Reference to representation of the position in source to
 *                   begin parsing
 *
 * RETURNS
 * - A populated parsing of source if successful
 * =================================
 */
char *parse_arg(char *source, size_t *offset) {
  char *parsed_arg = source + *offset;

  char next = source[*offset];

  while (next != '\n') {
    (*offset)++;
    next = source[*offset];
  }
  source[*offset] = '\0';
  (*offset)++;

  return parsed_arg;
}

/*
 * ========== OFFSET_STRNCPY() =========
 *
 * DESC: Functionality of strncpy() but the
 * source is copied into the destination at
 * a given offset from the beginning of the
 * destination buffer.
 *
 * ARGUMENTS:
 * - char *dest: destination string
 * - char *src: source string
 * - size_t num: number of characters to
 *   copy from srce
 * - size_t offset: offset from the
 *   beginning of dest to copy src into
 *
 * RETURNS:
 * Pointer to destination
 * =====================================
 */
char *offset_strncpy(char *dest, const char *src, size_t num, size_t offset) {
  for (int i = 0; i < num; i++) {
    dest[offset + i] = src[i];
  }

  return dest;
}

/*
 * ========== BEGINS_WITH() ==========
 * DESC: Checks if a given string begins
 * with a given token.
 *
 * ARGUMENTS:
 * - char *str: String which will be
 *   tested.
 * - char *token: token compared to
 *   the beginning of str
 *
 * RETURNS:
 * 0 if str begins with token, 1 if not
 */
int begins_with(const char *str, const char *token) {
  unsigned int pos = 0;
  while (token[pos] != '\0') {
    if (token[pos] != str[pos]) {
      return 1;
    }
    pos++;
  }

  return 0;
}

/*
 * ========== GET_TYPED_TEXT() ==========
 * DESC: Returns the typed-text portion for
 * a given code completion string
 *
 * ARGUMENTS:
 * - CXCompletionString comp_string: pointer
 *   to code completion string
 *
 * RETURNS:
 * Index of CXString corresponding to the
 * typed text chunk
 * ======================================
 */
int get_typed_text(CXCompletionString *comp_string) {
  unsigned int numChunks = clang_getNumCompletionChunks(*comp_string);

  for (int i = 0; i < numChunks; i++) {
    if (clang_getCompletionChunkKind(*comp_string, i) == CXCompletionChunk_TypedText) {
      return i;
    }
  }

  return -1;
}

/*
 * ========== BINARY_SEARCH() ==========
 * DESC: Performs binary search on a completion result,
 * utilizing the given comparison function and add function.
 *
 * ARGUMENTS:
 * - char *token: Token being searched for in completion list
 * - int token_len: length of given token
 * - unsigned int num_results: Number of code completion
 *   results
 * - size_t *buffer_len: Pointer to the length of the buffer
 *   that will be populated by the algorithm
 * - int comp_func(COMP_ARGS *args): Comparison function used
 *   by the algorithm to determine if the given token has been
 *   found. Must return:
 *   - EQUAL
 *   - LESS
 *   - GREATER
 *   - -1 (if current entry in code completion list is invalid for
 *         comparison)
 * - void add_func(ADD_ARGS *args): Function to be used once
 *   comparison returns EQUAL
 *
 * =====================================
 */
void binary_search(char *token, int token_len,
                   CXCompletionResult *comp_results,
                   unsigned int num_results, size_t *buffer_len,
                   int comp_func(COMP_ARGS *args),
                   void add_func(ADD_ARGS *args)) {
  int adding_completions = 1;

  unsigned int median = num_results / 2;
  unsigned int cur_index = median;

  unsigned int left_bound = 0;
  unsigned int right_bound = num_results - 1;

  while (adding_completions && left_bound < right_bound) {
    COMP_ARGS comp_args;
    comp_args.token = token;
    comp_args.token_len = token_len;
    comp_args.comp_results = comp_results;
    comp_args.index = cur_index;
    int comp = comp_func(&comp_args);

    if (comp == EQUAL) {
      ADD_ARGS add_args;
      add_args.token = token;
      add_args.comp_results = comp_results;
      add_args.num_results = num_results;
      add_args.cur_index = cur_index;
      add_args.buffer_len = buffer_len;
      add_func(&add_args);
      adding_completions = 0;
    } else if (comp == LESS) {
      // Token occurs before chunk
      if (cur_index <= median) {
        //If the current index is before the median, the new right bound is
        //defined by the current index, instead of the median to prevent
        //reanalyzing invalid entries
        right_bound = cur_index - 1;
      } else {
        right_bound = median - 1;
      }
      median = (left_bound + cur_index) / 2;
      cur_index = median;
    } else if (comp == GREATER) {
      // Token occurs after chunk
      if (cur_index >= median) {
        //If the current index is after the median, the new left bound is
        //defined by the current index, instead of the median to prevent
        //reanalyzing invalid entries
        left_bound = cur_index + 1;
      } else {
        //If the current index is before the median, all entries left of the
        //median are invalid entries. Therefore, there is nothing to analyze
        //further and the token does not exist.
        adding_completions = 0;
      }
      median = (cur_index + right_bound) / 2;
      cur_index = median;
    } else {
      //The comparison failed for the current completion result. Goal is now to
      //move the current index to an index inside the current partition that
      //is a valid completion result

      if (cur_index == right_bound) {
        //If the current index is the right bound, the entire right side of
        //the partition has invalid text. Now start looking to the left side.
        cur_index = median - 1;
      } else if (cur_index == left_bound) {
        //If the current index is the left bound, no values in the partition have
        //valid typed text. Therefore, there is no completions to add.
        adding_completions = 0;
      } else if (cur_index < median) {
        // Continue looking for valid entries on the left side
        cur_index--;
      } else {
        // Continue looking for valid entries on the right side
        cur_index++;
      }
    }
  }
}


