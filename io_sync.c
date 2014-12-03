#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/wait.h>
#include <errno.h>

#include <mppa_bsp.h>
#include <mppaipc.h>
#include <mppa/osconfig.h>

// Initialization of the barrier (must be called before mppa_spawn)
static int
mppa_io_init_barrier( char *sync_io_to_cc_path, int *sync_io_to_cluster_fd,
		char *sync_cc_to_io_path , int *sync_clusters_to_io_fd, int nb_clusters)
{

	int cluster_dnoc_rx_port = 1;
	int io_cnoc_rx_port = 1;
	int cluster_cnoc_rx_port = 1;

	snprintf(sync_io_to_cc_path, 128, "/mppa/sync/[0..%lu]:%d", nb_clusters - 1, cluster_cnoc_rx_port++);
	snprintf(sync_cc_to_io_path, 128, "/mppa/sync/%d:%d", mppa_getpid(), io_cnoc_rx_port++);

	// Open IO to Clusters sync connector
	*sync_io_to_cluster_fd = mppa_open(sync_io_to_cc_path, O_WRONLY);
	if (*sync_io_to_cluster_fd < 0) {
		EMSG("Opening %s failed!\n", sync_io_to_cc_path);
		return 1;
	}
	int ranks[BSP_NB_CLUSTER_MAX];
	int i;
	for (i = 0; i < nb_clusters; ++i) ranks[i] = i;

	if (mppa_ioctl(*sync_io_to_cluster_fd, MPPA_TX_SET_RX_RANKS, nb_clusters, ranks) < 0) {
		EMSG("ioctl MPPA_TX_SET_RX_RANK on sync_io_to_cluster_fd failed!\n");
		return 1;
	}

	// Open Clusters to IO sync connector
	*sync_clusters_to_io_fd = mppa_open(sync_cc_to_io_path, O_RDONLY);
	if (*sync_clusters_to_io_fd < 0) {
		EMSG("Opening %s failed!\n", sync_cc_to_io_path);
		return 1;
	}
	unsigned long long match = ~((0x1ULL << nb_clusters) - 1);
	if (mppa_ioctl(*sync_clusters_to_io_fd, MPPA_RX_SET_MATCH, match) < 0) {
		EMSG("ioctl MPPA_RX_SET_MATCH failed on %s\n", sync_cc_to_io_path);
		return 1;
	}

	return 0;

}

// Barrier between IO Cluster and all Clusters
static int
mppa_io_barrier(int sync_io_to_cluster_fd, int sync_clusters_to_io_fd)
{
	DMSG("[Cluster I/O] mppa_barrier...");

	unsigned long long dummy;
	if (mppa_read(sync_clusters_to_io_fd, &dummy, sizeof(dummy)) != sizeof(unsigned long long)) {
		EMSG("Read sync_clusters_to_io_fd failed!\n");
		return 1;
	}
	unsigned long long mask = 1LL;
	if (mppa_write(sync_io_to_cluster_fd, &mask, sizeof(mask)) != sizeof(mask)) {
		EMSG("write sync_io_to_cluster_fd failed!\n");
		return 1;
	}
	DMSG(" done!\n");

	return 0;

}

// Close barrier connectors
static void
mppa_io_close_barrier(int sync_io_to_cluster_fd, int sync_clusters_to_io_fd)
{
	mppa_close(sync_clusters_to_io_fd);
	mppa_close(sync_io_to_cluster_fd);
}

