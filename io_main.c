#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/wait.h>
#include <errno.h>

#include "libgomp.h"
#include "libkgomp.h"

void subfunction (void *data)
{
	long _s0, _e0;
	while (GOMP_loop_static_next (&_s0, &_e0))
	{
		long _e1 = _e0, i;
		for (i = _s0; i < _e1; i++)
			printf("thread id = %d\n", mppa_getpid());
	}
	GOMP_loop_end_nowait ();
}

void subfunction_mod(void *data){

	while(KGOMP_loop_static_next())
	{

	}
	GOMP_loop_end_nowait ();
}

int main(int argc, char **argv) {

	int lb = 0;
	int ub = 8;
	GOMP_parallel_loop_static (subfunction_mod, NULL, 0, lb, ub+1, 1, 0, 1);
	subfunction_mod(NULL);
	GOMP_parallel_end ();
}
