#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char **argv){
		int nproc;
		struct proc *ptable;

		nproc = getmaxproc();
		printf(1, "nproc=%d\n", nproc);
		
		ptable = malloc(sizeof(struct proc) * nproc);
		if(getptable(ptable, nproc) < 0)
				printf(1, "error\n");

		printf(1, "PID CMD\n");
		for(int i=0;i<nproc;i++){
				struct proc *p = &ptable[i];
				if(p->state == UNUSED) continue;
				printf(1, "%d    %s    \n", p->pid, p->name);
		}

		exit();
}
