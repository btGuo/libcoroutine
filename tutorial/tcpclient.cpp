#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <coroutine/coroutine.h>

using namespace std;


#define handler_error(msg) do{ perror(msg); exit(EXIT_FAILURE);}while(0)
#define PORT 1234

void tcpclient()
{
    printf("client start\n");
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
        handler_error("socket");
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    int ret;
    connect(fd, (struct sockaddr *)&addr, sizeof(addr));

    char buf[512];
    for(;;)
    {
        scanf("%s", buf);
        if(strcmp(buf, "exit") == 0)
            break;
        if((ret = write(fd, buf, strlen(buf))) == -1)
            handler_error("write");

        if((ret = read(fd, buf, 512)) == -1)
            handler_error("read");
        buf[ret] = '\0';
        printf("echo %s\n", buf);
    }
    printf("client exit\n");
    close(fd);
}

int main(int argc, char *argv[])
{
//    tcpclient();
    co_create(tcpclient);
    co_sched().start(1);
}

