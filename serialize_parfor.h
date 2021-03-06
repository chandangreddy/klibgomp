#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "libkgomp_util.h"

struct  parfor_work_item {
	//start of parallel loop chunk
	int start;
	//end of parallel loop chunk
	int end;
	//id of the thread which should execute this work
	int thread_id;
	//function id to select the function to execute
	int function_id;
};

typedef struct parfor_work_item parfor_work_item_t;

char* kgomp_serialize_parfor(parfor_work_item_t *pfor, int num, int *buf_size);
parfor_work_item_t* kgomp_deserialize_parfor(char *buf, int *num);
