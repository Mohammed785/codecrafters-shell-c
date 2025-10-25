#ifndef COMMAND_H
#define COMMAND_H
#include <stdbool.h>
#include "tokenizer.h"

extern bool is_running;
extern int exit_code;

void build_argv(Tokenizer*,char* []);
void handle_builtins(char*[]);
#endif
