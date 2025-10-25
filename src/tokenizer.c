#include "tokenizer.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

Tokenizer* new_tokenizer(){
	Tokenizer* tok = malloc(sizeof(Tokenizer));
	tok->buffer = NULL;
	tok->cursor = 0;
	tok->argc = 0;
	tok->tokens = NULL;
	tok->state = UNDEF;
	return tok;
};

Token* new_token(){
	Token* token = malloc(sizeof(Token));
	token->value = malloc(sizeof(char)*256);
	token->next = NULL;
	return token;
};

static inline void append_token_value(Token* token,int i,char c){
	if(strlen(token->value)>=DEFAULT_TOKEN_VALUE_LEN){
		if((token->value = realloc(token->value, sizeof(token->value)*2))==NULL){
			perror("couldn't realloc token value");
			exit(1);
		}
	}
	token->value[i] = c;
}

void insert_token(Tokenizer* tokenizer,Token* token){
	Token* curr = tokenizer->tokens;
	if(!curr){
		tokenizer->tokens = token;
		tokenizer->argc++;
		return;
	}
	while (curr->next) {
		curr = curr->next;
	}
	curr->next = token;
	tokenizer->argc++;
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

TokenType get_token_type(char c){
	switch (c) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			return SPACE;
		case '\\':
			return ESCAPE;
		case '\'':
			return SINGLE_QUOTE;
		case '"':
			return DOUBLE_QUOTE;
		case '\0':
			return EOL;
		default:
			return NO_QUOTE;
}
};
void parse(Tokenizer * tokenizer){
	Token* token = new_token();
	size_t cursor = 0;

	char current_char;
	TokenType current_char_type;
	int i = 0;
	tokenizer->state = get_token_type(tokenizer->buffer[0]);
	if(tokenizer->state==SINGLE_QUOTE||tokenizer->state==DOUBLE_QUOTE){
		i++;
	}
	for(;i<=strlen(tokenizer->buffer);i++){
		current_char = tokenizer->buffer[i];
		current_char_type = get_token_type(current_char);
		switch (tokenizer->state) {
			case SPACE:
				if(current_char_type==SPACE){
					continue;
				}else if(current_char_type==EOL){
					free(token);
				}else if(current_char_type==NO_QUOTE){
					// token->value[cursor++] = current_char;
					append_token_value(token, cursor++, current_char);
					tokenizer->state = NO_QUOTE;
				}else if(current_char_type==ESCAPE){
					// token->value[cursor++] = tokenizer->buffer[++i];
					append_token_value(token, cursor++, tokenizer->buffer[++i]);
					tokenizer->state = NO_QUOTE;
				}else if(current_char_type==SINGLE_QUOTE||current_char_type==DOUBLE_QUOTE){
					tokenizer->state = current_char_type;
				}
				continue;
			case NO_QUOTE:
				if(current_char_type==ESCAPE){
					append_token_value(token, cursor++, tokenizer->buffer[++i]);
				}else if(current_char_type==EOL){
					token->value[cursor] = '\0';
					insert_token(tokenizer, token);
				}else if(current_char_type==NO_QUOTE){
					append_token_value(token, cursor++, current_char);
				}else if(current_char_type==SPACE||current_char_type==SINGLE_QUOTE||current_char==DOUBLE_QUOTE){
					token->value[cursor] = '\0';
					insert_token(tokenizer, token);
					cursor = 0;
					token = new_token();
					tokenizer->state = current_char_type;
				}
				continue;
			case SINGLE_QUOTE:
				if(current_char_type!=SINGLE_QUOTE){
					append_token_value(token, cursor++, current_char);
					continue;
				}
				// end of quote
				char next = tokenizer->buffer[++i];
				TokenType next_char_type = get_token_type(next);
				switch (next) {
					case ' ':
						token->value[cursor] = '\0';
						insert_token(tokenizer, token);
						cursor = 0;
						token = new_token();
						tokenizer->state = SPACE;
						continue;
					case '\0':
						token->value[cursor] = '\0';
						insert_token(tokenizer, token);
						continue;
					case '\'':
					case '"':
						i++;
						break;
				}
				tokenizer->state = next_char_type;
				continue;
			case DOUBLE_QUOTE:
				if(current_char_type==ESCAPE){
					char next = tokenizer->buffer[i+1];
					switch (next) {
						case '$':
						case '\\':
						case '"':
							append_token_value(token, cursor++, tokenizer->buffer[++i]);
							continue;
						default:
							append_token_value(token, cursor++, current_char);
							continue;
						
					}
				}else if(current_char_type!=DOUBLE_QUOTE){
					append_token_value(token, cursor++, current_char);					
					continue;
				}
				next = tokenizer->buffer[++i];
				next_char_type = get_token_type(next);
				switch (next) {
					case ' ':
						token->value[cursor] = '\0';
						insert_token(tokenizer, token);
						cursor = 0;
						token = new_token();
						tokenizer->state = SPACE;
						continue;
					case '\0':
						token->value[cursor] = '\0';
						insert_token(tokenizer, token);
						continue;
					case '\'':
					case '"':
						i++;
						break;
				}
				tokenizer->state = next_char_type;
				continue;
			case EOL:
				break;
			default:
				perror("couldn't determine token type");
				exit(1);
		}
	}
}
