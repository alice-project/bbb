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

#include "r_video.h"

static globals global;

extern struct s_base_info m_base;

/******************************************************************************
Description.: pressing CTRL+C sends signals to this process instead of just
              killing it plugins can tidily shutdown and free allocated
              resources. The function prototype is defined by the system,
              because it is a callback function.
Input Value.: sig tells us which signal was received
Return Value: -
******************************************************************************/
void signal_handler(int sig)
{

    return;
}


/******************************************************************************
Description.:
Input Value.:
Return Value:
******************************************************************************/
int video_init(int id)
{
    int ret;
    global.in[id].param.parameters = "";
    global.in[id].param.global = &global;
    global.in[id].param.id = id;

    global.out[id].param.parameters = "";
    global.out[id].param.global = &global;
    global.out[id].param.id = id;

    ret = input_init(&global.in[id].param, id);
    ret |= output_init(&global.out[id].param);
    return ret;
}

int video_start(int id)
{
    int ret;
    ret = input_run(id);
    ret |= output_run(id);
    return ret;
}

int video_stop(int id)
{
    int ret;
    ret = input_stop(id);
    ret |= output_stop(id);
    return ret;
}


