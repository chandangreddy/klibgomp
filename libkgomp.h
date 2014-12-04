#pragma once

#include <mppa_bsp.h>
#include <mppaipc.h>
//#include <mppa/osconfig.h>

#include "libkgomp_util.h"

int kgomp_init(int nthreads);
int KGOMP_loop_static_next();
