#ifndef __REQUEST_H__

#define DEFAULT_BUFFER_SIZE 64
#define DEFAULT_THREADS 4
#define DEFAULT_SCHED_ALGO 0		// 0 - FIFO, 1 - SFF

int buffer_max_size;
int buffer_size;
int scheduling_algo;
int num_threads;

void request_handle(int fd);
void* thread_request_serve_static(void* arg);

#endif // __REQUEST_H__
