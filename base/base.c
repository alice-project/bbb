#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BASE_PORT    8888

char buffer[1500];
void *hamster_func(void *data)
{
    int fd = *(int *)data;
    if(fd < 0)
    {
        printf("fd error\n");
        return;
    }

    while(recv(fd, buffer, 1500, MSG_WAITALL) <= 0);

    printf("received data from socket:%d\n", fd);
}

int main(int argc, char *argv[])
{
    pthread_t hamster_thread;
    int s_fd, c_fd;
    struct sockaddr_in s_addr, c_addr;

    s_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(s_fd <= 0)
    {
        printf("socket failed\n");
        return -1;
    }

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = htons(INADDR_ANY);
    s_addr.sin_port = htons(BASE_PORT);

    if(bind(s_fd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
    {
        printf("bind failed\n");
        return -1;
    }

    if(listen(s_fd,SOMAXCONN) < 0)
    {
        printf("listen failed!");
        return -1;
    }

    for(;;)
    {
        socklen_t socklen = sizeof(c_addr);
        c_fd = accept(s_fd, (struct sockaddr*)&c_addr, &socklen);
        if(c_fd < 0)
        {
            printf("accept failed");
            close(c_fd);
            close(s_fd);
            return -1;
        }
        else
        {
            if(pthread_create(&hamster_thread, NULL, hamster_func, &c_fd) < 0)
            {
                printf("pthread create failed\n");
            }
        }
        sleep(1);
    }

    pthread_join(hamster_thread, 0);
    close(c_fd);
    close(s_fd);
    return 0;
}

