/*
 * seralize_parfor.c
 *
 *  Created on: Dec 2, 2014
 *      Author: accesscore
 */
#include "serialize_parfor.h"


char* serialize_parfor(parfor_work_item_t *pfor, int num, int *buf_size){

	//size of parfor struct and size of buf_size
	int total_size = sizeof(parfor_work_item_t) * num + sizeof(int);
	*buf_size = total_size;

	char *buf = (char *)malloc(total_size);
	assert(buf != NULL);

	char *buf_ptr = buf;
	memcpy(buf_ptr, &num, sizeof(int));
	buf_ptr += sizeof(int);

	memcpy(buf_ptr, pfor, sizeof(parfor_work_item_t)*num);

	return buf;
}

parfor_work_item_t* deserialize_parfor(char *buf, int *num){

	assert(buf != NULL);

	int total_num = 0;
	//read the num of structs
	memcpy(&total_num, buf, sizeof(int));
	buf += sizeof(int);

	assert(total_num >0);
	int size = sizeof(parfor_work_item_t) * total_num;
	parfor_work_item_t *pfor = (parfor_work_item_t)malloc(size);
	memcpy(pfor, buf, size);

	*num = total_num;
	return pfor;
}
