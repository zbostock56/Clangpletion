#ifndef FUNC_HELPER_H
#define FUNC_HELPER_H

char *function_helper(char *);

enum CXChildVisitResult visitor(CXCursor, CXCursor, CXClientData);
int populate_function_helper_args(char *);
int gen_help_header(char *);
//int gen_help_arg(char *, char *);
int gen_help_arg(char *);

#endif
