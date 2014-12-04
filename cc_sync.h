#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <mppaipc.h>
#include "libkgomp_util.h"

 int mppa_cc_init_barrier( int *sync_io_to_cluster_fd,
		int *sync_clusters_to_io_fd, int nb_clusters);

int mppa_cc_barrier(int sync_io_to_cluster_fd, int sync_clusters_to_io_fd);

void mppa_cc_close_barrier(int sync_io_to_cluster_fd, int sync_clusters_to_io_fd);
