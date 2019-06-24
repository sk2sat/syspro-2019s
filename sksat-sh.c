#include "types.h"
#include "user.h"

#define true	1

#define stdin	0
#define stdout	1
#define stderr	2

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

void parse_arg(char *buf, char **argv){
	int i;

	// とりあえずargv[0]だけのはりぼてコマンドラインパーサ
	for(i=0;i<BUF_SIZE;i++){
		if(buf[i] == ' ' || buf[i] == '\n'){
			buf[i] = '\0';
			argv[0] = buf;
			break;
		}
	}
}

void exec_cmd(char *buf){
	char *argv[MAX_ARGV];
	int pid;
	pid = fork(); // pid=0: child

	if(pid == 0){
		parse_arg(buf, argv);
		exec(argv[0], argv);
	}
	wait();
	printf(stdout, "\nfinished\n");
}
