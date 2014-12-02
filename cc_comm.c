#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/wait.h>
#include <errno.h>

#include <mppa_bsp.h>
#include <mppaipc.h>
#include <mppa/osconfig.h>


int mppa_comm_init(char *io_to_cc_path, char *cc_to_io_path, int *cc_to_io_fd, int *io_to_cc_fd){


		if((io_to_cc_fd = mppa_open(io_to_cc_path, O_RDONLY)) < 0) {
			EMSG("Open portal failed for %s\n", io_to_cc_path);
			mppa_exit(1);
		}

		if((cc_to_io_fd = mppa_open(cc_to_io_path, O_WRONLY)) < 0) {
			EMSG("Open portal failed for %s\n", cc_to_io_path);
			mppa_exit(1);
		}

		return 1;
}


void mppa_comm_close(int *cc_to_io_fd, int *io_to_cc_fd){

	mppa_close(cc_to_io_fd);
	mppa_close(io_to_cc_fd);
}


int mppa_comm_async_send(mppa_aiocb_t *cc_to_io_aiocb, int cc_to_io_fd, void *buf, int buf_size){

	mppa_aiocb_ctor(cc_to_io_aiocb, cc_to_io_fd, buf, buf_size);
	mppa_aiocb_set_pwrite(cc_to_io_aiocb, buf, buf_size, 0);
	int status = mppa_aio_write(cc_to_io_aiocb);
	assert(status == 0);
}

int mppa_comm_async_wait(mppa_aiocb_t io_to_cc_aiocb, int buf_size){

	int status = mppa_aio_wait(&io_to_cc_aiocb);
	assert(status == buf_size);
	return 1;
}

int mppa_comm_async_receive(mppa_aiocb_t *io_to_cc_aiocb, int io_to_cc_fd, void *buf, int buf_size){

	mppa_aiocb_ctor(io_to_cc_aiocb, io_to_cc_fd, buf, buf_size);
	mppa_aiocb_set_trigger(io_to_cc_aiocb, 1);
	int status = mppa_aio_read(io_to_cc_aiocb);
	assert(status == 0);
	return 1;
}

int mppa_comm_send(int cc_to_io_fd, void *buf, int buf_size){

	mppa_aiocb_t cc_to_io_aiocb;
	mppa_comm_async_send(&cc_to_io_aiocb, cc_to_io_fd, buf, buf_size);
	mppa_comm_async_wait(cc_to_io_aiocb, buf_size);

	return 1;
}


int mppa_comm_receive(int io_to_cc_fd, void *buf, int buf_size){

	mppa_aiocb_t io_to_cc_aiocb;
	mppa_comm_async_receive(&io_to_cc_aiocb, io_to_cc_fd, buf, buf_size);
	mppa_comm_async_wait(io_to_cc_aiocb, buf_size);

	return 1;
}
