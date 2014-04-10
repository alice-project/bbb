#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


struct sockaddr_in peeraddr;
struct in_addr ia;
int sockfd;
char recmsg[BUFLEN + 1];
unsigne int socklen, n;
struct hostent *group;
struct ip_mreq mreq;






/* 绑定自己的端口和IP信息到socket上 */
if (bind
(sockfd, (struct sockaddr *) &peeraddr,
sizeof(struct sockaddr_in)) == -1) {
printf("Bind error\n");
exit(0);
}

/* 循环接收网络上来的组播消息 */
for (;;) {
bzero(recmsg, BUFLEN + 1);
n = recvfrom(sockfd, recmsg, BUFLEN, 0,
(struct sockaddr *) &peeraddr, &socklen);
if (n < 0) {
printf("recvfrom err in udptalk!\n");
exit(4);
} else {
/* 成功接收到数据报 */
recmsg[n] = 0;
printf("peer:%s", recmsg);
}
}
} 

int main(int argc, char *argv[])
{
    int s_fd;
    struct hostent *server_host_name;
    struct sockaddr_in s_addr;
    struct ip_mreq m_req;
    struct hostent *m_group;
    struct in_addr ia;

    /* Multicast */
    s_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(s_fd < 0)
    {
        printf("Socket failed\n");
        return -1;
    }

    memset(&m_req, 0, sizeof(m_req));
    m_group = gethostbyname("224.0.0.1");

    memcpy((void *)m_group->h_addr, (void *) &ia, m_group->h_length);
    memcpy(&ia, &m_req.imr_multiaddr.s_addr, sizeof(ia));
    mm_req.imr_interface.s_addr = htonl(INADDR_ANY);
    
    if (setsockopt(s_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m_req, sizeof(m_req)) == -1) {
        printf("setsockopt failed\n");
        return -1;
    }

    socklen = sizeof(struct sockaddr_in);
    memset(&peeraddr, 0, socklen);
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(atoi(8888));
    if (inet_pton(AF_INET, argv[1], &peeraddr.sin_addr) <= 0) 
    {
        printf("Wrong dest IP address!\n");
        exit(0);
    }
    /* Multicast */
    
    s_fd = socket(PF_INET, SOCK_STREAM, 0);
    
    if(s_fd < 0)
    {
        printf("Socket failed\n");
        return -1;
    }
    
    server_host_name = gethostbyname("10.44.124.127");
    
    memset((void *)&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_addr.s_addr = ((struct in_addr *)(server_host_name->h_addr))->s_addr;
    s_addr.sin_port = htons(8888);
    
    connect(s_fd, (void *)&s_addr, sizeof(s_addr));
    
    send(s_fd, "test data", strlen("test data") + 1, 0);
    
    close(s_fd);
    
    return 0;
}
