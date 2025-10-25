#include "tokenizer.h"
#include <stdlib.h>

Tokenizer* new_tokenizer(){
	Tokenizer* tok = malloc(sizeof(Tokenizer));
	tok->buffer = NULL;
	tok->cursor = 0;
	tok->argc = 0;
	tok->tokens = NULL;
	return tok;
};

void clear_tokens(Tokenizer* tok){
	Token* prev = NULL;
	Token* curr = tok->tokens;
	while (curr) {
		prev = curr;
		curr = curr->next;
		free(prev->value);
		free(prev);
	}
	tok->tokens = NULL;
	tok->argc = 0;
}

void clear_tokenizer(Tokenizer* tok){
	clear_tokens(tok);
	free(tok->buffer);
	free(tok);
};
