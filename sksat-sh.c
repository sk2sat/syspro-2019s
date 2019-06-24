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

void get_cmd(char *buf);
void parse_arg(char *buf, char **argv);
void exec_cmd(char *buf);

int main(int argc, char **argv){
	char buf[BUF_SIZE];

	while(true){
		printf(stdout, "> ");
		get_cmd(buf);
		printf(stdout, "cmd: %s\n", buf);

		exec_cmd(buf);
	}

	exit();
}

void get_cmd(char *buf){
	memset(buf, '\0', BUF_SIZE);
	gets(buf, BUF_SIZE);

	if(buf[0] == EOF)
		exit();
}

bool is_space(const char c){
	switch(c){
	case ' ':
	case '\t':
	case '\n':
		return true;
	}

	return false;
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
		if(is_space(*p) || *p == '\0')
			break;
		p++;
	}
	return p;
}

bool is_pipe(char *buf){
	for(;;){
		if(*buf == '|')
			return true;
		else if(*buf == '\0')
			return false;
		buf++;
	}
}

void exec_cmd(char *buf){
	char *argv[MAX_ARGV];
	int pid;
	pid = fork(); // pid=0: child

	if(pid == 0){
		if(is_pipe(buf)){
			printf(stdout, "pipe!\n");
		}
		parse_arg(buf, argv);
		exec(argv[0], argv);
	}
	wait();
	printf(stdout, "\nfinished\n");
}
