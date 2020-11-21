#include "io_helper.h"
#include "request.h"
#define MAXBUF (8192)
#define MAX (1000)

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

//
//	TODO: add code to create and manage the buffer
//
//? Request struct and methods----------------------------------------
typedef struct __request
{
  int fd;
  char *filename;
  int filesize;
} request;

void newRequest(request *r,int fd, char *filename, int filesize)
{
  r->filename = strdup(filename);
  r->fd = fd;
  r->filesize = filesize;
}

void printRequest(request r)
{
    printf("Request: fd = %d, filename = %s, filesize = %d\n",r.fd,r.filename,r.filesize);
}

void requestToRequest(request *t,request s)
{
    t->fd = s.fd;
    t->filename = strdup(s.filename);
    t->filesize = s.filesize;
}
//? Request ends here--------------------------------------------------

//TODO : First In First Out
// * FIFO Starts here--------------------------------------------------

typedef struct __circ_buffer
{
  int front;
  int rear;
  int curr_size;
  request buff[MAX];
} FIFO_buffer;

void insertFIFO(FIFO_buffer *buff, int fd, char *filename, int filesize)
{
    if((buff->front == 0 && buff->rear == buffer_max_size-1) || (buff->rear == (buff->front-1)%(buffer_max_size-1)))
    {
        buff->curr_size = buffer_max_size;
        printf("Buffer is full\n");
        return;
    }
    else if(buff->front == -1)
    {
        buff->front = buff->rear =  0;
        buff->curr_size++;
        newRequest(&buff->buff[buff->rear],fd,filename,filesize);
    }
    else if (buff->rear == buffer_max_size-1 && buff->front != 0)
    {
        buff->curr_size++;
        buff->rear = 0;
        newRequest(&buff->buff[buff->rear],fd,filename,filesize);
    }
    else
    {
        buff->curr_size++;
        buff->rear++;
        newRequest(&buff->buff[buff->rear],fd,filename,filesize);
    }
}

void pullFIFO(FIFO_buffer *buff,request *r)
{
    if(buff->front == -1)
    {
        buff->curr_size = 0;
        printf("Buffer is empty\n");
        return;
    }

    newRequest(r,buff->buff[buff->front].fd,buff->buff[buff->front].filename,buff->buff[buff->front].filesize);
    free(buff->buff[buff->front].filename);

    if(buff->front == buff->rear)
    {
        buff->curr_size = 0;
        buff->front = -1;
        buff->rear = -1;
    }
    else if(buff->front == buffer_max_size-1)
    {
        buff->curr_size--;
        buff->front = 0;
    }
    else
    {
        buff->curr_size--;
        buff->front++;
    }
}
//* FIFO ends here-------------------------------------------------------




//TODO : Shortest File First
// *SFF Starts here------------------------------------------------------
typedef struct __heap
{
    request buff[MAX];
    int curr_size;
}SFF_buffer;


void SFFHeapifer(SFF_buffer *s,int idx)
{
    int min = idx;
    if(2*idx+1 < s->curr_size && s->buff[2*idx+1].filesize < s->buff[min].filesize)
        min = 2*idx+1;
    if(2*idx+2 < s->curr_size && s->buff[2*idx+2].filesize < s->buff[min].filesize)
        min = 2*idx+2;
    if(min==idx)
        return;
    else
    {
        request t;
        requestToRequest(&t,s->buff[min]);
        requestToRequest(&s->buff[min],s->buff[idx]);
        requestToRequest(&s->buff[idx],t);
        SFFHeapifer(s,min);
    }

}

void insertSFF(SFF_buffer *s,int fd,char *filename,int filesize)
{
    if(s->curr_size == buffer_max_size)
    {
        printf("Buffer is full\n");
        return;
    }

    newRequest(&s->buff[s->curr_size],fd,filename,filesize);
    int up = s->curr_size++;

    while (up>0 && s->buff[(up-1)/2].filesize > s->buff[up].filesize)
    {
        request t;
        requestToRequest(&t,s->buff[(up-1)/2]);
        requestToRequest(&s->buff[(up-1)/2],s->buff[up]);
        requestToRequest(&s->buff[up],t);
        up = (up-1)/2;
    }
}

void pullSFF(SFF_buffer *s,request *r)
{
    if(s->curr_size  <=0)
    {
        printf("Buffer is empty\n");
        return;
    }
    requestToRequest(r,s->buff[0]);
    s->curr_size--;
    if(s->curr_size!=0)
    {
        requestToRequest(&s->buff[0],s->buff[s->curr_size]);
        SFFHeapifer(s,0);
    }
}

//* SFF ends here-----------------------------------------------

// ! Global buffers for FIFO and SFF ---------------------------
FIFO_buffer effort = { .front = -1, .rear=-1, .curr_size=0 };
FIFO_buffer *f = &effort;

SFF_buffer std = {.curr_size=0};
SFF_buffer *s = &std;
// !-------------------------------------------------------------
//
// Sends out HTTP response in case of errors
//
void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXBUF], body[MAXBUF];

    // Create the body of error message first (have to know its length for header)
    sprintf(body, ""
	    "<!doctype html>\r\n"
	    "<head>\r\n"
	    "  <title>OSTEP WebServer Error</title>\r\n"
	    "</head>\r\n"
	    "<body>\r\n"
	    "  <h2>%s: %s</h2>\r\n"
	    "  <p>%s: %s</p>\r\n"
	    "</body>\r\n"
	    "</html>\r\n", errnum, shortmsg, longmsg, cause);

    // Write out the header information for this response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    write_or_die(fd, buf, strlen(buf));

    sprintf(buf, "Content-Type: text/html\r\n");
    write_or_die(fd, buf, strlen(buf));

    sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
    write_or_die(fd, buf, strlen(buf));

    // Write out the body last
    write_or_die(fd, body, strlen(body));

    // close the socket connection
    close_or_die(fd);
}

