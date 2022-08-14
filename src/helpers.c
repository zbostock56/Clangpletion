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
