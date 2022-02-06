#include <clang-c/Index.h>
#include <stdio.h>
#include <stdlib.h>
#include "clangpletion.h"

char *complete(char *location) {
  // Reserve memory on the heap for path of the plugin
  char *plugin_loc = (char *) malloc(PLUGIN_LOC_INIT_MAX);
  size_t plugin_loc_max = PLUGIN_LOC_INIT_MAX;
  size_t plugin_loc_len = 0;
  // Reserve memory on the heap for name of file that will be analyzed
  char *file = (char *) malloc(FILENAME_INIT_MAX);
  size_t file_max = FILENAME_INIT_MAX;
  size_t file_len = 0;
  // Reserve memory for the row the user is currently at
  char row[20] = "";
  // Reserve memory for the column the user is currently at
  char col[20] = "";
  // Reserve memory for the current token the user has typed
  static char wrd[100];
  // Reserve memory on the heap for the contents of the file the user is
  // currently editing
  char *contents = (char *) malloc(CONTENTS_INIT_MAX);
  size_t contents_max = CONTENTS_INIT_MAX;
  size_t contents_len = 0;

  // String that stores all the completion recommendations that will be fed
  // to the user
  static char recommendations[COMP_MAX];

  for (int i = 0; i < COMP_MAX; i++) {
    if (i < 100) {
      wrd[i] = '\0';
    }
    recommendations[i] = '\0';
  }

  char *args[] = { PLUGIN_LOC, FILENAME, row, col, WORD, CONTENTS };

  // Populate arguments from location string
  char next_char = '\0';
  int i = 0;
  int pos = 0;
  int arg = ARG_PLUGIN;

  while (arg < NUM_ARGS) {
    next_char = location[i];
    if ((next_char == '\n' && arg < ARG_CONTENT) || next_char == '\0') {
      args[arg][pos] = '\0';
      arg++;
      pos = 0;
      i++;
      continue;
    }

    // Process arguments that will be stored on the heap
    if (arg == ARG_PLUGIN || arg == ARG_FILE || arg == ARG_CONTENT) {
      args[arg][pos] = next_char;
      pos++;

      if (arg == ARG_PLUGIN) {
        plugin_loc_len++;
        if (plugin_loc_len == plugin_loc_max) {
          plugin_loc_max *= 2;
          PLUGIN_LOC = (char *) realloc(PLUGIN_LOC, plugin_loc_max);
          if (PLUGIN_LOC == NULL) {
            free((void *) FILENAME);
            free((void *) CONTENTS);
            return "FAILED TO ALLOCATE PLUGIN LOCATION";
          }
        }
      }
      else if (arg == ARG_FILE) {
        file_len++;
        if (file_len == file_max) {
          file_max *= 2;
          FILENAME = (char *) realloc(FILENAME, file_max);
          if (file == NULL) {
            free((void *) PLUGIN_LOC);
            free((void *) CONTENTS);
            return "FAILED TO ALLOCATE FILENAME";
          }
        }
      }
      else if (arg == ARG_CONTENT) {
        contents_len++;
        if (contents_len == contents_max) {
          contents_max *= 2;
          CONTENTS = (char *) realloc(CONTENTS, contents_max);
          if (CONTENTS == NULL) {
            free((void *) PLUGIN_LOC);
            free((void *) FILENAME);
            return "FAILED TO ALLOCATE FILE CONTENTS";
          }
        }
      }
    }
    // Process arguments that will be stored on the stack
    else {
      args[arg][pos] = next_char;
      pos++;
    }
    i++;
  }

  char *debug_path = (char *) malloc(plugin_loc_len + 19);
  sprintf(debug_path, "%s/src/debug_log.txt", PLUGIN_LOC);

  FILE *debug_log = fopen(debug_path, "w");
  free((void *) debug_path);
  if (debug_log == NULL) {
    free((void *) PLUGIN_LOC);
    free((void *) FILENAME);
    free((void *) CONTENTS);

    return "DEBUG FAILED";
  }

  fprintf(debug_log, "DEGUG INITIALIZED\n\n");


  fprintf(debug_log, "Plugin Location: %s\nFilename: %s\nRow: %d\nCol: %d\nWord: %s\n"
          "========== CONTENTS ==========\n%s\n=========== CONTENTS END ==========\n",
          PLUGIN_LOC, FILENAME, ROW, COL, WORD, CONTENTS);

  // Clang set-up
  CXIndex index = clang_createIndex(0, 0);

  CXTranslationUnit unit = clang_parseTranslationUnit(
    index,
    FILENAME,
    0,
    0,
    0,
    0,
    clang_defaultCodeCompleteOptions()
  );

  if (unit == 0) {
    free((void *) PLUGIN_LOC);
    free((void *) FILENAME);
    free((void *) CONTENTS);
    fclose(debug_log);
    debug_log = NULL;
    return "NULL";
  }

  struct CXUnsavedFile unsaved_file = { FILENAME, CONTENTS, contents_len };

  // Code Complete at the specified location
  CXCodeCompleteResults *comp_results = clang_codeCompleteAt(
    unit,
    FILENAME,
    ROW,
    COL,
    &unsaved_file,
    1,
    clang_defaultCodeCompleteOptions()
  );

  if (comp_results == NULL) {
    free((void *) PLUGIN_LOC);
    free((void *) FILENAME);
    free((void *) CONTENTS);
    fclose(debug_log);
    debug_log = NULL;
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
          const char *chunk_txt = clang_getCString(clang_getCompletionChunkText(comp_str, j));

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
  free((void *) PLUGIN_LOC);
  free((void *) FILENAME);
  free((void *) CONTENTS);
  fclose(debug_log);
  debug_log = NULL;
  return recommendations;
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

