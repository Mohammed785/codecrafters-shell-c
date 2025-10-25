#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <stddef.h>

typedef struct Token {
	char* value;
	struct Token *next;
} Token;

typedef struct Tokenizer{
	char* buffer;
	size_t cursor;
	Token* tokens;
	int argc;
	int state; //edit or remove
} Tokenizer;

Token* new_token();
Tokenizer* new_tokenizer();

void insert_token(Tokenizer*,Token*);
void clear_tokens(Tokenizer*);
void clear_tokenizer(Tokenizer*);

void parse(Tokenizer*);
#endif
