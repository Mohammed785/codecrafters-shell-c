#include "command.h"
#include "tokenizer.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>

const char* builtins[] = {
	"exit",
	"echo",
	"type",
	"pwd",
	"cd",
};

static inline char* join_strings(const char* arr[],char* sep,int count){
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

char* find_exc_path(const char* cmd){
	char* path = strdup(getenv("PATH"));
	for(char* dir = strtok(path, ":");dir!=NULL;dir = strtok(NULL, ":")){
		size_t dir_name_len =strlen(dir);
		size_t full_len= dir_name_len+2+strlen(cmd);
		char* fpath = malloc(full_len);
		snprintf(fpath, full_len, "%s/%s",dir,cmd);
		if(access(fpath, X_OK)==0){
			return fpath;
		}
	}
	return NULL;
};

inline bool is_builtin(const char* command){
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
			char* path = find_exc_path(argv[1]);
			if(path!=NULL){
				printf("%s is %s\n",argv[1],path);
			}else{
				printf("%s: not found\n",argv[1]);
			}
		}
	}else if(strcmp(argv[0], "pwd")==0){
		char buf[256];
		if(getcwd(buf, 256)!=NULL){
			printf("%s\n",buf);
		}
	}else if(strcmp(argv[0], "cd")==0){
		if(strcmp(argv[1], "~")==0){
			argv[1] = getenv("HOME");
		}
		if(chdir(argv[1])==-1){
			printf("cd: %s: No such file or directory\n",argv[1]);
		}
	}
}

void exec_command(int argc,const char* argv[]){
	if(is_builtin(argv[0])){
		exec_builtins(argc, argv);
	}else{
		char* path = find_exc_path(argv[0]);
		if(path==NULL){
			printf("%s: command not found\n",argv[0]);
			return;
		}
		pid_t p = fork();
		if(p==0){
			execvp(argv[0], (char *const *)argv);
			exit(0);
		}else if(p>0){
			int status;
			waitpid(p,&status,0);
		}else{
			perror("fork");
			exit_code = 1;
		}
		return;
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
