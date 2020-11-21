#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 1000

int buffer_max_size = 64;

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



SFF_buffer std = {.curr_size=0};
SFF_buffer *s = &std;
void main()
{
    insertSFF(s,1,"Wroooom",2);
    printRequest(s->buff[0]);
    insertSFF(s,2,"Sam",1);
    printRequest(s->buff[0]);
    insertSFF(s,3,"Tejas",3);
    int i;
    printf("LOOP:\n");
    for(i=0;i<3;i++)
    {
        request r;
        pullSFF(s,&r);
        printRequest(r);
    }
}