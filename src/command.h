#ifndef COMMAND_H
#define COMMAND_H
#include <stdbool.h>
#include "tokenizer.h"

extern bool is_running;
extern int exit_code;
typedef enum RedirectType
{
	NO_REDIRECT,
	REDIRECT_OUT,
	REDIRECT_ERR,
	APPEND_OUT,
	APPEND_ERR,
} RedirectType;
typedef struct RedirectState
{
	RedirectType type;
	const char *path;
	int fd;
} RedirectState;

void build_argv(Tokenizer *, char *[]);
void exec_command(int, char *[]);
void exec_builtins(int, char *[]);
bool is_builtin(char *command);
char *find_exc_path(char *);
void check_redirect(int *argc, char *[], RedirectState *);

void build_argv_pipeline(Tokenizer *, char **[], int *);
void exec_commands(int,int *, char **[]);
bool check_pipeline(Tokenizer *);
#endif
