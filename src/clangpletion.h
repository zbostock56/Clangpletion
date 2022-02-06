#ifndef CLANGPLETION_H
#define CLANGPLETION_H

#define NUM_ARGS 6

#define ARG_PLUGIN 0
#define ARG_FILE 1
#define ARG_ROW 2
#define ARG_COL 3
#define ARG_WORD 4
#define ARG_CONTENT 5

#define PLUGIN_LOC plugin_loc
#define FILENAME file
#define ROW parse_int(row)
#define COL parse_int(col)
#define WORD wrd
#define CONTENTS contents

#define PLUGIN_LOC_INIT_MAX 100
#define FILENAME_INIT_MAX 100
#define CONTENTS_INIT_MAX BUFSIZ
#define COMP_MAX BUFSIZ


char *complete(char *);
int parse_int(char *);
int compare_str(char *, char*);

#endif
