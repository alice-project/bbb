/*******************************************************************************
# Linux-UVC streaming input-plugin for MJPG-streamer                           #
#                                                                              #
# This package work with the Logitech UVC based webcams with the mjpeg feature #
#                                                                              #
# Copyright (C) 2005 2006 Laurent Pinchart &&  Michel Xhaard                   #
#                    2007 Lucas van Staden                                     #
#                    2007 Tom Stöveken                                         #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
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
#include <syslog.h>

#include "pub.h"
#include "common.h"

#include "v4l2uvc.h"
#include "huffman.h"
#include "jpeg_utils.h"
#include "dynctrl.h"

#define INPUT_PLUGIN_NAME "UVC webcam grabber"

/*
 * UVC resolutions mentioned at: (at least for some webcams)
 * http://www.quickcamteam.net/hcl/frame-format-matrix/
 */
static const struct {
    const char *string;
    const int width, height;
} resolutions[] = {
    { "QSIF", 160,  120  },
    { "QCIF", 176,  144  },
    { "CGA",  320,  200  },
    { "QVGA", 320,  240  },
    { "CIF",  352,  288  },
    { "VGA",  640,  480  },
    { "SVGA", 800,  600  },
    { "XGA",  1024, 768  },
    { "SXGA", 1280, 1024 }
};

extern struct s_base_info m_base;

/* private functions and variables to this plugin */
static globals *pglobal;
static int gquality = 80;
static u_int32 minimum_size = 0;
static int dynctrls = 1;

void *cam_thread(void *);
void cam_cleanup(void *);
int input_cmd(int plugin, u_int32 control, u_int32 group, int value);

static unsigned char sndbuffer[MAX_VIDEO_FRAME_MSG];


/*** plugin interface functions ***/
/******************************************************************************
Description.: This function ializes the plugin. It parses the commandline-
              parameter and stores the default and parsed values in the
              appropriate variables.
Input Value.: param contains among others the command-line string
Return Value: 0 if everything is fine
              1 if "--help" was triggered, in this case the calling programm
              should stop running and leave.
******************************************************************************/
int input_init(input_parameter *param, int id)
{
    char *dev = "/dev/video0", *s;
    int width = 640, height = 480, fps = 5, format = V4L2_PIX_FMT_MJPEG, i;
    /* initialize the mutes variable */
    if(pthread_mutex_init(&cams[id].controls_mutex, NULL) != 0) {
        IPRINT("could not initialize mutex variable\n");
        exit(EXIT_FAILURE);
    }

    param->argv[0] = INPUT_PLUGIN_NAME;

    /* show all parameters for DBG purposes */
    for(i = 0; i < param->argc; i++) {
        DBG("argv[%d]=%s\n", i, param->argv[i]);
    }

    DBG("input id: %d\n", id);
    cams[id].id = id;
    cams[id].pglobal = param->global;

    /* allocate webcam datastructure */
    cams[id].videoIn = malloc(sizeof(struct vdIn));
    if(cams[id].videoIn == NULL) {
        IPRINT("not enough memory for videoIn\n");
        exit(EXIT_FAILURE);
    }
    memset(cams[id].videoIn, 0, sizeof(struct vdIn));

    /* display the parsed values */
    IPRINT("Using V4L2 device.: %s\n", dev);
    IPRINT("Desired Resolution: %i x %i\n", width, height);
    IPRINT("Frames Per Second.: %i\n", fps);
    IPRINT("Format............: %s\n", (format == V4L2_PIX_FMT_YUYV) ? "YUV" : "MJPEG");
    if(format == V4L2_PIX_FMT_YUYV)
        IPRINT("JPEG Quality......: %d\n", gquality);

    DBG("vdIn pn: %d\n", id);
    /* open video device and prepare data structure */
    if(init_videoIn(cams[id].videoIn, dev, width, height, fps, format, 1, cams[id].pglobal, id) < 0) {
        IPRINT("init_VideoIn failed\n");
        closelog();
        exit(EXIT_FAILURE);
    }
    /*
     * recent linux-uvc driver (revision > ~#125) requires to use dynctrls
     * for pan/tilt/focus/...
     * dynctrls must get initialized
     */
    if(dynctrls)
        initDynCtrls(cams[id].videoIn->fd);

    enumerateControls(cams[id].videoIn, cams[id].pglobal, id); // enumerate V4L2 controls after UVC extended mapping

    return 0;
}

