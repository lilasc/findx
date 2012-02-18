#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "stack.h"
#include "chktest.h"

#define MAX_PATHS 128
#define OP_TYPES 2
#define EXEC_TYPES 4

struct stack expr;
char test_args[512][32];
int option[OP_TYPES];

int check_expr(struct stack* s_expr);

const char* option_type[] = 
{
    "-mindepth",
    "-maxdepth"
};

const char* exec_type[] =
{
    "-exec",
    "-ok",
    "-print",
    "-ls"
};

//find the paths, return when the first option or test found
//if the path args is empty, set the default path cwd
int get_path(int index, char** argv, char** path)
{
    char** c_argv = argv + index;
    int c_index = index;
    char** c_path = path;

    char buffer[PATH_MAX_SIZE];

    while (*c_argv)
    {
        if ((*c_argv)[0] == '-' || (*c_argv)[0] == '(' || (*c_argv)[0] == '\\')
            break;
        else
        {
            if (strcmp(*c_argv, ".") == 0)
            {
                getcwd(buffer, PATH_MAX_SIZE);
                *c_path = (char*) malloc(sizeof(char) * strlen(buffer) + 1);
                strcpy(*c_path, buffer);
                c_argv++;
                c_index++;
            }
            else if (strcmp(*c_argv, "..") ==0)
            {
                getcwd(buffer, PATH_MAX_SIZE);
                chdir(buffer);
                chdir("..");
                getcwd(buffer, PATH_MAX_SIZE);
                *c_path = (char*) malloc(sizeof(char) * strlen(buffer) + 1);
                strcpy(*c_path, buffer);
                c_argv++;
                c_index++;
            }
            else
            {
                *c_path = (char*) malloc(sizeof(char) * strlen(*c_argv) + 1);
                strcpy(*c_path++, *c_argv++);
                c_index++;
            }
        }
    }

    if (c_index == index)
    {
        getcwd(buffer, PATH_MAX_SIZE);
        *c_path = (char*) malloc(sizeof(char) * strlen(buffer) + 1);
        strcpy(*c_path, buffer);
    }

    return c_index;
}

int get_option(int index, char** argv, int* option)
{
    int c_index = index;
    char** c_argv = argv + index;
    int i;

    while (*c_argv)
    {
        for (i = 0; i < OP_TYPES; i++)
        {
            if (strcmp(*c_argv, option_type[i]) == 0)
            {
                option[i] = atoi(*(++c_argv));
                c_index += 2;
                c_argv++;
                break;
            }
            else if (i == OP_TYPES - 1)
                return c_index;
        }
    }


    return c_index;
}

int chk_option(int depth)
{
    int b = 1;

    if (option[0] != -1)
        if (depth < option[0])
            b = 0;
    if (option[1] != -1)
        if (depth > option[1])
            b = 0;

    return b;
}

void travel_path(char* path,char* pre, int depth)
{
    char* c_path = path;

    DIR *dp;
    struct dirent *entry;
    struct stat finfo;
    char nxtpre[PATH_MAX_SIZE];
    struct stack s;
    s.index = -1;

    if ((dp = opendir(c_path)) == NULL)
    {
        fprintf(stderr, "can't open diretory: %s\n", c_path);
        return;
    }
    chdir(c_path);
    while ((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &finfo);
        if (S_ISDIR(finfo.st_mode))
        {
            if (strcmp(".", entry->d_name) == 0 ||
                strcmp("..", entry->d_name) == 0)
                continue;

            strcpy(nxtpre, pre);
            if (strcmp(nxtpre, "/") != 0)
                strcat(nxtpre, "/");
            strcat(nxtpre, entry->d_name);

            set_finfo(nxtpre, entry->d_name);
            cvt_bool_expr(&expr, &s, test_args);
            if (check_expr(&s) == 1 && chk_option(depth))
            {
                printf("%s/%s/\n", pre, entry->d_name);
            }

            travel_path(entry->d_name, nxtpre, ++depth);
        }
        else
        {
            strcpy(nxtpre, pre);
            strcat(nxtpre, "/");
            strcat(nxtpre, entry->d_name);

            set_finfo(nxtpre, entry->d_name);
            cvt_bool_expr(&expr, &s, test_args);
            if (check_expr(&s) == 1 && chk_option(depth))
            {
                printf("%s\n", nxtpre);
            }

        }
    }
    chdir("..");
    closedir(dp);
}

