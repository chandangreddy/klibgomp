#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/wait.h>
#include <errno.h>

#include <mppa_bsp.h>
#include <mppaipc.h>

#include "libgomp.h"
#include "cc_sync.h"
#include "cc_comm.h"
#include "serialize_parfor.h"
#include "libkgomp_util.h"

char *io_to_cc_path;
char *cc_to_io_path;

char *sync_io_to_cc_path;
char *sync_cc_to_io_path;

int io_to_cc_fd;
int cc_to_io_fd;

int sync_io_to_cc_fd;
int sync_cc_to_io_fd;

void subfunction (void *data) {
        printf("thread id = %d\n", mppa_getpid());
}

void execute_parfor_task(parfor_work_item_t task){

	int i;
	switch (task.function_id) {
		case 1:
			for(i=task.start; i<task.end; i++)
				subfunction(NULL);
			break;
		default:
			EMSG("Invalid function ID\n");
			break;
	}

}


int main(int argc, char **argv) {

	int i;
	int argn = 1;

	io_to_cc_path = argv[argn++];
	cc_to_io_path = argv[argn++];

	sync_io_to_cc_path = argv[argn++];
	sync_cc_to_io_path = argv[argn++];

	char *nb_clusters_str = argv[argn++];
	char *nb_threads_str = argv[argn++];

	int nb_clusters = convert_str_to_ul(nb_clusters_str);
	int nb_threads = convert_str_to_ul(nb_threads_str);

	mppa_cc_init_barrier(&sync_io_to_cc_fd, &sync_cc_to_io_fd, nb_clusters);
	mppa_cc_comm_init(io_to_cc_path, cc_to_io_path, &io_to_cc_fd, &io_to_cc_fd);

	mppa_cc_barrier(sync_io_to_cc_fd, sync_cc_to_io_fd);

	int max_buf_size = 1024 *10;
	char *buf = (char *)malloc(max_buf_size);
	mppa_cc_comm_receive(io_to_cc_fd, buf, max_buf_size);

	int num_work_items = 0;
	parfor_work_item_t *work_items = kgomp_deserialize_parfor(buf, &num_work_items);

	for(i=0;i<num_work_items;i++){
		execute_parfor_task(work_items[i]);
	}

	mppa_cc_barrier(sync_io_to_cc_fd, sync_cc_to_io_fd);
	mppa_cc_comm_close(cc_to_io_fd, io_to_cc_fd);
	mppa_cc_close_barrier(sync_io_to_cc_fd, sync_cc_to_io_fd);

	return 0;
}


