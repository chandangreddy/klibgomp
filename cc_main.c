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
        printf("thread id = %d\n", mppa_getpid());
}

int main(int argc, char **argv) {

	int lb = 0;
	int ub = 8;
	subfunction (NULL);
}
