#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/wait.h>
#include <errno.h>

#include <mppa_bsp.h>
#include <mppaipc.h>
#include "libkgomp_util.h"

int mppa_cc_comm_init(char *io_to_cc_path, char *cc_to_io_path, int *cc_to_io_fd, int *io_to_cc_fd);
void mppa_cc_comm_close(int cc_to_io_fd, int io_to_cc_fd);
int mppa_cc_comm_async_send(mppa_aiocb_t *cc_to_io_aiocb, int cc_to_io_fd, void *buf, int buf_size);
int mppa_cc_comm_async_wait(mppa_aiocb_t io_to_cc_aiocb, int buf_size);
int mppa_cc_comm_async_receive(mppa_aiocb_t *io_to_cc_aiocb, int io_to_cc_fd, void *buf, int buf_size);
int mppa_cc_comm_send(int cc_to_io_fd, void *buf, int buf_size);
int mppa_cc_comm_receive(int io_to_cc_fd, void *buf, int buf_size);

