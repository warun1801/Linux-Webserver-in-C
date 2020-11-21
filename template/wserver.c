//
//	Main webserver code file (with main() fuction)
//

#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>

char default_root[] = ".";

//
// ./wserver [-d basedir] [-p port] [-t threads] [-b buffersize] [-s schedalg (0 - FIFO, 1 - SFF)]
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
    
	// below default values are defined in 'request.h'
    num_threads = DEFAULT_THREADS;
    buffer_max_size = DEFAULT_BUFFER_SIZE;
    scheduling_algo = DEFAULT_SCHED_ALGO;	
    
	// fetch (and set) values from command line arguments
    while ((c = getopt(argc, argv, "d:p:t:b:s:")) != -1)
		switch (c) {
			case 'd':
				root_dir = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'b':
				buffer_max_size = atoi(optarg);
				break;
			case 's':
				scheduling_algo = atoi(optarg);
				break;
			default:
				fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffersize] [-s schedalg (0 - FIFO, 1 - SFF)]\n");
				exit(1);
		}

    // browse to webserver's root directory
    chdir_or_die(root_dir);

	// create the thread pool
	pthread_t thread_pool[num_threads];
	for(int i=0; i<num_threads; i++)
    	pthread_create(&thread_pool[i], NULL, thread_request_serve_static, NULL);

	buffer_size = 0;	// initial buffer size
	
    // open the socket connection
    int listen_fd = open_listen_fd_or_die(port);
    while (1) {
		
		// start accepting HTTP requests from client
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		
		// process the HTTP request
		request_handle(conn_fd);
		
		// close the connection to client
		//close_or_die(conn_fd);	// this has to be carefully done at the correct time in request.c
    }
    
    return 0;
}
