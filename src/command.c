#include "command.h"
#include "tokenizer.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include<readline/readline.h>
#include <readline/history.h>

int history_limit = -1;
int last_append_offset = -1;

const char *builtins[] = {
	"exit",
	"echo",
	"type",
	"pwd",
	"cd",
	"history",
};

static inline char *join_strings(char *arr[], char *sep, int count)
{
	if (count <= 0)
		return "";
	size_t sep_len = strlen(sep);
	int total_length = 1;
	int i = 0;
	for (i = 0; i < count; i++)
		total_length += strlen(arr[i]) + (i < count - 1 ? sep_len : 0);
	char *str = malloc(sizeof(char) * total_length);
	if (!str)
		return NULL;
	str[0] = '\0';
	for (i = 0; i < count; i++)
	{
		strcat(str, arr[i]);
		if (i < (count - 1))
			strcat(str, sep);
	}
	return str;
}

char *find_exc_path(char *cmd)
{
	char *path = strdup(getenv("PATH"));
	for (char *dir = strtok(path, ":"); dir != NULL; dir = strtok(NULL, ":"))
	{
		size_t dir_name_len = strlen(dir);
		size_t full_len = dir_name_len + 2 + strlen(cmd);
		char *fpath = malloc(full_len);
		snprintf(fpath, full_len, "%s/%s", dir, cmd);
		if (access(fpath, X_OK) == 0)
		{
			free(path);
			return fpath;
		}
		free(fpath);
	}
	free(path);
	return NULL;
};

inline bool is_builtin(char *command)
{
	for (int i = 0; i < 6; i++)
	{
		if (strcmp(command, builtins[i]) == 0)
		{
			return true;
		}
	}
	return false;
}

void exec_builtins(int argc, char *argv[])
{
	if (strcmp(argv[0], "echo") == 0)
	{
		if (argc <= 1)
		{
			printf("\n");
			return;
		}
		char *str = join_strings(&argv[1], " ", argc - 1);
		printf("%s\n", str);
		free(str);
	}
	else if (strcmp(argv[0], "type") == 0)
	{
		if (argc <= 1)
		{
			printf("");
			return;
		}
		if (is_builtin(argv[1]))
		{
			printf("%s is a shell builtin\n", argv[1]);
		}
		else
		{
			char *path = find_exc_path(argv[1]);
			if (path != NULL)
			{
				printf("%s is %s\n", argv[1], path);
			}
			else
			{
				printf("%s: not found\n", argv[1]);
			}
		}
	}
	else if (strcmp(argv[0], "pwd") == 0)
	{
		char buf[256];
		if (getcwd(buf, 256) != NULL)
		{
			printf("%s\n", buf);
		}
	}
	else if (strcmp(argv[0], "cd") == 0)
	{
		if (argc <= 1 || strcmp(argv[1], "~") == 0)
		{
			argv[1] = getenv("HOME");
		}
		if (chdir(argv[1]) == -1)
		{
			printf("cd: %s: No such file or directory\n", argv[1]);
		}
	}else if(strcmp(argv[0], "history")==0){
		if(argc==2){
			HISTORY_STATE *hist_state = history_get_history_state();
			HIST_ENTRY** hist_list = history_list();
			if(hist_list!=NULL){
				history_limit = atoi(argv[1]);
				int limit = (hist_state->length-history_limit)>=0?hist_state->length-history_limit:0;
				for(int i=limit;i<hist_state->length;i++){
					printf("    %d  %s\n",i+1,hist_list[i]->line);
				}
			}
		}else if(argc==3){
			if(strcmp(argv[1],"-r")==0){
				read_history(argv[2]);
			}else if(strcmp(argv[1],"-w")==0){
				write_history(argv[2]);
				// HIST_ENTRY** hist_list = history_list();
				// if(hist_list!=NULL){
				// 	for(HIST_ENTRY** entry=hist_list;*hist_list!=NULL;entry++){
				// 		fprintf(f, "%s\n",(*entry)->line);
				// 	}
				// }
				// fclose(f);
			}else if(strcmp(argv[1],"-a")==0){
				HISTORY_STATE *hist_state = history_get_history_state();
				if(last_append_offset==-1){
					last_append_offset = hist_state->length;
				}else{
					last_append_offset = hist_state->length-last_append_offset;
				}
				append_history(last_append_offset, argv[2]);
			}
		}else{
		    HISTORY_STATE *hist_state = history_get_history_state();
			HIST_ENTRY** hist_list = history_list();
			if(hist_list!=NULL){
				// int limit = 0;
				// if(history_limit!=-1){
				// 	limit = (hist_state->length-history_limit)>=0?hist_state->length-history_limit:0;
				// }
				for(int i=0;i<hist_state->length;i++){
					printf("    %d  %s\n",i+1,hist_list[i]->line);
				}
			}
		}
	}
}