/******************************************************************************
Description.: Stops the execution of worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int input_stop(int id)
{
    DBG("will cancel camera thread #%02d\n", id);
    pthread_cancel(cams[id].threadID);
    return 0;
}

/******************************************************************************
Description.: spins of a worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int input_run(int id)
{
    cams[id].pglobal->in[id].buf = malloc(cams[id].videoIn->framesizeIn);
    if(cams[id].pglobal->in[id].buf == NULL) {
        fprintf(stderr, "could not allocate memory\n");
        exit(EXIT_FAILURE);
    }

    DBG("launching camera thread #%02d\n", id);
    /* create thread and pass context to thread function */
    pthread_create(&(cams[id].threadID), NULL, cam_thread, &(cams[id]));
    pthread_detach(cams[id].threadID);
    return 0;
}


/******************************************************************************
Description.: this thread worker grabs a frame and copies it to the global buffer
Input Value.: unused
Return Value: unused, always NULL
******************************************************************************/
void *cam_thread(void *arg)
{
    int n;
    struct s_com *msg;
    struct s_hm_video *video;

    context *pcontext = arg;
    pglobal = pcontext->pglobal;

    /* set cleanup handler to cleanup allocated ressources */
    pthread_cleanup_push(cam_cleanup, pcontext);

    while(!pglobal->stop) {
        while(pcontext->videoIn->streamingState == STREAMING_PAUSED) {
            usleep(1); // maybe not the best way so FIXME
        }

        /* grab a frame */
        if(uvcGrab(pcontext->videoIn) < 0) {
            IPRINT("Error grabbing frames\n");
            exit(EXIT_FAILURE);
        }

        printf("received frame of size: %d from plugin: %d\n", pcontext->videoIn->buf.bytesused, pcontext->id);

        /*
         * Workaround for broken, corrupted frames:
         * Under low light conditions corrupted frames may get captured.
         * The good thing is such frames are quite small compared to the regular pictures.
         * For example a VGA (640x480) webcam picture is normally >= 8kByte large,
         * corrupted frames are smaller.
         */
        if(pcontext->videoIn->buf.bytesused < minimum_size) {
            DBG("dropping too small frame, assuming it as broken\n");
            continue;
        }

        msg = (struct s_com *)sndbuffer;
        msg->code = HM_CAMERA;
        msg->id = 0;
        msg->flags = 0;
        video = (struct s_hm_video *)(sndbuffer + sizeof(struct s_com));
        video->size = pcontext->videoIn->buf.bytesused;

        /*
         * If capturing in YUV mode convert to JPEG now.
         * This compression requires many CPU cycles, so try to avoid YUV format.
         * Getting JPEGs straight from the webcam, is one of the major advantages of
         * Linux-UVC compatible devices.
         */
        if(pcontext->videoIn->formatIn == V4L2_PIX_FMT_YUYV) {
            DBG("compressing frame from input: %d\n", (int)pcontext->id);
            pglobal->in[pcontext->id].size = compress_yuyv_to_jpeg(pcontext->videoIn, video->data, pcontext->videoIn->framesizeIn, gquality);
        } else {
            DBG("copying frame from input: %d\n", (int)pcontext->id);
            pglobal->in[pcontext->id].size = memcpy_picture(video->data, pcontext->videoIn->tmpbuffer, pcontext->videoIn->buf.bytesused);
        }

        /* copy this frame's timestamp to user space */
        pglobal->in[pcontext->id].timestamp = pcontext->videoIn->buf.timestamp;

        // send back client's message that came in udpbuffer
        n = sendto(m_base.fd, msg,
               video->size+sizeof(struct s_com)+sizeof(struct s_hm_video), 0, 
               (struct sockaddr *) &m_base.m_addr, sizeof(struct sockaddr_in));
printf("n=%d\n",n);

        /* only use usleep if the fps is below 5, otherwise the overhead is too long */
        if(pcontext->videoIn->fps < 5) {
            DBG("waiting for next frame for %d us\n", 1000 * 1000 / pcontext->videoIn->fps);
            usleep(100 * 1000 / pcontext->videoIn->fps);
        } else {
            DBG("waiting for next frame\n");
        }
    }

    DBG("leaving input thread, calling cleanup function now\n");
    pthread_cleanup_pop(1);

    return NULL;
}

