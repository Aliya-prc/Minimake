#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minimake.h"

// rajoute une regle
static void add_rule(struct rule **rules, struct rule *new)
{
    if (*rules == NULL)
    {
        *rules = new;
        return;
    }
    struct rule *tmp = *rules;
    while (tmp->next != NULL)
    {
        tmp = tmp->next;
    }
    tmp->next = new;
}

// enleve les espace en trop
static char *trim(char *str)
{
    int i = 0;
    while (str[i] == ' ' || str[i] == '\t')
        i++;
    char *res = str + i;
    size_t j = strlen(res);
    while (j > 0
           && (res[j - 1] == ' ' || res[j - 1] == '\t' || res[j - 1] == '\n'))
        j--;
    res[j] = '\0';
    return res;
}
// enleve espace que de gauche
static char *trim_left(char *str)
{
    int i = 0;
    while (str[i] == ' ' || str[i] == '\t')
        i++;
    char *res = str + i;
    return res;
}

// verifie que le nom est bien okay
static int valid_name(const char *name)
{
    if (name == NULL || name[0] == '\0')
        return 0;
    for (size_t i = 0; name[i] != '\0'; i++)
    {
        if (name[i] == ':' || name[i] == '=' || name[i] == '#' || name[i] == ' '
            || name[i] == '\t')
        {
            return 0;
        }
    }
    return 1;
}

// enregistre les dependances
static int parse_dependencies(struct rule *rule, char *text)
{
    struct dependency *last = NULL;
    char *dep = strtok(text, " \t\n");
    while (dep != NULL)
    {
        if (!valid_name(dep))
            return 1;
        struct dependency *new = malloc(sizeof(*new));
        if (!new)
            return 1;
        new->name = strdup(dep);
        if (new->name == NULL)
        {
            free(new);
            return 1;
        }
        new->next = NULL;
        if (last == NULL)
            rule->deps = new;
        else
            last->next = new;
        last = new;
        dep = strtok(NULL, " \t\n");
    }
    return 0;
}

static void free_dependencies(struct dependency *dep)
{
    while (dep != NULL)
    {
        struct dependency *next = dep->next;
        free(dep->name);
        free(dep);
        dep = next;
    }
}

// sert a stocker les commandes
static int parse_recipe(char *line, struct rule *last)
{
    if (last == NULL)
        return 2;
    line = trim_left(line + 1);
    size_t l = strlen(line);
    if (l > 0 && line[l - 1] == '\n')
        line[l - 1] = '\0';
    struct command *new = malloc(sizeof(struct command));
    if (!new)
        return 2;
    new->text = strdup(line);
    if (new->text == NULL)
    {
        free(new);
        return 2;
    }
    new->next = NULL;
    if (last->commands == NULL)
    {
        last->commands = new;
    }
    else
    {
        struct command *temp = last->commands;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = new;
    }
    return 0;
}

// recupere le nom apres $ + cherche sa val
static char *get_variable_val(char *text, size_t *i, struct variable *vars)
{
    size_t start = *i;
    size_t len = 0;
    char end = '\0';
    if (text[*i] == '(' || text[*i] == '{')
    {
        if (text[*i] == '(')
            end = ')';
        else
            end = '}';
        (*i)++;
        start = *i;
        while (text[*i] != '\0' && text[*i] != end)
            (*i)++;
        if (text[*i] != end)
            return NULL;
        len = *i - start;
        (*i)++;
    }
    else
    {
        if (text[*i] == '\0')
            return NULL;
        len = 1;
        (*i)++;
    }
    char *name = strndup(text + start, len);
    if (name == NULL)
        return NULL;
    char *val = "";
    while (vars != NULL)
    {
        if (strcmp(vars->name, name) == 0)
        {
            val = vars->value;
        }
        vars = vars->next;
    }
    free(name);
    return val;
}

// parcours text+construit nouvelle chaine+$
char *expand_variables(char *text, struct variable *variables)
{
    size_t size = strlen(text) + 1;
    char *res = malloc(size);
    if (!res)
        return NULL;
    size_t i = 0;
    size_t j = 0;
    while (text[i] != '\0')
    {
        if (text[i] != '$')
        {
            res[j++] = text[i++];
            continue;
        }
        i++;
        if (text[i] == '\0')
        {
            res[j++] = '$';
            break;
        }
        if (text[i] == '$')
        {
            res[j++] = '$';
            i++;
            continue;
        }
        char *val = get_variable_val(text, &i, variables);
        if (val == NULL)
        {
            free(res);
            return NULL;
        }
        size_t len = strlen(val);
        size_t n = j + len + strlen(text + i) + 1;
        if (n > size)
        {
            char *tmp = realloc(res, n);
            if (tmp == NULL)
            {
                free(res);
                return NULL;
            }
            res = tmp;
            size = n;
        }
        memcpy(res + j, val, len);
        j += len;
    }
    res[j] = '\0';
    return res;
}

