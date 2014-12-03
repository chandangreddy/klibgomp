
#include "libgomp.h"
#include <stdlib.h>
#include "serialize_parfor.h"
#include "io_comm.h"

/* This function implements the STATIC scheduling method.  The caller should
   iterate *pstart <= x < *pend.  Return zero if there are more iterations
   to perform; nonzero if not.  Return less than 0 if this thread had
   received the absolutely last iteration.  */

parfor_work_item_t *
kgomp_iter_static_all(int *num_items)
{
	struct gomp_thread *thr = gomp_thread ();
	struct gomp_team *team = thr->ts.team;
	struct gomp_work_share *ws = thr->ts.work_share;
	unsigned long nthreads = team ? team->nthreads : 1;
	*num_items = nthreads;

	if (thr->ts.static_trip == -1)
		return -1;

	parfor_work_item_t *work_item = gomp_malloc(nthreads  * sizeof(parfor_work_item_t));

	/* Quick test for degenerate teams and orphaned constructs.  */
	if (nthreads == 1)
	{
		work_item[0].start = ws->next;
		work_item[0].end = ws->end;
		thr->ts.static_trip = -1;
		return ws->next == ws->end;
	}

	/* We interpret chunk_size zero as "unspecified", which means that we
     should break up the iterations such that each thread makes only one
     trip through the outer loop.  */
	if (ws->chunk_size == 0)
	{
		unsigned long n, q, i, t;
		unsigned long s0, e0;
		long s, e;

		if (thr->ts.static_trip > 0)
			return NULL;

		/* Compute the total number of iterations.  */
		s = ws->incr + (ws->incr > 0 ? -1 : 1);
		n = (ws->end - ws->next + s) / ws->incr;


		//      i = thr->ts.team_id;

		for(i=0;i<nthreads;i++){
			/* Compute the "zero-based" start and end points.  That is, as
                 if the loop began at zero and incremented by one.  */
			q = n / nthreads;
			t = n % nthreads;
			if (i < t)
			{
				t = 0;
				q++;
			}
			s0 = q * i + t;
			e0 = s0 + q;

			/* Notice when no iterations allocated for this thread.  */
			if (s0 >= e0)
			{
				thr->ts.static_trip = 1;
				work_item[i].start  = -1;
				work_item[i].end = -1;
				continue;
			}

			/* Transform these to the actual start and end numbers.  */
			s = (long)s0 * ws->incr + ws->next;
			e = (long)e0 * ws->incr + ws->next;

			work_item[i].start = s;
			work_item[i].end = e;
			thr->ts.static_trip = (e0 == n ? -1 : 1);
		}

		return work_item;
	}
	else
	{
		unsigned long n, s0, e0, i, c;
		long s, e;

		/* Otherwise, each thread gets exactly chunk_size iterations
	 (if available) each time through the loop.  */

		s = ws->incr + (ws->incr > 0 ? -1 : 1);
		n = (ws->end - ws->next + s) / ws->incr;
		c = ws->chunk_size;

		i = thr->ts.team_id;


		for(i=0;i<nthreads;i++){
			/* Initial guess is a C sized chunk positioned nthreads iterations
                 in, offset by our thread number.  */
			s0 = (thr->ts.static_trip * nthreads + i) * c;
			e0 = s0 + c;

			/* Detect overflow.  */
			if (s0 >= n){
				work_item[i].start = -1;
				work_item[i].end = -1;
				continue;
			}
			if (e0 > n)
				e0 = n;

			/* Transform these to the actual start and end numbers.  */
			s = (long)s0 * ws->incr + ws->next;
			e = (long)e0 * ws->incr + ws->next;

			work_item[i].start = s;
			work_item[i].end = e;

			if (e0 == n)
				thr->ts.static_trip = -1;
			else
				thr->ts.static_trip++;

		}

		return work_item;
	}
}

#define KGOMP_NUM_THREADS_PER_CLUSTER 8
#define KGOMP_MAX_CLUSTERS 16

int io_to_cc_fd[KGOMP_MAX_CLUSTERS];
char io_to_cc_path[KGOMP_MAX_CLUSTERS][128];

int cc_to_fd[KGOMP_MAX_CLUSTERS];
char cc_to_io_path[KGOMP_MAX_CLUSTERS][128];

char  sync_io_to_cc_path[128];
int sync_io_to_cc_fd;

char sync_cc_to_io_path[128];
int sync_cc_to_io_fd;

inline
int kgomp_get_num_clusters(int nthreads){

	return nthreads/KGOMP_NUM_THREADS_PER_CLUSTER;
}

int kgomp_init(int nthreads){

	int nclusters = kgomp_get_num_clusters(nthreads);
	mppa_io_comm_init(nclusters, io_to_cc_path, cc_to_io_path, io_to_cc_fd, cc_to_fd);


}

int kgomp_send_workitems(int *cluster_handles, parfor_work_item_t *work_items, int nthreads){

	assert(cluster_handles != NULL);
	assert(work_items != NULL);
	assert(nthreads > 0);

	int nclusters = nthreads / KGOMP_NUM_THREADS_PER_CLUSTER;
	int i;

	int cluster_items_start, cluster_items_end,  num_cluster_items;
	for(i=0;i<nclusters;i++){
		cluster_items_start =  i * KGOMP_NUM_THREADS_PER_CLUSTER;

		cluster_items_end = ( i+1) * KGOMP_NUM_THREADS_PER_CLUSTER;
		if(cluster_items_end > nthreads)
			cluster_items_end = nthreads;

		num_cluster_items = cluster_items_end - cluster_items_start;

		int buf_size = 0;
		char *buf = kgomp_serialize_parfor(work_items[cluster_items_start], num_cluster_items, &buf_size);

		mppa_io_comm_send(cluster_handles[i], buf, buf_size);
	}

	return 1;
}
