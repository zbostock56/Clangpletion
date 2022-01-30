#include <clang-c/Index.h>
#include <stdio.h>
#include <stdlib.h>
#include "clangpletion.h"

#define FILENAME args[0]
#define ROW parse_int(args[1])
#define COL parse_int(args[2])
#define WORD args[3]
#define CONTENTS args[4]
#define COMP_MAX BUFSIZ

char *complete(char *location) {
  FILE *debug_log = fopen("../src/debug_log.txt", "w");
  if (debug_log == NULL) {
    return "DEBUG FAILED";
  }

  fprintf(debug_log, "DEGUG INITIALIZED\n\n");

  // Reserve space for file, row and column argument
  char file[100] = "";
  char row[20] = "";
  char col[20] = "";
  static char wrd[100];
  char *contents = (char *) malloc(BUFSIZ);
  size_t contents_max = BUFSIZ;
  size_t contents_len = 0;

  static char recommendations[COMP_MAX];

  for (int i = 0; i < COMP_MAX; i++) {
    if (i < 100) {
      wrd[i] = '\0';
    }
    recommendations[i] = '\0';
  }

  char *args[] = {file, row, col, wrd, contents};

  // Populate arguments from location string
  char next_char;
  int i = 0;
  int pos = 0;
  int num_args = 0;
  while ((next_char = location[i]) != '\0') {
    if (num_args < 4) {
      // Newline characters denote end of a single argument
      if (next_char == '\n') {
        args[num_args][pos] = '\0';
        num_args++;
        pos = 0;
      } else {
        args[num_args][pos] = next_char;
        pos++;
      }
    } else {
      args[num_args][pos] = next_char;
      contents_len++;
      pos++;

      if (contents_len == contents_max) {
        contents_max *= 2;
        contents = (char *) realloc(contents, contents_max);
        if (contents == NULL) {
          fclose(debug_log);
          debug_log = NULL;
          return "FAILED TO ALLOCATE FILE CONTENTS";
        }
      }
    }
    i++;
  }

  fprintf(debug_log, "Filename: %s\nRow: %s\nCol: %s\nWord: %s\n"
          "========== CONTENTS ==========\n\n%s\n\n=========== CONTENTS END ==========\n",
          args[0], args[1], args[2], args[3], args[4]);

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
    free((void *) contents);
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
    free((void *) contents);
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
  free((void *) contents);
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

