#ifndef CHKTEST_H
#define CHKTEST_H

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include "stack.h"

#define PATH_MAX_SIZE 512

struct stat finfo;
char fname[PATH_MAX_SIZE];

int is_test(char* expr);
void set_finfo(const char* path, const char *fn);
void cvt_bool_expr(const struct stack* expr, struct stack* bool_expr, char test_args[][32]);

#endif