/******************************************************************************
Description.:
Input Value.:
Return Value:
******************************************************************************/
void cam_cleanup(void *arg)
{
    static u_int8 first_run = 1;
    context *pcontext = arg;
    pglobal = pcontext->pglobal;
    if(!first_run) {
        DBG("already cleaned up ressources\n");
        return;
    }

    first_run = 0;
    IPRINT("cleaning up ressources allocated by input thread\n");

    close_v4l2(pcontext->videoIn);
    if(pcontext->videoIn->tmpbuffer != NULL) free(pcontext->videoIn->tmpbuffer);
    if(pcontext->videoIn != NULL) free(pcontext->videoIn);
    if(pglobal->in[pcontext->id].buf != NULL)
        free(pglobal->in[pcontext->id].buf);
}

/******************************************************************************
Description.: process commands, allows to set v4l2 controls
Input Value.: * control specifies the selected v4l2 control's id
                see struct v4l2_queryctr in the videodev2.h
              * value is used for control that make use of a parameter.
Return Value: depends in the command, for most cases 0 means no errors and
              -1 signals an error. This is just rule of thumb, not more!
******************************************************************************/
int input_cmd(int plugin_number, u_int32 control_id, u_int32 group, int value)
{
    int ret = -1;
    int i = 0;
    DBG("Requested cmd (id: %d) for the %d plugin. Group: %d value: %d\n", control_id, plugin_number, group, value);
    switch(group) {
    case IN_CMD_GENERIC: {
            int i;
            for (i = 0; i<pglobal->in[plugin_number].parametercount; i++) {
                if ((pglobal->in[plugin_number].in_parameters[i].ctrl.id == control_id) &&
                    (pglobal->in[plugin_number].in_parameters[i].group == IN_CMD_GENERIC)){
                    DBG("Generic control found (id: %d): %s\n", control_id, pglobal->in[plugin_number].in_parameters[i].ctrl.name);
                    DBG("New %s value: %d\n", pglobal->in[plugin_number].in_parameters[i].ctrl.name, value);
                    return 0;
                }
            }
            DBG("Requested generic control (%d) did not found\n", control_id);
            return -1;
        } break;
    case IN_CMD_V4L2: {
            ret = v4l2SetControl(cams[plugin_number].videoIn, control_id, value, plugin_number, pglobal);
            if(ret == 0) {
                pglobal->in[plugin_number].in_parameters[i].value = value;
            } else {
                DBG("v4l2SetControl failed: %d\n", ret);
            }
            return ret;
        } break;
    case IN_CMD_RESOLUTION: {
        // the value points to the current formats nth resolution
        if(value > (pglobal->in[plugin_number].in_formats[pglobal->in[plugin_number].currentFormat].resolutionCount - 1)) {
            DBG("The value is out of range");
            return -1;
        }
        int height = pglobal->in[plugin_number].in_formats[pglobal->in[plugin_number].currentFormat].supportedResolutions[value].height;
        int width = pglobal->in[plugin_number].in_formats[pglobal->in[plugin_number].currentFormat].supportedResolutions[value].width;
        ret = setResolution(cams[plugin_number].videoIn, width, height);
        if(ret == 0) {
            pglobal->in[plugin_number].in_formats[pglobal->in[plugin_number].currentFormat].currentResolution = value;
        }
        return ret;
    } break;
    case IN_CMD_JPEG_QUALITY:
        if((value >= 0) && (value < 101)) {
            pglobal->in[plugin_number].jpegcomp.quality = value;
            if(IOCTL_VIDEO(cams[plugin_number].videoIn->fd, VIDIOC_S_JPEGCOMP, &pglobal->in[plugin_number].jpegcomp) != EINVAL) {
                DBG("JPEG quality is set to %d\n", value);
                ret = 0;
            } else {
                DBG("Setting the JPEG quality is not supported\n");
            }
        } else {
            DBG("Quality is out of range\n");
        }
        break;
    }
    return ret;
}

