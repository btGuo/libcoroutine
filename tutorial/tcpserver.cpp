#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>

#include "header.h"

#define handler_error(msg) do{ perror(msg); exit(EXIT_FAILURE);}while(0)
#define PORT 1234

void tcpserver()
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
        handler_error("socket");
    struct sockaddr_in local;
    bzero(&local, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(PORT);
        
    if(bind(listenfd, (struct sockaddr *)&local, sizeof(local)) < 0)
        handler_error("bind");
    listen(listenfd, 20);

    struct sockaddr_in remote;
    socklen_t len = sizeof(remote);
    for(;;)
    {
        int fd = accept(listenfd, (struct sockaddr *)&remote, &len);
        printf("accept %d\n", fd);
        if(fd == -1)
            handler_error("accept");
    
        co_create([fd]{
            ssize_t ret;
            char buf[512];
            while((ret = read(fd, buf, 512))) 
            {
                if(ret == -1 && errno == EAGAIN)
                    continue;
                printf("read %ld(bytes)\n", ret);
                write(fd, buf, ret);
            }
            if(ret == -1)
                handler_error("read");
            close(fd);
        });
    }

    close(listenfd);
}

int main(int argc, char **argv)
{
//    tcpserver();
    co_create(tcpserver);
    co_sched().start(1);
}

