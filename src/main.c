#include "command.h"
#include "tokenizer.h"
#include <dirent.h>
#include <readline/history.h>
#include <readline/keymaps.h>
#include <readline/readline.h>
#include <readline/rlstdc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
bool is_running = true;
int exit_code = 0;

// https://tiswww.case.edu/php/chet/readline/readline.html#Custom-Completers
char *dupstr(char *s) {
  char *r;
  r = malloc(strlen(s) + 1);
  strcpy(r, s);
  return (r);
}

const char *commands_names[] = {"echo", "exit", "type", "pwd", "cd", NULL};

void initialize_readline(char *);

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  initialize_readline(argv[0]);
  size_t len = 0;
  ssize_t n;
  Tokenizer *tokenizer = new_tokenizer();
  rl_bind_key('\t', rl_complete);
  using_history();
  while (is_running) {
    char *input;
    if (isatty(STDOUT_FILENO)) {
      input = readline("$ ");
    } else {
      input = readline(NULL);
    }
    if (!input) {
      clear_tokenizer(tokenizer);
      exit(1);
    }
    if (strlen(input) == 0) {
      continue;
    }
    add_history(input);
    tokenizer->buffer = input;
    parse(tokenizer);
    if (check_pipeline(tokenizer)) {
      char **cmds_argv[tokenizer->pipes + 1];
      int cmds_argc[tokenizer->pipes + 1];
      build_argv_pipeline(tokenizer, cmds_argv, cmds_argc);
      exec_commands(tokenizer->pipes, cmds_argc, cmds_argv);
    } else {
      char *cmd_argv[tokenizer->argc];
      build_argv(tokenizer, cmd_argv);
      if (strcmp(cmd_argv[0], "exit") == 0) {
        exit(0);
      }
      exec_command(tokenizer->argc, cmd_argv);
    }
    clear_tokens(tokenizer);
    free(input);
  }
  return exit_code;
}

char *command_generator(const char *, int);
char **completion_function(const char *, int, int);

void initialize_readline(char *pname) {
  rl_readline_name = pname;
  rl_attempted_completion_function = completion_function;
}

char **completion_function(const char *text, int start, int end) {
  char **matches;
  matches = (char **)NULL;
  if (start == 0)
    matches = rl_completion_matches(text, command_generator);

  return (matches);
}

char *command_generator(const char *text, int state) {
  static int list_index, len;
  char *name;
  static char *path = NULL, *dir = NULL;
  static DIR *dirp = NULL;
  static struct dirent *dp = NULL;
  if (!state) {
    list_index = 0;
    len = strlen(text);
    if (path)
      free(path);
    path = strdup(getenv("PATH"));
    dir = strtok(path, ":");
    if (dirp)
      closedir(dirp);
    dirp = NULL;
  }

  while ((name = commands_names[list_index])) {
    list_index++;

    if (strncmp(name, text, len) == 0)
      return (dupstr(name));
  }
  while (dir) {
    if (!dirp)
      dirp = opendir(dir);
    if (dirp) {
      while ((dp = readdir(dirp)) != NULL) {
        if (strncmp(dp->d_name, text, len) == 0) {
          char fpath[PATH_MAX];
          snprintf(fpath, sizeof(fpath), "%s/%s", dir, dp->d_name);
          if (access(fpath, X_OK) == 0) {
            return dupstr(dp->d_name);
          }
        }
      }
      closedir(dirp);
      dirp = NULL;
    }
    dir = strtok(NULL, ":");
  }
  printf("\a");
  return ((char *)NULL);
}
