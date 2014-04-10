#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#include "r_commu.h"

char buffer[1500];
void *commu_thread(void *data)
{
    int m_fd;
    struct sockaddr_in m_addr;
    struct hostent *server_host_name;

    m_fd = socket(PF_INET, SOCK_STREAM, 0);
    
    if(m_fd < 0)
    {
        printf("Socket failed\n");
        return NULL;
    }
    
    server_host_name = gethostbyname("10.44.124.127");
    
    memset((void *)&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_addr.sin_addr.s_addr = ((struct in_addr *)(server_host_name->h_addr))->s_addr;
    m_addr.sin_port = htons(8888);
    
    while(connect(m_fd, (void *)&m_addr, sizeof(m_addr)) < 0)
    {
        sleep(10);
    }

    for(;;)
    {    
        recv(m_fd, (void *)buffer, 1500, MSG_WAITALL);
        printf("Packet Received!\n");
    }

    send(m_fd, "test data", strlen("test data") + 1, 0);
    
    close(m_fd);

    return NULL;
}