// stocker nom + valeur
static int parse_variable(char *line, struct variable **variables)
{
    char *sep = strchr(line, '=');
    if (sep == NULL)
        return 2;
    *sep = '\0';
    char *name = trim(line); // garder espace de fin ?
    char *value = trim_left(sep + 1);
    size_t len = strlen(value);
    if (len > 0 && value[len - 1] == '\n')
        value[len - 1] = '\0';
    struct variable *new = malloc(sizeof(*new));
    if (!new)
        return 2;
    new->name = expand_variables(name, *variables);
    new->value = strdup(value);
    if (!new->name || !new->value)
    {
        free(new->name);
        free(new->value);
        free(new);
        return 2;
    }
    new->next = NULL;
    if (!valid_name(new->name))
    {
        free(new->name);
        free(new->value);
        free(new);
        return 2;
    }
    if (*variables == NULL)
        *variables = new;
    else
    {
        struct variable *tmp = *variables;
        while (tmp->next != NULL)
        {
            tmp = tmp->next;
        }
        tmp->next = new;
    }
    return 0;
}

// creer regles
struct rule *parse_rule(char *line, struct variable *variables)
{
    char *sep = strchr(line, ':');
    if (sep == NULL)
        return NULL;
    *sep = '\0';
    char *target = trim(line);
    char *deps = sep + 1;
    char *new_t = expand_variables(target, variables);
    char *new_d = expand_variables(deps, variables);
    if (new_t == NULL || new_d == NULL || !valid_name(new_t))
    {
        free(new_t);
        free(new_d);
        return NULL;
    }
    struct rule *r = malloc(sizeof(*r));
    if (!r)
    {
        free(new_t);
        free(new_d);
        return NULL;
    }
    r->deps = NULL;
    r->commands = NULL;
    r->next = NULL;
    r->target = new_t; // copie sans soucis
    int ret = parse_dependencies(r, new_d);
    free(new_d);
    if (ret != 0)
    {
        free_dependencies(r->deps);
        free(r->target);
        free(r);
        return NULL;
    }
    return r;
} // faire cible vide

static int parse_line(char *line, struct rule **rules, struct rule **last,
                      struct variable **variables)
{
    if (line[0] == '\t')
        return parse_recipe(line, *last);
    char *ptr = line;
    while (*ptr == ' ')
        ptr++;
    if (*ptr == '\0' || *ptr == '\n')
        return 0;
    while (*ptr != '\0')
    {
        if (*ptr == ':')
        {
            char *target = trim_left(line);
            if (target[0] == ':')
            {
                *last = NULL;
                return 0;
            }
            struct rule *r = parse_rule(line, *variables);
            if (r == NULL)
                return 2;
            add_rule(rules, r);
            *last = r;
            return 0;
        }

        if (*ptr == '=')
        {
            *last = NULL;
            return parse_variable(line, variables);
        }
        ptr++;
    }
    return 2;
}

// lire le fichier
int parse_makefile(const char *filename, struct rule **rules,
                   struct variable **variables)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
        return 2;
    char *line = NULL;
    size_t n = 0;
    struct rule *last = NULL;
    while (getline(&line, &n, f) != -1)
    {
        int num = parse_line(line, rules, &last, variables);
        if (num != 0)
        {
            free(line);
            fclose(f);
            return 2;
        }
    }
    int error = ferror(f);
    free(line);
    fclose(f);
    if (error)
        return 2;
    return 0;
}

static void free_commands(struct command *cmd)
{
    while (cmd != NULL)
    {
        struct command *next = cmd->next;
        free(cmd->text);
        free(cmd);
        cmd = next;
    }
}

void free_variables(struct variable *vars)
{
    while (vars != NULL)
    {
        struct variable *next = vars->next;
        free(vars->name);
        free(vars->value);
        free(vars);
        vars = next;
    }
}

void free_rules(struct rule *rules)
{
    while (rules != NULL)
    {
        struct rule *next = rules->next;
        free(rules->target);
        free_dependencies(rules->deps);
        free_commands(rules->commands);
        free(rules);
        rules = next;
    }
}
