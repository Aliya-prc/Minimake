#ifndef MINIMAKE_H
#define MINIMAKE_H

struct dependency
{
    char *name;
    struct dependency *next;
}; // les dependances du coup

struct command
{
    char *text;
    struct command *next;
}; // chaque command

struct rule
{
    char *target;
    struct dependency *deps;
    struct command *commands;
    struct rule *next;
}; // et chaque regle du makefile

struct variable
{
    char *name;
    char *value;
    struct variable *next;
}; // chaque variable

struct options
{
    const char *file;
    int print;
    int help;
}; // pour les flags

struct rule *parse_rule(char *line, struct variable *variables);
int parse_makefile(const char *filename, struct rule **rules,
                   struct variable **variables);
char *expand_variables(char *text, struct variable *variables);
void free_rules(struct rule *rules);
void free_variables(struct variable *vars);

int parse_options(int argc, char **argv, struct options *o);
void print_help(void);
void pretty_print(struct rule *rules, struct variable *vars);

int build_targets(int argc, char **argv, struct rule *rules,
                  struct variable *vars);

#endif // MINIMAKE_H
