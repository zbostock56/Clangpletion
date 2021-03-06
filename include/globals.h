#ifndef GLOBALS_H
#define GLOBALS_H

#define NUM_ARGS 6

#define ARG_PLUGIN 0
#define ARG_FILE 1
#define ARG_ROW 2
#define ARG_COL 3
#define ARG_WORD 4
#define ARG_CONTENT 5

#define PLUGIN_LOC g_plugin_loc
#define FILENAME g_filename
#define ROW g_row
#define COL g_col
#define WORD g_token
#define CONTENTS g_contents

#define PLUGIN_LOC_INIT_MAX 100
#define FILENAME_INIT_MAX 100
#define TOKEN_INIT_MAX 100
#define CONTENTS_INIT_MAX BUFSIZ
#define COMP_MAX 250
#define FUNC_HELP_MAX 80

extern char *g_plugin_loc;
extern size_t g_plugin_loc_max;
extern size_t g_plugin_loc_len;

extern char *g_filename;
extern size_t g_filename_max;
extern size_t g_filename_len;

extern int g_row;
extern int g_col;

extern char *g_token;
extern size_t g_token_max;
extern size_t g_token_len;

extern char *g_contents;
extern size_t g_contents_max;
extern size_t g_contents_len;

extern char recommendations[COMP_MAX];

extern char function_help[FUNC_HELP_MAX];
extern int g_help_len;

extern CXIndex g_index;
extern CXTranslationUnit g_unit;

char *free_memory(char *);
int free_allocated_memory(void);
int parse_int(char *);
int compare_str(char *, char*);
int compare_strl(char *, char *);

#endif
