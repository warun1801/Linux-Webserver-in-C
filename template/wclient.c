//
// wclient.c: A very, very primitive HTTP client.
// 
// To run, try: 
//      ./wclient hostname portnumber filepath ('filepath' is the absolute path from the root directory of the server)
//
// Sends one HTTP request to the specified HTTP server.
// Prints out the HTTP response.
//
// When we test your server, we will be using this client.
//

#include "io_helper.h"

#define MAXBUF (8192)

//
// Send an HTTP request for the specified file 
//
void client_send(int fd, char *filename) {
    char buf[MAXBUF];
    char hostname[MAXBUF];
    
    gethostname_or_die(hostname, MAXBUF);
    
    /* Form and send the HTTP request */
    sprintf(buf, "GET %s HTTP/1.1\n", filename);
    sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
    write_or_die(fd, buf, strlen(buf));
}

//
// Read the HTTP response and print it out
//
void client_print(int fd) {
    char buf[MAXBUF];  
    int n;
    
    // Read and display the HTTP Header 
    n = readline_or_die(fd, buf, MAXBUF);
    while (strcmp(buf, "\r\n") && (n > 0)) {
		printf("Header: %s", buf);
		n = readline_or_die(fd, buf, MAXBUF);
    }
    
    // Read and display the HTTP Body 
    n = readline_or_die(fd, buf, MAXBUF);
    while (n > 0) {
		printf("%s", buf);
		n = readline_or_die(fd, buf, MAXBUF);
    }
}

int main(int argc, char *argv[]) {
    char *host, *filename;
    int port;
    int clientfd;
    
    if (argc != 4) {
		fprintf(stderr, "Usage: %s <host> <port> <filepath>\n", argv[0]);
		exit(1);
    }
    
    host = argv[1];
    port = atoi(argv[2]);
    filename = argv[3];
    
    /* Open a single connection to the specified host and port */
    clientfd = open_client_fd_or_die(host, port);
    
    client_send(clientfd, filename);
    client_print(clientfd);
    
    close_or_die(clientfd);
    
    exit(0);
}
