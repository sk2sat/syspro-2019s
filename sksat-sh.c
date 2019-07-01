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
void exec_cmd(char *buf);

int main(int argc, char **argv){
	while(true){
		printf(stdout, "> ");
		char *cmdline = get_cmdline();
		printf(stdout, "cmdline: %s\n", cmdline);

		exec_cmd(cmdline);
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

bool is_delim(const char c){
	if(is_space(c))
		return true;
	switch(c){
	case '|':		// pipe
		return true;
	}

	return false;
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
	int pid;
	pid = fork(); // pid=0: child

	if(pid == 0){
		char** argv = parse_arg(cmd);
		printf(stdout, "argv: %s, %s\n", argv[0], argv[1]);
		exec(argv[0], argv);
	}
	wait();
	printf(stdout, "\ncmd finished\n");
}
