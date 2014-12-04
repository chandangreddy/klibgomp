#include "io_comm.h"


int mppa_io_comm_init(int nb_clusters, char** io_to_cc_path, char** cc_to_io_path,
		int *io_to_cc_fd, int *cc_to_io_fd){

	int  k = 0;

	int io_dnoc_rx_port = 10;
	int cluster_dnoc_rx_port = 3;
	int io_cnoc_rx_port = 3;
	int cluster_cnoc_rx_port = 3;

	for(k=0;k<nb_clusters;k++){
			snprintf(io_to_cc_path[k], 128, "/mppa/portal/%d:%d", k, cluster_dnoc_rx_port++);
			snprintf(cc_to_io_path[k], 128, "/mppa/portal/%d:%d", mppa_getpid() + k%BSP_NB_DMA_IO, io_dnoc_rx_port++);
	}

	int rank;
	//Opening portal from io to all compute clusters and vice versa
	for (rank = 0; rank < nb_clusters; rank++) {

			// Open a multicast portal to send task groups
			DMSG("Open portal %s\n", io_to_cc_path[rank]);
			if((io_to_cc_fd[rank] = mppa_open(io_to_cc_path[rank], O_WRONLY)) < 0){
				EMSG("Failed to open io_to_cc_portal %d to rank  \n", rank  );
				mppa_exit(1);
			}

			// Set unicast target 'rank'
			if (mppa_ioctl(io_to_cc_fd[rank], MPPA_TX_SET_RX_RANK, rank) < 0) {
				EMSG("Preparing multiplex %d failed!\n", rank);
				mppa_exit(1);
			}

			// Select which interface 'fd' will use to send task groups  to 'rank'
			// We will use mppa_aio_write to perform the transfer: mppa_aio_write automatically select and reserve HW
			// resources at each call. It select between the [4 IO DMA interfaces] * [8 DMA resources] = 32 DMA resources
			// MPPA_TX_SET_IFACE ioctl force mppa_aio_write for this fd to select a DMA resource in the given interface
			// 'rank % BSP_NB_DMA_IO' policy ensure that transfers on one interface don't interfere with transfers of other
			// interfaces.
			if (mppa_ioctl(io_to_cc_fd[rank], MPPA_TX_SET_IFACE, rank % BSP_NB_DMA_IO) < 0) {
				EMSG("Set iface %d failed!\n", rank % BSP_NB_DMA_IO);
				mppa_exit(1);
			}

			// mppa_aio_write needs some HW resources to perform the transfer. If no hardware resource are currently
			// available, default behavior of mppa_aio_write is to return -EAGAIN. In this case, safe blocking call of this
			// function should be done like:
			// while ( ( res = mppa_aio_write(&aiocb) ) == -EAGAIN ) {}
			// if ( res < 0 ) ...
			// Going in and out of MPPAIPC continuously may disturb the system.
			// With MPPA_TX_WAIT_RESOURCE_ON ioctl, further call of mppa_aio_write on this fd will block until an hardware
			// resource is available.
			if (mppa_ioctl(io_to_cc_fd[rank], MPPA_TX_WAIT_RESOURCE_ON, 0) < 0) {
				EMSG("Preparing wait resource %d failed!\n", rank);
				mppa_exit(1);
			}


		DMSG("Opening cc to io portals\n");
		//open portals to transfer tasks from  cc to io
			DMSG("Open portal %s\n", cc_to_io_path[rank]);
			if((cc_to_io_fd[rank] = mppa_open(cc_to_io_path[rank], O_RDONLY)) < 0){
					EMSG("Failed to open io_to_cc_portal %d to rank  \n", rank  );
					mppa_exit(1);
				}

	}

	return 1;
}

int mppa_io_comm_close(int nb_clusters, int *io_to_cc_fd, int *cc_to_io_fd){

	int rank;
	//Opening portal from io to all compute clusters and vice versa
	for (rank = 0; rank < nb_clusters; rank++) {
		mppa_close(io_to_cc_fd[rank]);
		mppa_close(cc_to_io_fd[rank]);
	}
}


int mppa_io_comm_async_send(mppa_aiocb_t *io_to_cc_aiocb, int io_to_cc_fd, void *buf, int buf_size){

	mppa_aiocb_ctor(io_to_cc_aiocb, io_to_cc_fd, buf, buf_size);
	mppa_aiocb_set_pwrite(io_to_cc_aiocb, buf, buf_size, 0);
	int status = mppa_aio_write(io_to_cc_aiocb);
	assert(status == 0);
}

int mppa_io_comm_async_wait(mppa_aiocb_t cc_to_io_aiocb, int buf_size){

	int status = mppa_aio_wait(&cc_to_io_aiocb);
	assert(status == buf_size);
	return 1;
}

int mppa_io_comm_async_receive(mppa_aiocb_t *cc_to_io_aiocb, int cc_to_io_fd, void *buf, int buf_size){

	mppa_aiocb_ctor(cc_to_io_aiocb, cc_to_io_fd, buf, buf_size);
	mppa_aiocb_set_trigger(cc_to_io_aiocb, 1);
	int status = mppa_aio_read(cc_to_io_aiocb);
	assert(status == 0);
	return 1;
}

int mppa_io_comm_send(int io_to_cc_fd, void *buf, int buf_size){

	mppa_aiocb_t io_to_cc_aiocb;
	mppa_io_comm_async_send(&io_to_cc_aiocb, io_to_cc_fd, buf, buf_size);
	mppa_io_comm_async_wait(io_to_cc_aiocb, buf_size);

	return 1;
}


int mppa_io_comm_receive(int cc_to_io_fd, void *buf, int buf_size){

	mppa_aiocb_t cc_to_io_aiocb;
	mppa_io_comm_async_receive(&cc_to_io_aiocb, cc_to_io_fd, buf, buf_size);
	mppa_io_comm_async_wait(cc_to_io_aiocb, buf_size);

	return 1;
}