void check_redirect(int *argc, char *argv[], RedirectState *state)
{
	state->type = NO_REDIRECT;
	state->fd = 0;
	for (int i = 0; i < *argc; i++)
	{
		char *arg = argv[i];
		if (strcmp(">", arg) == 0 || strcmp("1>", arg) == 0)
		{
			state->type = REDIRECT_OUT;
		}
		else if (strcmp("2>", arg) == 0)
		{
			state->type = REDIRECT_ERR;
		}
		else if (strcmp(">>", arg) == 0 || strcmp("1>>", arg) == 0)
		{
			state->type = APPEND_OUT;
		}
		else if (strcmp("2>>", arg) == 0)
		{
			state->type = APPEND_ERR;
		}
		else
		{
			continue;
		}
		if (i + 1 >= *argc)
		{
			state->type = NO_REDIRECT;
			continue;
		}
		state->path = argv[i + 1];
		argv[i] = NULL;
		argv[i + 1] = NULL;
		*argc = *argc - 2;
		return;
	}
}

void exec_command(int argc, char *argv[])
{
	RedirectState state = {.fd = 0, .path = NULL, .type = NO_REDIRECT};
	check_redirect(&argc, argv, &state);
	FILE *fp;
	if (state.type != NO_REDIRECT)
	{
		fp = fopen(state.path, (state.type == REDIRECT_ERR || state.type == REDIRECT_OUT) ? "w" : "a");
		if (state.type == REDIRECT_OUT || state.type == APPEND_OUT)
		{
			state.fd = dup(STDOUT_FILENO);
			dup2(fileno(fp), STDOUT_FILENO);
		}
		else if (state.type == REDIRECT_ERR || state.type == APPEND_ERR)
		{
			state.fd = dup(STDERR_FILENO);
			dup2(fileno(fp), STDERR_FILENO);
		}
	}
	if (is_builtin(argv[0]))
	{
		exec_builtins(argc, argv);
	}
	else
	{
		char *path = find_exc_path(argv[0]);
		if (path == NULL)
		{
			printf("%s: command not found\n", argv[0]);
			return;
		}
		pid_t p = fork();
		if (p == 0)
		{
			execvp(argv[0], argv);
			exit(0);
		}
		else if (p > 0)
		{
			int status;
			waitpid(p, &status, 0);
		}
		else
		{
			perror("fork");
			exit_code = 1;
		}
	}
	if (state.type != NO_REDIRECT)
	{
		dup2(state.fd, (state.type == REDIRECT_OUT || state.type == APPEND_OUT) ? STDOUT_FILENO : STDERR_FILENO);
		fclose(fp);
		close(state.fd);
	}
}

void build_argv(Tokenizer *tokenizer, char *argv[])
{
	Token *token = tokenizer->tokens;
	int i = 0;
	while (token)
	{
		argv[i] = token->value;
		token = token->next;
		i++;
	}
	argv[i] = NULL;
}
#define CMD_ARGS_COUNT 5
void build_argv_pipeline(Tokenizer *tokenizer, char ***argv, int *argc)
{
	Token *token = tokenizer->tokens;
	int i = 0;
	int j = 0;
	int cap = CMD_ARGS_COUNT;
	argv[i] = malloc(CMD_ARGS_COUNT * sizeof(char *));
	while (token)
	{
		if (strcmp(token->value, "|") == 0)
		{
			argv[i][j] = NULL;
			argc[i] = j;
			i++;
			argv[i] = malloc(CMD_ARGS_COUNT * sizeof(char *));
			j = 0;
			token = token->next;
			cap = CMD_ARGS_COUNT;
			continue;
		}
		argv[i][j] = token->value;
		token = token->next;
		j++;
		if (j == cap)
		{
			cap *= 2;
			argv[i] = realloc(argv[i], cap * sizeof(char *));
		}
	}
	argv[i][j] = NULL;
	argv[i + 1] = NULL;
}

bool check_pipeline(Tokenizer *tokenizer)
{
	for (Token *token = tokenizer->tokens; token; token = token->next)
	{
		if (strncmp(token->value, "|", 1) == 0)
		{
			return true;
		}
	}
	return false;
}

void exec_commands(int pipes_c, int *cmd_argc, char ***cmds)
{
	int N = pipes_c;
	int pipes[N][2];
	int pids[pipes_c+1];
	for (int i = 0; i < N; i++)
	{
		if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
	}
	int i = 0;
	for (char ***cmd = cmds; *cmd != NULL; i++, cmd++)
	{
		pids[i] = fork();
		if (pids[i] == 0)
		{
			if (i > 0)
			{
				dup2(pipes[i - 1][0], STDIN_FILENO);
			}
			if (i < N)
			{
				dup2(pipes[i][1], STDOUT_FILENO);
			}
			for (int j = 0; j < N; j++)
			{
				close(pipes[j][0]);
				close(pipes[j][1]);
			}
			if (is_builtin(*cmd[0]))
			{
				exec_builtins(cmd_argc[i], *cmd);
				_exit(0);
			}
			else
			{
				char *path = find_exc_path(*cmd[0]);
				if (path == NULL)
				{
					printf("%s: command not found\n", *cmd[0]);
					return;
				}
				execvp(*cmd[0], *cmd);
				perror("execvp error");
				_exit(1);
			}
		}
	}
	for (int i = 0; i < N; i++)
	{
		close(pipes[i][0]);
		close(pipes[i][1]);
	}

	for (int i = 0; i < N+1; i++)
	{
		waitpid(pids[i], NULL, 0);
	}
}
