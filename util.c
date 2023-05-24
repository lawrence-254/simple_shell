#include "shell.h"
/**
 * get_args - gets command from standard input
 * @line: memory to store the command
 * @exe_ret: value of the last executed command
 * Return: NULL else pointer to the stored command
 */
char *get_args(char *line, int *exe_ret)
{
size_t n = 0;
ssize_t read;
char *prompt = "£ ";

if (line)
free(line);

read = get_line(&line, &n, STDIN_FILENO);
if (read == -1)
return (NULL);
if (read == 1)
{
hist++;
if (isatty(STDIN_FILENO))
write(STDOUT_FILENO, prompt, 2);
return (get_args(line, exe_ret));
}

line[read - 1] = '\0';
rep_var(&line, exe_ret);
handle_line(&line, read);

return (line);
}

/**
 * call_args - operates on arguments
 * @args: arguments in question
 * @front: pointer to the beginning of args.
 * @exe_ret: value of the parent process' last executed command.
 * Return: value of the last executed command.
 */
int call_args(char **args, char **front, int *exe_ret)
{
int ret, index;

if (!args[0])
return (*exe_ret);
for (index = 0; args[index]; index++)
{
if (str_cmp_n(args[index], "||", 2) == 0)
{
free(args[index]);
args[index] = NULL;
args = replace_aliases(args);
ret = run_args(args, front, exe_ret);
if (*exe_ret != 0)
{
args = &args[++index];
index = 0;
}
else
{
for (index++; args[index]; index++)
free(args[index]);
return (ret);
}
}
else if (str_cmp_n(args[index], "&&", 2) == 0)
{
free(args[index]);
args[index] = NULL;
args = replace_aliases(args);
ret = run_args(args, front, exe_ret);
if (*exe_ret == 0)
{
args = &args[++index];
index = 0;
}
else
{
for (index++; args[index]; index++)
free(args[index]);
return (ret);
}
}
}
args = replace_aliases(args);
ret = run_args(args, front, exe_ret);
return (ret);
}

/**
 * run_args - executes a command
 * @args: arguments
 * @front: pointer to the beginning of args
 * @exe_ret: value of the parent process' last executed command
 * Return: value of the last executed command
 */
int run_args(char **args, char **front, int *exe_ret)
{
int ret, i;
int (*builtin)(char **args, char **front);

builtin = get_builtin(args[0]);

if (builtin)
{
ret = builtin(args + 1, front);
if (ret != EXIT)
*exe_ret = ret;
}
else
{
*exe_ret = execute(args, front);
ret = *exe_ret;
}

hist++;

for (i = 0; args[i]; i++)
free(args[i]);

return (ret);
}

/**
 * handle_args - determines execution performed on a command
 * @exe_ret: value of the parent process' last executed command
 * Return: END_OF_FILE (-2) 0 or -1 when there is error
 */
int handle_args(int *exe_ret)
{
int ret = 0, index;
char **args, *line = NULL, **front;

line = get_args(line, exe_ret);
if (!line)
return (END_OF_FILE);

args = str_tok(line, " ");
free(line);
if (!args)
return (ret);
if (check_args(args) != 0)
{
*exe_ret = 2;
free_args(args, args);
return (*exe_ret);
}
front = args;

for (index = 0; args[index]; index++)
{
if (str_cmp_n(args[index], ";", 1) == 0)
{
free(args[index]);
args[index] = NULL;
ret = call_args(args, front, exe_ret);
args = &args[++index];
index = 0;
}
}
if (args)
ret = call_args(args, front, exe_ret);

free(front);
return (ret);
}

/**
 * check_args - confirms if there are any leading ';', ';;', '&&', or '||'.
 * @args: pointer to tokenized commands and arguments.
 * Return: - 2 or 0.
 */
int check_args(char **args)
{
size_t i;
char *cur, *nex;

for (i = 0; args[i]; i++)
{
cur = args[i];
if (cur[0] == ';' || cur[0] == '&' || cur[0] == '|')
{
if (i == 0 || cur[1] == ';')
return (define_error(&args[i], 2));
nex = args[i + 1];
if (nex && (nex[0] == ';' || nex[0] == '&' || nex[0] == '|'))
return (define_error(&args[i + 1], 2));
}
}
return (0);
}

