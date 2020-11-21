#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#define MAX 1000

typedef struct __stack
{
    char arr[MAX][100];
    int top;
}stack;

void push(stack **stk,char *s)
{
    if((*stk)->top == MAX)
    {
        return;
    }
    strcpy((*stk)->arr[(*stk)->top],s);
    (*stk)->top++;
}

void pop(stack **stk,char ret[][100])
{
    if((*stk)->top == 0)
    {
        return;
    }
    (*stk)->top--;
    strcpy(*ret,(*stk)->arr[(*stk)->top]);
}

void initStack(stack **stk)
{
    (*stk) = (stack *)malloc(sizeof(stack));
    (*stk)->top = 0;
}


void shortenPath(char * path) 
{ 
    stack *s;
    initStack(&s);
    char *dir;
    int dir_idx; 
    char res[100];
    res[0] = '/';
    res[1] = '\0';  
    int len = strlen(path);
    int i;
    char r[100];
    for (int i = 0; i < len; i++)
    { 
        dir_idx = 0;
        dir = (char *)malloc((len+1)*sizeof(char));
        while (path[i] == '/') 
            i++;
        while (i < len && path[i] != '/') 
            dir[dir_idx++] = path[i++];
        dir[dir_idx] = '\0'; 
        if (strstr(dir,"..") != NULL)
        { 
            if (s->top!=0)  
                pop(&s,&r);
        }
        else if (strstr(dir,".") != NULL)  
            continue; 
        else if (strlen(dir) != 0)
        {
            push(&s,dir);         
        }
        free(dir);
        dir = NULL;
    } 
    stack *st1;
    initStack(&st1); 
    while (!(s->top == 0)) { 
        push(&st1,s->arr[s->top-1]); 
        pop(&s,&r);
    }
    while (st1->top!=0) 
    { 
        char *temp = strdup(st1->arr[st1->top-1]);
        char c = '/'; 
        if (st1->top != 1)
        {
            strncat(temp,&c,1);
            strcat(res,temp);
        }
        else
        {
            strcat(res,temp); 
        }
        pop(&st1,&r);
    } 
    puts(res); 
} 
  
// Driver code. 
int main() 
{ 
    char *str = "/files/test1.html"; 
    shortenPath(str);
    return 0; 
} 


