#include "types.h"
#include "stat.h"
#include "user.h"

#define BUF_SIZE	512

int main(int argc, char **argv){
	char snum;
	char buf[BUF_SIZE];

	for(;;){
		snum = read(0, buf, BUF_SIZE);
		if(snum < 0 || buf[0]==0) break;
		write(1, buf, snum);
	}

	exit();
}
