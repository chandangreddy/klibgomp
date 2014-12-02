
#include "libgomp.h"
#include <stdlib.h>
#include "serialize_parfor.h"

/* This function implements the STATIC scheduling method.  The caller should
   iterate *pstart <= x < *pend.  Return zero if there are more iterations
   to perform; nonzero if not.  Return less than 0 if this thread had
   received the absolutely last iteration.  */

parfor_work_item_t *
kgomp_iter_static_all()
{
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_team *team = thr->ts.team;
  struct gomp_work_share *ws = thr->ts.work_share;
  unsigned long nthreads = team ? team->nthreads : 1;

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


      i = thr->ts.team_id;

      for(i=0;i<numthreads;i++){
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
                  return 1;
                }

              /* Transform these to the actual start and end numbers.  */
              s = (long)s0 * ws->incr + ws->next;
              e = (long)e0 * ws->incr + ws->next;

              *pstart = s;
              *pend = e;
              thr->ts.static_trip = (e0 == n ? -1 : 1);

      }

      return 0;
    }
  else
    {
      unsigned long n, s0, e0, i, c;
      long s, e;

      /* Otherwise, each thread gets exactly chunk_size iterations
	 (if available) each time through the loop.  */

      s = ws->incr + (ws->incr > 0 ? -1 : 1);
      n = (ws->end - ws->next + s) / ws->incr;
      i = thr->ts.team_id;
      c = ws->chunk_size;

      /* Initial guess is a C sized chunk positioned nthreads iterations
	 in, offset by our thread number.  */
      s0 = (thr->ts.static_trip * nthreads + i) * c;
      e0 = s0 + c;

      /* Detect overflow.  */
      if (s0 >= n)
	return 1;
      if (e0 > n)
	e0 = n;

      /* Transform these to the actual start and end numbers.  */
      s = (long)s0 * ws->incr + ws->next;
      e = (long)e0 * ws->incr + ws->next;

      *pstart = s;
      *pend = e;

      if (e0 == n)
	thr->ts.static_trip = -1;
      else
	thr->ts.static_trip++;
      return 0;
    }
}
