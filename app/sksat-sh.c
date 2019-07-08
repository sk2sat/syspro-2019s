#include "types.h"
#include "user.h"
#include "fcntl.h"

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

typedef struct {
	char *input, *output;
} RedirInfo;

char* get_cmdline();
void run_cmdline(char *cmdline);
void exec_cmd(char *cmd);
void exec_pipe(char *cmd, char *rcmd);

char *skip_space(char *p);

int main(int argc, char **argv){
	printf(stdout, "Welcome to sksat-sh, a tiny shell\n");
	while(true){
		printf(stdout, "> ");
		char *cmdline = get_cmdline();
		printf(stdout, "cmdline: \"%s\"\n", cmdline);

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

	for(int n=BUF_SIZE-1;;n--){
		if(buf[n] == '\n'){
			buf[n] = '\0';
			break;
		}
	}

	return buf;
}

char* get_rightcmd(char *cmdline){
	for(;*cmdline!='\0';cmdline++){
		if(*cmdline == '|'){
			*cmdline = '\0';
			return skip_space(cmdline+1);
		}
	}

	return NULL;
}

void run_cmdline(char *cmdline){
	char *rcmdline = get_rightcmd(cmdline);
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

	if(cmd == end) return NULL;

	String *arg = malloc(sizeof(String));
	arg->len = end - cmd;
	arg->str = malloc(end-begin+1);

//	printf(stdout, "get_arg: ");
	for(int n=0;n<arg->len;n++){
		//printf(stdout, "%c", begin[n]);
		arg->str[n] = begin[n];
	}
//	printf(stdout, "\n");

	arg->str[arg->len] = '\0';

	return arg;
}

RedirInfo* parse_redirect(char *cmd){
	RedirInfo *info = malloc(sizeof(RedirInfo));

	for(char *p=cmd;*p!='\0';p++){
		if(*p == '>'){
			*p = '\0';
			p = skip_space(p+1);
//			printf(stdout, "output file: %s\n", p);
			info->output = p;
			for(;*p!='\0';p++){
				if(is_space(*p)){
					*p = '\0';
					break;
				}
			}
			break;
		}
	}
	return info;
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
	RedirInfo *rinfo = parse_redirect(cmd);
//	printf(stdout, "before parse arg: %s\n", cmd);
	char** argv = parse_arg(cmd);

	if(rinfo->output != NULL){
		close(stdout);
		int fd = open(rinfo->output, O_WRONLY|O_CREATE);
		printf(stderr, "file open: fname=\"%s\", fd=%d\n", rinfo->output, fd);
	}

//	printf(stdout, "argv: %s, %s\n", argv[0], argv[1]);
	exec(argv[0], argv);
//	printf(stdout, "\ncmd finished\n");
}

void exec_pipe(char *cmd, char *rcmd){
	int fd[2];

	printf(stderr, "exec pipe\n");

	if(pipe(fd) < 0){
		printf(stderr, "pipe error\n");
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
