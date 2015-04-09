/*******************************************************************************
#                                                                              #
#      MJPG-streamer allows to stream JPG frames from an input-plugin          #
#      to several output plugins                                               #
#                                                                              #
#      Copyright (C) 2007 Tom Stöveken                                         #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/

/*
  This output plugin is based on code from output_file.c
  Writen by Dimitrios Zachariadis
  Version 0.1, May 2010

  It provides a mechanism to take snapshots with a trigger from a UDP packet.
  The UDP msg contains the path for the snapshot jpeg file
  It echoes the message received back to the sender, after taking the snapshot
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <syslog.h>

#include <dirent.h>
#include "pub.h"
#include "common.h"
#include "r_video.h"

#define OUTPUT_PLUGIN_NAME "UDP output plugin"

extern struct s_base_info m_base;

enum RTSP_State {
    RTSP_State_Setup,
    RTSP_State_Playing,
    RTSP_State_Paused,
    RTSP_State_Teardown,
};

static pthread_t worker;
static globals *pglobal;
static int fd, delay, max_frame_size;
static char *folder = "/tmp";
static unsigned char *frame = NULL;
static int input_number = 0;
//static unsigned char sndbuffer[1500];

/******************************************************************************
Description.: clean up allocated ressources
Input Value.: unused argument
Return Value: -
******************************************************************************/
void worker_cleanup(void *arg)
{
    static unsigned char first_run = 1;

    if(!first_run) {
        DBG("already cleaned up ressources\n");
        return;
    }

    first_run = 0;
    OPRINT("cleaning up ressources allocated by worker thread\n");

    if(frame != NULL) {
        free(frame);
    }
    close(fd);
}

/******************************************************************************
Description.: this is the main worker thread
              it loops forever, grabs a fresh frame and stores it to file
Input Value.:
Return Value:
******************************************************************************/
void *worker_thread(void *arg)
{
    int frame_size = 0;
    int buffer_left = 0, offset=0;
    unsigned char *sndbuffer;

    struct s_com *msg;
    struct s_hm_video *video;
    
    /* set cleanup handler to cleanup allocated ressources */
    pthread_cleanup_push(worker_cleanup, NULL);

    if(m_base.fd < 0)
        return NULL;

    while(!pglobal->stop) {
        pthread_mutex_lock(&pglobal->in[input_number].db);
        pthread_cond_wait(&pglobal->in[input_number].db_update, &pglobal->in[input_number].db);

        /* read buffer */
        frame_size = pglobal->in[input_number].size;

        /* copy frame to our local buffer now */
        memcpy(frame, pglobal->in[input_number].buf, frame_size);

        /* allow others to access the global buffer again */
        pthread_mutex_unlock(&pglobal->in[input_number].db);

        // send back client's message that came in udpbuffer
        // send 1024 data every time
        buffer_left = frame_size;
        offset = 0;
        while(buffer_left >= 1024)
        {
            sndbuffer = (unsigned char *)malloc(1500);
            memset(sndbuffer, 0, 1500);
            msg = (struct s_com *)sndbuffer;
            msg->code = HM_CAMERA;
            msg->id = 0;
            msg->flags = VIDEO_MORE_PACKETS;
            video = (struct s_hm_video *)(sndbuffer + sizeof(struct s_com));
            video->length = 1024;
            memcpy(video->data, frame+offset, 1024);
            sendto(m_base.fd, sndbuffer, 1024+sizeof(struct s_com)+sizeof(struct s_hm_video), 0, 
                    (struct sockaddr *) &m_base.m_addr, sizeof(struct sockaddr_in));
            buffer_left -= 1024;
            offset += 1024;
            free(sndbuffer);
        }
        if(buffer_left > 0)
        {
            sndbuffer = (unsigned char *)malloc(1500);
            memset(sndbuffer, 0, 1500);
            msg = (struct s_com *)sndbuffer;
            msg->code = HM_CAMERA;
            msg->id = 0;
            msg->flags = 0;
            video = (struct s_hm_video *)(sndbuffer + sizeof(struct s_com));
            video->length = buffer_left;
            memcpy(video->data, frame+offset, buffer_left);
            msg->flags = 0;
            sendto(m_base.fd, sndbuffer, buffer_left+sizeof(struct s_com)+sizeof(struct s_hm_video), 0, 
                   (struct sockaddr *) &m_base.m_addr, sizeof(struct sockaddr_in));
            free(sndbuffer);
        }

        /* if specified, wait now */
        if(delay > 0) {
            usleep(1000 * delay);
        }
    }

    /* cleanup now */
    pthread_cleanup_pop(1);

    return NULL;
}

/*** plugin interface functions ***/
/******************************************************************************
Description.: this function is called first, in order to initialise
              this plugin and pass a parameter string
Input Value.: parameters
Return Value: 0 if everything is ok, non-zero otherwise
******************************************************************************/
int output_init(output_parameter *param)
{
    pglobal = param->global;

    frame = malloc(640*480*10);

    return 0;
}

/******************************************************************************
Description.: calling this function stops the worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int output_stop(int id)
{
    DBG("will cancel worker thread\n");
    pthread_cancel(worker);
    return 0;
}

/******************************************************************************
Description.: calling this function creates and starts the worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int output_run(int id)
{
    DBG("launching worker thread\n");
    pthread_create(&worker, 0, worker_thread, NULL);
    pthread_detach(worker);
    return 0;
}


