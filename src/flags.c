#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>

#include "minimake.h"

void print_help(void)
{
    printf("Usage: minimake [options] [targets]\n");
    printf("  -f FILE, --file=FILE, --makefile=FILE\n");
    printf("  -h, --help\n");
    printf("  -p, --pretty-print, --print-data-base\n");
}

int parse_options(int argc, char **argv, struct options *o)
{
    o->file = "Makefile";
    o->print = 0;
    o->help = 0;
    int unknown = 0;
    for (int i = 1; i < argc; i++)
        if (argv[i][0] == '\0')
            return 2;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0)
        {
            if (i + 1 >= argc)
                return 2;
            o->file = argv[++i];
        }
        else if (strncmp(argv[i], "--file=", 7) == 0)
            o->file = argv[i] + 7;
        else if (strncmp(argv[i], "--makefile=", 11) == 0)
            o->file = argv[i] + 11;
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            o->help = 1;
        else if (strcmp(argv[i], "-p") == 0
                 || strcmp(argv[i], "--pretty-print") == 0
                 || strcmp(argv[i], "--print-data-base") == 0)
            o->print = 1;
        else if (argv[i][0] == '-')
            unknown = 1;
    }
    if (o->file[0] == '\0')
        return 2;
    if (unknown && !o->help)
        return 2;
    return 0;
}

void pretty_print(struct rule *rules, struct variable *vars)
{
    printf("# variables\n");
    while (vars != NULL)
    {
        printf("'%s' = '%s'\n", vars->name, vars->value);
        vars = vars->next;
    }
    printf("# rules\n");
    while (rules != NULL)
    {
        printf("(%s):", rules->target);
        struct dependency *dep = rules->deps;
        while (dep != NULL)
        {
            printf(" [%s]", dep->name);
            dep = dep->next;
        }
        printf("\n");
        struct command *cmd = rules->commands;
        while (cmd != NULL)
        {
            printf("    '%s'\n", cmd->text);
            cmd = cmd->next;
        }
        rules = rules->next;
    }
}
