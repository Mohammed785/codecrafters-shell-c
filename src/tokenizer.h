#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <stddef.h>

#define DEFAULT_TOKEN_VALUE_LEN 256

typedef enum TokenType
{
	NO_QUOTE,
	DOUBLE_QUOTE,
	SINGLE_QUOTE,
	ESCAPE,
	EOL,
	SPACE,
	UNDEF,
} TokenType;

typedef struct Token
{
	char *value;
	struct Token *next;
} Token;

typedef struct Tokenizer
{
	char *buffer;
	int commands;
	int pipes;
	Token *tokens;
	int argc;
	TokenType state;
} Tokenizer;

Token *new_token();
Tokenizer *new_tokenizer();

void insert_token(Tokenizer *, Token *);
void clear_tokens(Tokenizer *);
void clear_tokenizer(Tokenizer *);
TokenType get_token_type(char);
void parse(Tokenizer *);
#endif
