#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "minimake.h"

static struct rule *find_rule(struct rule *rules, const char *name)
{
    while (rules != NULL)
    {
        if (strcmp(rules->target, name) == 0)
            return rules;
        rules = rules->next;
    }
    return NULL;
}

struct visit
{
    const char *name;
    int changed;
    struct visit *next;
};

struct build_ctx
{
    struct rule *rules;
    struct variable *vars;
    struct visit *done;
    struct visit *active;
};

static struct visit *find_visit(struct visit *v, const char *name)
{
    while (v != NULL)
    {
        if (strcmp(v->name, name) == 0)
            return v;
        v = v->next;
    }
    return NULL;
}

static int newer(const struct stat *a, const struct stat *b)
{
    if (a->st_mtim.tv_sec > b->st_mtim.tv_sec)
        return 1;
    if (a->st_mtim.tv_sec < b->st_mtim.tv_sec)
        return 0;
    return a->st_mtim.tv_nsec > b->st_mtim.tv_nsec;
}

static int execute_command(const char *raw, struct variable *vars)
{
    char *cmd = expand_variables((char *)raw, vars);
    if (cmd == NULL)
        return 2;
    char *p = cmd;
    int silent = 0;
    if (*p == '@')
    {
        silent = 1;
        p++;
    }
    if (!silent && *p != '\0')
        printf("%s\n", p);
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0)
    {
        execl("/bin/sh", "sh", "-c", p, (char *)NULL);
        _exit(127);
    }
    if (pid < 0)
    {
        free(cmd);
        return 2;
    }
    int status = 0;
    while (waitpid(pid, &status, 0) < 0)
    {
        if (errno == EINTR)
            continue;
        free(cmd);
        return 2;
    }
    free(cmd);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        return 2;
    return 0;
}

static int run_recipe(struct rule *r, struct variable *vars)
{
    struct command *cmd = r->commands;
    while (cmd != NULL)
    {
        if (execute_command(cmd->text, vars) != 0)
        {
            fprintf(stderr, "minimake: recipe failed for '%s'\n", r->target);
            return 2;
        }
        cmd = cmd->next;
    }
    return 0;
}

static int record_done(struct build_ctx *ctx, const char *name, int changed)
{
    struct visit *v = malloc(sizeof(*v));
    if (v == NULL)
        return 2;
    v->name = name;
    v->changed = changed;
    v->next = ctx->done;
    ctx->done = v;
    return 0;
}

static int build_target(struct build_ctx *ctx, const char *name, int *changed);

static int build_deps(struct build_ctx *ctx, struct rule *r,
                      const struct stat *target, int exists, int *stale)
{
    struct dependency *dep = r->deps;
    while (dep != NULL)
    {
        int dep_changed = 0;
        if (build_target(ctx, dep->name, &dep_changed) != 0)
            return 2;
        struct stat info = { 0 };
        int dep_exists = stat(dep->name, &info) == 0;
        if (dep_changed || !exists || !dep_exists || newer(&info, target))
            *stale = 1;
        dep = dep->next;
    }
    return 0;
}

static int build_rule(struct build_ctx *ctx, struct rule *r, int *changed)
{
    struct stat info = { 0 };
    int exists = stat(r->target, &info) == 0;
    int stale = !exists;
    struct visit active = { r->target, 0, ctx->active };
    ctx->active = &active;
    int res = build_deps(ctx, r, &info, exists, &stale);
    ctx->active = active.next;
    if (res != 0)
        return res;
    if (stale && run_recipe(r, ctx->vars) != 0)
        return 2;
    *changed = stale;
    return record_done(ctx, r->target, stale);
}

static int build_target(struct build_ctx *ctx, const char *name, int *changed)
{
    struct visit *done = find_visit(ctx->done, name);
    if (done != NULL)
    {
        *changed = done->changed;
        return 0;
    }
    if (find_visit(ctx->active, name) != NULL)
    {
        fprintf(stderr, "minimake: dependency cycle involving '%s'\n", name);
        return 2;
    }
    struct rule *r = find_rule(ctx->rules, name);
    if (r != NULL)
        return build_rule(ctx, r, changed);
    struct stat info;
    if (stat(name, &info) == 0)
    {
        *changed = 0;
        return 0;
    }
    fprintf(stderr, "minimake: no rule to make target '%s'\n", name);
    return 2;
}

static void free_visits(struct visit *v)
{
    while (v != NULL)
    {
        struct visit *next = v->next;
        free(v);
        v = next;
    }
}

static int select_targets(int argc, char **argv, struct build_ctx *ctx)
{
    int found = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0)
        {
            i++;
            continue;
        }
        if (strncmp(argv[i], "--file=", 7) == 0
            || strncmp(argv[i], "--makefile=", 11) == 0
            || strcmp(argv[i], "-p") == 0
            || strcmp(argv[i], "--pretty-print") == 0
            || strcmp(argv[i], "--print-data-base") == 0)
            continue;
        int changed = 0;
        if (build_target(ctx, argv[i], &changed) != 0)
            return 2;
        found = 1;
    }
    if (!found && ctx->rules != NULL)
    {
        int changed = 0;
        return build_target(ctx, ctx->rules->target, &changed);
    }
    return 0;
}

int build_targets(int argc, char **argv, struct rule *rules,
                  struct variable *vars)
{
    struct build_ctx ctx = { rules, vars, NULL, NULL };
    int res = select_targets(argc, argv, &ctx);
    free_visits(ctx.done);
    return res;
}