//
// Reads and discards everything up to an empty text line
//
void request_read_headers(int fd) {
    char buf[MAXBUF];

    readline_or_die(fd, buf, MAXBUF);
    while (strcmp(buf, "\r\n")) {
		readline_or_die(fd, buf, MAXBUF);
    }
    return;
}

//
// Return 1 if static, 0 if dynamic content (executable file)
// Calculates filename (and cgiargs, for dynamic) from uri
//
int request_parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;

    if (!strstr(uri, "cgi")) {
	// static
	strcpy(cgiargs, "");
	sprintf(filename, ".%s", uri);
	if (uri[strlen(uri)-1] == '/') {
	    strcat(filename, "index.html");
	}
	return 1;
    } else {
	// dynamic
	ptr = index(uri, '?');
	if (ptr) {
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	} else {
	    strcpy(cgiargs, "");
	}
	sprintf(filename, ".%s", uri);
	return 0;
    }
}

//
// Fills in the filetype given the filename
//
void request_get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
    else
		strcpy(filetype, "text/plain");
}

//
// Handles requests for static content
//
void request_serve_static(int fd, char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXBUF], buf[MAXBUF];

    request_get_filetype(filename, filetype);
    srcfd = open_or_die(filename, O_RDONLY, 0);

    // Rather than call read() to read the file into memory,
    // which would require that we allocate a buffer, we memory-map the file
    srcp = mmap_or_die(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close_or_die(srcfd);

    // put together response
    sprintf(buf, ""
	    "HTTP/1.0 200 OK\r\n"
	    "Server: OSTEP WebServer\r\n"
	    "Content-Length: %d\r\n"
	    "Content-Type: %s\r\n\r\n",
	    filesize, filetype);

    write_or_die(fd, buf, strlen(buf));

    //  Writes out to the client socket the memory-mapped file
    write_or_die(fd, srcp, filesize);
    munmap_or_die(srcp, filesize);
}

//
// Fetches the requests from the buffer and handles them (thread locic)
//
void* thread_request_serve_static(void* arg)
{
	// TODO: write code to actualy respond to HTTP requests
    int i;
    while(1)
    {
        sleep(1);
        pthread_mutex_lock(&mutex);
        if(scheduling_algo)
        {
            while(s->curr_size == 0)
                pthread_cond_wait(&full,&mutex);
        }
        else
        {
            while(f->curr_size == 0)
                pthread_cond_wait(&full,&mutex);
        }
        request r;
        if(scheduling_algo)
            pullSFF(s,&r);
        else
            pullFIFO(f,&r);
        printf("Request for %s is removed from the buffer\n",r.filename);

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);

        // *Actual request serving
        request_serve_static(r.fd,r.filename,r.filesize);
        close_or_die(r.fd);
        // *----------------------
    }

}

//
// Initial handling of the request
//
void request_handle(int fd) {
    int is_static;
    struct stat sbuf;
    char buf[MAXBUF], method[MAXBUF], uri[MAXBUF], version[MAXBUF];
    char filename[MAXBUF], cgiargs[MAXBUF];

	// get the request type, file path and HTTP version
    readline_or_die(fd, buf, MAXBUF);
    sscanf(buf, "%s %s %s", method, uri, version);
    printf("method:%s uri:%s version:%s\n", method, uri, version);

	// verify if the request type is GET is not
    if (strcasecmp(method, "GET")) {
		request_error(fd, method, "501", "Not Implemented", "server does not implement this method");
		return;
    }
    request_read_headers(fd);

	// check requested content type (static/dynamic)
    is_static = request_parse_uri(uri, filename, cgiargs);

    // TODO: code for security check
    if(strstr(filename, "..") != NULL){
        request_error(fd, filename, "403", "Forbidden", "Traversing up in filesystem is not allowed");
        return;
    }
	// get some data regarding the requested file, also check if requested file is present on server
    if (stat(filename, &sbuf) < 0) {
		request_error(fd, filename, "404", "Not found", "server could not find this file");
		return;
    }

	// verify if requested content is static
    if (is_static) {
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
			request_error(fd, filename, "403", "Forbidden", "server could not read this file");
			return;
		}

		// TODO: write code to add HTTP requests in the buffer based on the scheduling policy
        pthread_mutex_lock(&mutex);
        if(scheduling_algo)
        {
            while(s->curr_size == buffer_max_size )
                pthread_cond_wait(&empty,&mutex);
            insertSFF(s,fd,filename,sbuf.st_size);
        }
        else
        {
            while(f->curr_size == buffer_max_size )
                pthread_cond_wait(&empty,&mutex);
            insertFIFO(f,fd,filename,sbuf.st_size);
        }
        printf("Request for %s is added to the buffer\n",filename);
        if(scheduling_algo)
            printf("Added size SFF = %d\n",s->curr_size);
        else
            printf("Added size FIFO = %d\n",f->curr_size);
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);

    } else {
		request_error(fd, filename, "501", "Not Implemented", "server does not serve dynamic content request");
    }
}
