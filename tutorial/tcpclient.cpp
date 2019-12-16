#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include "../coroutine.h"

#define handler_error(msg) do{ perror(msg); exit(EXIT_FAILURE);}while(0)
#define PORT 1234

void tcpclient()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
        handler_error("socket");
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    connect(fd, (struct sockaddr *)&addr, sizeof(addr));

    const char *msg = "hello world";
    write(fd, msg, strlen(msg));
    char buf[512];
    ssize_t ret = read(fd, buf, 512);
    if(ret)
        printf("read from server %ld(byte) content %s\n", ret, buf);

    close(fd);
}

int main(int argc, char *argv[])
{
//    tcpclient();
    co_create(tcpclient);
    co_sched().start(1);
}

