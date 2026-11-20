#define _POSIX_C_SOURCE 200809L
#include "minimake.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    struct options opts;
    int res = parse_options(argc, argv, &opts);
    if (res != 0)
    {
        fprintf(stderr, "minimake: invalid argument\n");
        print_help();
        return 2;
    }
    if (opts.help)
    {
        print_help();
        return 0;
    }
    struct rule *rules = NULL;
    struct variable *vars = NULL;
    res = parse_makefile(opts.file, &rules, &vars);
    if (res != 0)
        fprintf(stderr, "minimake: parsing failed\n");
    else if (opts.print)
        pretty_print(rules, vars);
    else
        res = build_targets(argc, argv, rules, vars);
    free_rules(rules);
    free_variables(vars);
    return res;
}
