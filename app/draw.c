#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char **argv){
	setvga();
	for(int y=0;y<200;y++){
		for(int x=0;x<320;x++){
			draw_pixel(x,y,1);
		}
	}
	exit();
}
