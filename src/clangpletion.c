#include <clang-c/Index.h>
#include <stdio.h>
#include "clangpletion.h"

#define FILENAME args[0]
#define ROW parse_int(args[1])
#define COL parse_int(args[2])
#define WORD args[3] 
#define COMP_MAX BUFSIZ  

/*
int main() {
  printf("%s\n", complete("complete.c\n12\n11")); 
}
*/
char *complete(char *location) {
  // Reserve space for file, row and column argument
  char file[100];
  char row[20];
  char col[20];
  static char wrd[100];
  static char recommendations[COMP_MAX];

  for (int i = 0; i < BUFSIZ; i++) {
    if (i < 100) {
      wrd[i] = '\0';
    }
    recommendations[i] = '\0';  
  }
  

  char *args[] = {file, row, col, wrd};

  // Populate arguments from location string
  char next_char;
  int i = 0;
  int pos = 0;
  int num_args = 0;
  while ((next_char = location[i]) != '\0') {
    // Newline characters denote end of a single argument
    if (next_char == '\n') {
      args[num_args][pos] = '\0';
      num_args++;
      pos = 0;
    } else {
      args[num_args][pos] = next_char;
      pos++;
    }
    i++;
  }
 
  //printf("Filename: %s\nRow: %s\nCol: %s\n", args[0], args[1], args[2]);

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
    //printf("Unit Broke\n");
    return "NULL";
  }
    
  // Code Complete at the specified location
  CXCodeCompleteResults *comp_results = clang_codeCompleteAt(
    unit,
    FILENAME,
    ROW,
    COL,
    0,
    0,
    clang_defaultCodeCompleteOptions()
  );

  if (comp_results == NULL) {
    //printf("Results Broke\n");
    return "Null";
  }

  //CXFile c_file = clang_getFile(unit, FILENAME);

  //if (c_file == NULL) {
  //  return "C_File Null\n";
  //}

  //CXSourceLocation src_loc = clang_getLocation(unit, c_file, ROW, COL);

  //CXToken *token = clang_getToken(unit, src_loc);

  //if (token == NULL) {
    //return WORD;
  //}

  //CXString tok_str = clang_getTokenSpelling(unit, *token);

  /* 
     A single recommendations string that will be filled with individual 
     code-completion recomendations, with reccommendations being separated 
     by a '\n' character
  */
  int position = 0;
  //char *rec_pos = recommendations;

  /* From the array of results provided by comp_results, populate the
     completion string with reccommendations */
  int num_results = comp_results->NumResults;
 
  /*unsigned int isIncomplete;

  enum CXCursorKind kind = clang_codeCompleteGetContainerKind(comp_results, &isIncomplete);
  int length = sprintf(rec_pos, "kind: %d\n", kind);
  rec_pos += length;
  
  length = sprintf( rec_pos, "isIncomplete: %d\n", isIncomplete);
  rec_pos += length;

  CXString usr = clang_codeCompleteGetContainerUSR(comp_results);
  length = sprintf(rec_pos, "usr: %s\n", clang_getCString(usr));
  rec_pos += length;

  return recommendations;
  */
  /*for (int j = 0; j < num_results && position != -1; j++) {
    CXCompletionResult res = results[j];
    char res_type[4];
    sprintf(res_type, "%d\n", res.CursorKind);  
    
    if (position + 5 <= COMP_MAX) {
      char res_next;
      int k = 0;
      while ((res_next = res_type[k]) != '\0') {
        recommendations[k] = res_next; 
        k++;
        position++;
      }
      recommendations[k] = '\n';
    } else {
      position = -1;
    }
  }

  return recommendations;
  */
  int result = 0;
  int adding = 1;
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


