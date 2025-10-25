#include <stdio.h>
#include <stdlib.h>
#include "tokenizer.h"

int main(int argc, char *argv[]) {
	setbuf(stdout, NULL);
    setbuf(stderr, NULL);
	size_t len =0;
	ssize_t nread;
	Tokenizer* tokenizer = new_tokenizer();
	while (1) {
		int n;
		printf("$ ");
		if((n=getline(&tokenizer->buffer, &len, stdin))==-1){
			clear_tokenizer(tokenizer);
			exit(1);
		}
		tokenizer->buffer[n-1]='\0';
		printf("%s: command not found\n",tokenizer->buffer);
	}

  return 0;
}
