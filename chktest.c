#include "chktest.h"
#include <stdlib.h>
#include <time.h>
#include <fnmatch.h>

// insert one comment
const char* test_type[] =
{
    "-atime",
    "-mtime",
    "-name",
    "-newer",
    "-type",
};

int is_test(char* expr)
{
    const char** test_expr = test_type;
    int i = 0;

    while (*test_expr)
    {
        if (strcmp(*test_expr++, expr) == 0)
            return i;
        i++;
    }

    return -1;
}

void set_finfo(const char* path, const char *fn)
{
    strcpy(fname, fn);
    lstat(path, &finfo);
}

int chk_atime(int diffdays)
{
    time_t now = time(NULL);
    double diff = difftime(now, finfo.st_atime);
    if (diff > diffdays * 3600 * 24 && diff < (diffdays + 1) * 3600 * 24)
        return 1;

    return 0;
}

int chk_mtime(int diffdays)
{
    time_t now = time(NULL);
    double diff = difftime(now, finfo.st_mtime);
    if (diff > diffdays * 3600 * 24 && diff < (diffdays + 1) * 3600 * 24)
        return 1;

    return 0;
}

int chk_name(const char* pattern)
{
    return !fnmatch(pattern, fname, FNM_PATHNAME);
}

int chk_newer(const char* path)
{
    struct stat buf;
    lstat(path, &buf);

    if (finfo.st_ctime > buf.st_ctime)
        return 1;

    return 0;
}

int chk_type(const char* type)
{
    switch (finfo.st_mode & S_IFMT) {
    case S_IFBLK:
        if (strcmp(type, "b") == 0)
            return 1;
        break;
    case S_IFCHR:
        if (strcmp(type, "c") == 0)
            return 1;
        break;
    case S_IFDIR:
        if (strcmp(type, "d") == 0)
            return 1;
        break;
    case S_IFIFO:
        if (strcmp(type, "i") == 0)
            return 1;
        break;
    case S_IFLNK:
        if (strcmp(type, "l") == 0)
            return 1;
        break;
    case S_IFREG:
        if (strcmp(type, "f") == 0)
            return 1;
        break;
    case S_IFSOCK:
        if (strcmp(type, "s") == 0)
           return 1;
        break;
    default:
        return 0;
     }

    return 0;
}

//use char* type instead the index of the type to decrease the couple effect
int chk_test(const char* type, const char* args)
{
    if (strcmp("-atime", type) == 0)
        return chk_atime(atoi(args));
    if (strcmp("-mtime", type) == 0)
        return chk_mtime(atoi(args));
    if (strcmp("-name", type) == 0)
        return chk_name(args);
    if (strcmp("-newer", type) == 0)
        return chk_newer(args);
    if (strcmp("-type", type) == 0)
        return chk_type(args);

    return -1;
}

void cvt_bool_expr(const struct stack* expr, struct stack* bool_expr, char test_args[][32])
{
    struct stack c_expr = *expr;
    char c;
    int count = 0;

    clear(bool_expr);
    reverse(&c_expr);

    while (top(&c_expr) != -1)
    {
        switch (c = pop(&c_expr))
        {
        case '(':
        case ')':
        case '|':
        case '&':
        case '!':
            push(bool_expr, c);
            break;
        default:
            push(bool_expr, chk_test(test_type[(int)c], test_args[count++]));
            break;
        }
    }
}
