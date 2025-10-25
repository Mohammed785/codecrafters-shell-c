#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tokenizer.h"
#include "command.h"

bool is_running = true;
int exit_code = 0;

int main(int argc, char *argv[]) {
	setbuf(stdout, NULL);
    setbuf(stderr, NULL);
	size_t len =0;
	ssize_t n;
	Tokenizer* tokenizer = new_tokenizer();
	while (1) {
		printf("$ ");
		if((n=getline(&tokenizer->buffer, &len, stdin))==-1){
			clear_tokenizer(tokenizer);
			exit(1);
		}
        if ( n == 1 )
            continue;
        tokenizer->buffer[n-1] = '\0';
        Token *token = tokenizer->tokens;
        while ( token )
        {
            printf("%s\n", token->value);
            token = token->next;
        }
		parse(tokenizer);
		char* argv[tokenizer->argc];
		build_argv(tokenizer, argv);
		if(strcmp(argv[0],"exit")==0){
			exit(0);
		}
		printf("%s: command not found\n",argv[0]);
		clear_tokens(tokenizer);
	}

  return 0;
}
