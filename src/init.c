#include <init.h>

/*
 * ========== INIT() ==========
 * DESC
 * Populates the autocomplete file's contents file
 *
 * ARGUMENTS
 * - char *args: String containing function arguments passed from vim-side code
 *   - structure:
 *     [<TEMP_DIR>\n<FILE_NAME>\n<CONTENTS>]
 *     - TEMP_DIR: Absolute path to the directory where the unsaved version of
 *       each file is stored
 *     - FILE_NAME: Name of file being used for autocomplete
 *     - CONTENTS: Contents of file being used for autocomplete
 *
 * RETURNS
 * NULL
 * ============================
 */
char *init(char *args) {
  INIT_ARGS *parsed_args = parse_init_args(args);

  /*FILE *debug = fopen("C:/Users/Jack/Documents/C/Clangpletion/plugin/debug.txt", "w");
  fprintf(debug, "%s\n", args);
  fflush(debug);
  fclose(debug);*/

  if (parsed_args == NULL) {
    return "PARSE_ERR";
  }

  FILE *file = fopen(parsed_args->temp_dir, "w");
  if (file == NULL) {
    return "FILE_ERR";
  }

  fwrite(parsed_args->contents, strlen(parsed_args->contents) + 1, 1, file);

  free(parsed_args->temp_dir);
  free(parsed_args);

  fclose(file);
  return "SUCCESS";
}


