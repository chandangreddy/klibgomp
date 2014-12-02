#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/wait.h>
#include <errno.h>

#include <mppa_bsp.h>
#include <mppaipc.h>
#include <mppa/osconfig.h>

#include "libgomp.h"

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

int main(int argc, char **argv) {

	int lb = 0;
	int ub = 8;
	GOMP_parallel_loop_static (subfunction, NULL, 0, lb, ub+1, 1, 0, 1);
	subfunction (NULL);
	GOMP_parallel_end ();
}