int get_priority(char c)
{
    switch (c)
    {
    case '#':
        return 0;
    case '|':
        return 1;
    case '&':
        return 2;
    case '!':
        return 3;
    default:
        return -1;
    }
}

int check_expr(struct stack* s_expr)
{
    struct stack s_op, s_vl;
    int i;
    int len = s_expr->index + 1;
    char* expr = s_expr->value;

    int l_bracket = 0;

    s_op.index = -1;
    s_vl.index = -1;

    push(&s_op, '#');
    for (i = 0; i < len; i++)
    {
        switch (expr[i])
        {
        case '(':
            l_bracket++;
            push(&s_op, '(');
            break;
        case '|':
            while (get_priority(top(&s_op)) >= get_priority(expr[i]))
                push(&s_vl, pop(&s_op));
            push(&s_op, expr[i]);
            break;
        case '&':
            while (get_priority(top(&s_op)) >= get_priority(expr[i]))
                push(&s_vl, pop(&s_op));
            push(&s_op, expr[i]);
            break;
        case '!':
            while (get_priority(top(&s_op)) >= get_priority(expr[i]))
                push(&s_vl, pop(&s_op));
            push(&s_op, expr[i]);
            break;
        case ')':
            if (l_bracket-- <= 0)
            {
                puts("missing left bracket");
                exit(EXIT_FAILURE);
            }
            while (top(&s_op) != '(')
                push(&s_vl, pop(&s_op));
            pop(&s_op);
            break;
        default:
            push(&s_vl, expr[i]);
            break;
        }
    }

    if (l_bracket > 0)
    {
        puts("missing right bracket");
        exit(EXIT_FAILURE);
    }

    while (top(&s_op) != '#')
        push(&s_vl, pop(&s_op));
    clear(&s_op);

    reverse(&s_vl);
    while (top(&s_vl) != -1)
    {
        switch(top(&s_vl))
        {
        case '|':
            push(&s_op, pop(&s_op) | pop(&s_op));
            pop(&s_vl);
            break;
        case '&':
            push(&s_op, pop(&s_op) & pop(&s_op));
            pop(&s_vl);
            break;
        case '!':
            push(&s_op, !pop(&s_op));
            pop(&s_vl);
        default:
            push(&s_op, pop(&s_vl));
        }
    }

    return pop(&s_op);
}

int get_test_expr(int index, char** argv, struct stack* expr, char test_args[][32])
{
    int c_index = index;
    char** c_argv = argv + index;
    int count = 0;
    //store the test expr
    char* idx;
    int t_test;

    while (*c_argv)
    {
        if (strchr(*c_argv, '(') != NULL)
        {
            push(expr, '(');
            if (strcmp(*c_argv, "(") == 0)
                c_argv++;
            else
                (*c_argv)++;
        }
        else if ((idx = strchr(*c_argv, ')')) != NULL)
        {
            push(expr, ')');
            if (strcmp(*c_argv, ")") == 0)
                c_argv++;
            else
                (*c_argv)++;
        }
        else if (strcmp(*c_argv, "-or") == 0)
        {
            push(expr, '|');
            c_argv++;
            c_index++;
        }
        else if (strcmp(*c_argv, "-and") == 0)
        {
            push(expr, '&');
            c_argv++;
            c_index++;
        }
        else if (strcmp(*c_argv, "-not") == 0)
        {
            push(expr, '!');
            c_argv++;
            c_index++;
        }
        else if ((t_test = is_test(*c_argv)) != -1)
        {
            push(expr, t_test);

            c_argv++;
            if ((idx = strchr(*c_argv, ')')) != NULL)
            {
                strncpy(test_args[count], *c_argv, idx - *c_argv);
                (*c_argv) = idx;
            }
            else
            {
                strcpy(test_args[count], *c_argv);
                c_argv++;
                c_index++;
            }
            count++;
        }
        else
        {
            puts("test parser error");
        }
    }

    return c_index;
}

int main(int argc, char** argv)
{
    char** path = (char**) malloc(sizeof(char) * MAX_PATHS); 
    int index = 1;
    int i;

    expr.index = -1;

    for (i = 0; i < OP_TYPES; i++)
        option[i] = -1;

    index = get_path(index, argv, path);
    index = get_option(index, argv, option);
    index = get_test_expr(index, argv, &expr, test_args);


    while (*path)
    {
        travel_path(*path, *path, 0);
        path++;
    }

    return EXIT_SUCCESS;
}
