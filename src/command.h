#ifndef COMMAND_H
#define COMMAND_H
#include <stdbool.h>
#include "tokenizer.h"

extern bool is_running;
extern int exit_code;

void build_argv(Tokenizer*,char* []);
void exec_command(int,const char*[]);
void exec_builtins(int,const char*[]);
bool is_builtin(const char* command);
#endif
