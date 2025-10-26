#include "command.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char* builtins[] = {
	// "exit",
	"echo",
	"type",
	"pwd",
	"cd",
};

char* join_strings(const char* arr[],char* sep,int count){
	char* str = NULL;
	int total_length = 0;
	int i=0;
	for(i=0;i<count;i++) total_length+=strlen(arr[i]);
	str = malloc(sizeof(char)*total_length);
	str[0] = '\0';
	for(i=0;i<count;i++){
		strcat(str, arr[i]);
		if(i<(count-1))strcat(str,sep);
	}
	return str;
}

bool is_builtin(const char* command){
	for(int i=0;i<4;i++){
		if(strcmp(command, builtins[i])==0){
			return true;
		}
	}
	return false;
}

void exec_builtins(int argc,const char* argv[]){
	if(strcmp(argv[0], "echo")==0){
		char* str = join_strings(&argv[1], " ", argc-1);
		printf("%s\n",str);
		free(str);
	}else if(strcmp(argv[0], "type")==0){

	}
}

void exec_command(int argc,const char* argv[]){
	if(is_builtin(argv[0])){
		exec_builtins(argc, argv);
	}else{
		printf("%s: command not found\n",argv[0]);
	}
}

void build_argv(Tokenizer* tokenizer,char* argv[]){
	Token* token = tokenizer->tokens;
	int i=0;
	while (token) {
		argv[i] = token->value;
		token = token->next;
		i++;
	}
	argv[i] = NULL;
}
