#include "command.h"
#include "tokenizer.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

const char* builtins[] = {
	"exit",
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
	for(int i=0;i<5;i++){
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
		if(is_builtin(argv[1])){
			printf("%s is a shell builtin\n",argv[1]);
		}else{
			char* path = getenv("PATH");
			for(char* token = strtok(path, ":");token!=NULL;token = strtok(NULL, ":")){
				DIR* dir = opendir(token);
				if(dir==NULL){
					continue;
				}
				struct dirent* in_file;
				struct stat st;
				size_t dir_name_len =strlen(path);
				char *fpath = malloc(dir_name_len+1+255);
				strcpy(fpath, path);
				fpath[dir_name_len] = '/';
				while ((in_file = readdir(dir))!=NULL) {
					if(strcmp(in_file->d_name, argv[1])==0){
						strcpy(fpath+1+dir_name_len, in_file->d_name);
						if(stat(fpath, &st)==0&&st.st_mode&(S_IFREG|S_IXUSR)){
							printf("%s is %s\n",argv[1],fpath);
							closedir(dir);
							return;
						}
					}
				}
				free(fpath);
				closedir(dir);
			}
			printf("%s: not found\n",argv[1]);
		}
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
