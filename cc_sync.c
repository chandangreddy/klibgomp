#include "cc_sync.h"

// Initialization of the barrier
static int
mppa_cc_init_barrier( int *sync_io_to_cluster_fd,
		int *sync_clusters_to_io_fd, int nb_clusters)
{

	int cluster_dnoc_rx_port = 1;
	int io_cnoc_rx_port = 1;
	int cluster_cnoc_rx_port = 1;

	char sync_io_to_cc_path[128];
	snprintf(sync_io_to_cc_path, 128, "/mppa/sync/[0..%lu]:%d", nb_clusters - 1, cluster_cnoc_rx_port++);
	char sync_cc_to_io_path[128];
	snprintf(sync_cc_to_io_path, 128, "/mppa/sync/%d:%d", mppa_getpid(), io_cnoc_rx_port++);

	*sync_io_to_cluster_fd = mppa_open(sync_io_to_cc_path, O_RDONLY);
	if (*sync_io_to_cluster_fd < 0) {
		EMSG("Opening %s failed!\n", sync_io_to_cc_path);
		return 1;
	}

	unsigned long long match = -(1LL << 1);
	if (mppa_ioctl(*sync_io_to_cluster_fd, MPPA_RX_SET_MATCH, match) < 0) {
		EMSG("ioctl MPPA_RX_SET_MATCH failed on %s\n", sync_io_to_cc_path);
		return 1;
	}

	*sync_clusters_to_io_fd = mppa_open(sync_cc_to_io_path, O_WRONLY);
	if (*sync_clusters_to_io_fd < 0) {
		EMSG("Opening %s failed!\n", sync_cc_to_io_path);
		return 1;
	}

	return 0;
}


// Barrier between IO Cluster and all Clusters
static int
mppa_cc_barrier(int sync_io_to_cluster_fd, int sync_clusters_to_io_fd)
{
	//DMSG("[Cluster %d] mppa_barrier...", mppa_getpid());
	int rank = mppa_getpid();
	long long mask = 1ULL << rank;
	if (mppa_write(sync_clusters_to_io_fd, &mask, sizeof(mask)) != sizeof(mask)) {
		EMSG("mppa_write barrier failed!\n");
		return 1;
	}

	unsigned long long dummy;
	if (mppa_read(sync_io_to_cluster_fd, &dummy, sizeof(dummy)) != sizeof(unsigned long long)) {
		EMSG("mppa_read barrier failed!\n");
		return 1;
	}

	//DMSG("done\n");
	return 0;
}

// Close barrier connectors
static void
mppa_cc_close_barrier(int sync_io_to_cluster_fd, int sync_clusters_to_io_fd)
{
	mppa_close(sync_clusters_to_io_fd);
	mppa_close(sync_io_to_cluster_fd);
}
