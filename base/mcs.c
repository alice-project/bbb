#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

#include "../pub/pub.h"

#define BUFLEN 255
int main(int argc, char **argv)
{
    struct sockaddr_in peeraddr;
    struct in_addr ia;
    int sockfd;
    char recmsg[BUFLEN + 1];
    unsigned int socklen, n;
    struct hostent *group;
    struct ip_mreqn mreq;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("socket creating err in udptalk\n");
        exit(1);
    }
    
    bzero(&mreq, sizeof(struct ip_mreqn));
        if ((group = gethostbyname(MUL_IPADDR)) == (struct hostent *) 0) {
            perror("gethostbyname");
            exit(errno);
        }
    
    bcopy((void *) group->h_addr, (void *) &ia, group->h_length);
    bcopy(&ia, &mreq.imr_multiaddr.s_addr, sizeof(struct in_addr));
    
    mreq.imr_address.s_addr = htonl(INADDR_ANY);
    
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreqn)) == -1) {
        perror("setsockopt");
        exit(-1);
    }
    
    socklen = sizeof(struct sockaddr_in);
    memset(&peeraddr, 0, socklen);
    peeraddr.sin_family = AF_INET;
        peeraddr.sin_port = htons(BASE_PORT);

        if (inet_pton(AF_INET, MUL_IPADDR, &peeraddr.sin_addr) <= 0) {
            printf("Wrong dest IP address!\n");
            exit(0);
        }
    
    if (bind(sockfd, (struct sockaddr *) &peeraddr,sizeof(struct sockaddr_in)) == -1) {
        printf("Bind error\n");
        exit(0);
    }
    
    for (;;) {
        bzero(recmsg, BUFLEN + 1);
        n = recvfrom(sockfd, recmsg, BUFLEN, 0,(struct sockaddr *) &peeraddr, &socklen);
        if (n < 0) {
            printf("recvfrom err in udptalk!\n");
            exit(4);
        } else {
            recmsg[n] = 0;
            printf("peer:%s", recmsg);
        }
    }
} 
