#include "types.h"
#include "user.h"

#define bool	_Bool
#define true	1
#define false	0

#define stdin	0
#define stdout	1
#define stderr	2

#define NULL	0
#define EOF	0

#define BUF_SIZE	100
#define MAX_ARGV	10

typedef struct {
	char *str;
	int len;
} String;

char* get_cmdline();
void run_cmdline(char *cmdline);
void exec_cmd(char *cmd);
void exec_pipe(char *cmd, char *rcmd);

int main(int argc, char **argv){
	while(true){
		printf(stdout, "> ");
		char *cmdline = get_cmdline();
		printf(stdout, "cmdline: %s\n", cmdline);

		run_cmdline(cmdline);
	}

	exit();
}

char* get_cmdline(){
	static char buf[BUF_SIZE];
	memset(buf, '\0', BUF_SIZE);
	gets(buf, BUF_SIZE);

	if(buf[0] == EOF)
		exit();
	return buf;
}

char* get_rightcmd(char **pcmdline){
	char *cl = *pcmdline;
	for(;*cl!='\0';cl++){
		if(*cl == '|'){
			*cl = '\0';
			return cl+1;
		}
	}

	return NULL;
}

void run_cmdline(char *cmdline){
	char *rcmdline = get_rightcmd(&cmdline);
	if(rcmdline != NULL){
		printf(stdout, "rcmdline: %s\n", rcmdline);
		exec_pipe(cmdline, rcmdline);
		return;
	}

	if(fork() == 0)
		exec_cmd(cmdline);
	wait();
}

bool is_space(const char c){
	switch(c){
	case ' ':
	case '\t':
	case '\n':
		return true;
	default:
		return false;
	}
}

char* skip_space(char *p){
	for(;;){
		if(!is_space(*p) || *p == '\0')
			break;
		p++;
	}
	return p;
}

String* get_arg(char *cmd){
	char *begin = skip_space(cmd);
	char *end = begin;

	if(*begin == '\0')
		return NULL;
	for(;;){
		if(is_space(*end) || *end=='\0')
			break;
		end++;
	}
	*end = '\0';

	if(cmd == end) return NULL;

	String *arg = malloc(sizeof(String));
	arg->len = end - cmd;
	arg->str = malloc(end-begin+1);

	for(int n=0;n<arg->len;n++){
		printf(stdout, "%c ", arg->str[n]);
		arg->str[n] = begin[n];
	}
	arg->str[arg->len] = '\0';

	return arg;
}

char** parse_arg(char *cmd){
	static char *argv[MAX_ARGV] = {
		"",
	};

	int n=0;
	for(int argc=1;;argc++){
		String *a = get_arg(cmd+n);
		if(a == NULL)
			break;
		argv[argc-1] = a->str;
		n += a->len;
	}

	return argv;
}

void exec_cmd(char *cmd){
	char** argv = parse_arg(cmd);
	printf(stdout, "argv: %s, %s\n", argv[0], argv[1]);
	exec(argv[0], argv);
	printf(stdout, "\ncmd finished\n");
}

void exec_pipe(char *cmd, char *rcmd){
	int fd[2];

	if(pipe(fd) < 0){
		printf(stdout, "pipe error\n");
		exit();
	}
	if(fork() == 0){
		close(stdout);
		dup(fd[1]);
		close(fd[0]);
		close(fd[1]);
		exec_cmd(cmd);
	}
	if(fork() == 0){
		close(stdin);
		dup(fd[0]);
		close(fd[0]);
		close(fd[1]);
		exec_cmd(rcmd);
	}

	close(fd[0]);
	close(fd[1]);
	wait();
	wait();
}
