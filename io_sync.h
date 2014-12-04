#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/wait.h>
#include <errno.h>

#include <mppa_bsp.h>
#include <mppaipc.h>
#include <mppa/osconfig.h>


static int
mppa_io_init_barrier( char *sync_io_to_cc_path, int *sync_io_to_cluster_fd,
		char *sync_cc_to_io_path , int *sync_clusters_to_io_fd, int nb_clusters);

static int
mppa_io_barrier(int sync_io_to_cluster_fd, int sync_clusters_to_io_fd);

static void
mppa_io_close_barrier(int sync_io_to_cluster_fd, int sync_clusters_to_io_fd);
