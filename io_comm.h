#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/wait.h>
#include <errno.h>

#include <mppa_bsp.h>
#include <mppaipc.h>
#include <mppa/osconfig.h>


int mppa_io_comm_init(int nb_clusters, int nb_threads, int *cc_to_io_fd, int *io_to_cc_fd);
int mppa_io_comm_close(int nb_clusters, int *io_to_cc_fd, int *cc_to_io_fd);
int mppa_io_comm_async_send(mppa_aiocb_t *io_to_cc_aiocb, int io_to_cc_fd, void *buf, int buf_size);
int mppa_io_comm_async_wait(mppa_aiocb_t cc_to_io_aiocb, int buf_size);
int mppa_io_comm_async_receive(mppa_aiocb_t *cc_to_io_aiocb, int cc_to_io_fd, void *buf, int buf_size);
int mppa_io_comm_send(int io_to_cc_fd, void *buf, int buf_size);
int mppa_io_comm_receive(int cc_to_io_fd, void *buf, int buf_size);
