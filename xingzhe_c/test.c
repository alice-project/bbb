#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <pthread.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <syslog.h>


#define NB_BUFFER 4


#define IOCTL_RETRY 4

/* ioctl with a number of retries in the case of I/O failure
* args:
* fd - device descriptor
* IOCTL_X - ioctl reference
* arg - pointer to ioctl data
* returns - ioctl result
*/
int xioctl(int fd, int IOCTL_X, void *arg);

#ifdef USE_LIBV4L2
#include <libv4l2.h>
#define IOCTL_VIDEO(fd, req, value) v4l2_ioctl(fd, req, value)
#define OPEN_VIDEO(fd, flags) v4l2_open(fd, flags)
#define CLOSE_VIDEO(fd) v4l2_close(fd)
#else
#define IOCTL_VIDEO(fd, req, value) ioctl(fd, req, value)
#define OPEN_VIDEO(fd, flags) open(fd, flags)
#define CLOSE_VIDEO(fd) close(fd)
#endif

int xioctl(int fd, int IOCTL_X, void *arg)
{
    int ret = 0;
    int tries = IOCTL_RETRY;
    do {
        ret = IOCTL_VIDEO(fd, IOCTL_X, arg);
    } while(ret && tries-- &&
            ((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

    if(ret && (tries <= 0)) fprintf(stderr, "ioctl (%i) retried %i times - giving up: %s)\n", IOCTL_X, IOCTL_RETRY, strerror(errno));

    return (ret);
}


int main(int argc, char *argv[])
{
    int ops=0;
    int fd;

    char *dev = "/dev/video0";

    if((fd = OPEN_VIDEO(dev, O_RDWR)) == -1) {
        perror("ERROR opening V4L interface");
        printf("errno: %d", errno);
        return -1;
    }

    if(memcmp(argv[1], "-d", sizeof("-d"))==0)
        ops = 1;

    switch (ops)
    {
        case 1:
            break;
        default:
            break;
    }

    CLOSE_VIDEO(fd);

    return 0;
}

