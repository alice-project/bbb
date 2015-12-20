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
#include <fcntl.h>
#include <time.h>
#include <syslog.h>

#include <dirent.h>

#include "pub.h"
#include "utils.h"
#include "r_video.h"

#define OUTPUT_PLUGIN_NAME "FILE output plugin"

static pthread_t worker;
static globals *pglobal;
static int fd, delay, max_frame_size;
static char *folder = "/tmp";
static unsigned char *frame = NULL;
static char *command = NULL;
static int input_number = 0;

/******************************************************************************
Description.: print a help message
Input Value.: -
Return Value: -
******************************************************************************/
void help(void)
{
    fprintf(stderr, " ---------------------------------------------------------------\n" \
            " Help for output plugin..: "OUTPUT_PLUGIN_NAME"\n" \
            " ---------------------------------------------------------------\n" \
            " The following parameters can be passed to this plugin:\n\n" \
            " [-f | --folder ]........: folder to save pictures\n" \
            " [-m | --mjpeg ]........: save the stream to an mjpeg file\n" \
            " [-d | --delay ].........: delay after saving pictures in ms\n" \
            " [-s | --size ]..........: size of ring buffer (max number of pictures to hold)\n" \
            " [-e | --exceed ]........: allow ringbuffer to exceed limit by this amount\n" \
            " [-c | --command ].......: execute command after saving picture\n\n" \
            " [-i | --input ].......: read frames from the specified input plugin\n\n" \
            " ---------------------------------------------------------------\n");
}

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
Description.: compares a directory entry with a pattern
Input Value.: directory entry
Return Value: 0 if string do not match, 1 if they match
******************************************************************************/
int check_for_filename(const struct dirent *entry)
{
    int rc;

    int year, month, day, hour, minute, second;
    unsigned long long number;

    /*
     * try to scan the string using scanf
     * I would like to use a define for this format string later...
     */
    rc = sscanf(entry->d_name, "%d_%d_%d_%d_%d_%d_picture_%09llu.jpg", &year, \
                &month, \
                &day, \
                &hour, \
                &minute, \
                &second, \
                &number);

    DBG("%s, rc is %d (%d, %d, %d, %d, %d, %d, %llu)\n", entry->d_name, \
        rc, \
        year, \
        month, \
        day, \
        hour, \
        minute, \
        second, \
        number);

    /* if scanf could find all values, it matches our filenames */
    if(rc != 7) return 0;

    return 1;
}

/******************************************************************************
Description.: this is the main worker thread
              it loops forever, grabs a fresh frame and stores it to file
Input Value.:
Return Value:
******************************************************************************/
void *worker_thread(void *arg)
{
    int ok = 1, frame_size = 0, rc = 0;
    char buffer1[1024] = {0}, buffer2[1024] = {0};
    unsigned long long counter = 0;
    time_t t;
    struct tm *now;
    unsigned char *tmp_framebuffer = NULL;
    struct s_com *msg;
    struct s_sxz_video *qianliyan;

    /* set cleanup handler to cleanup allocated ressources */
    pthread_cleanup_push(worker_cleanup, NULL);

    while(ok >= 0 && !pglobal->stop) {
        DBG("waiting for fresh frame\n");
        pthread_mutex_lock(&pglobal->in[input_number].db);
        pthread_cond_wait(&pglobal->in[input_number].db_update, &pglobal->in[input_number].db);

        /* read buffer */
        frame_size = pglobal->in[input_number].size;

        /* check if buffer for frame is large enough, increase it if necessary */
        if(frame_size > max_frame_size) {
            DBG("increasing buffer size to %d\n", frame_size);

            max_frame_size = frame_size + (1 << 16);
            if((tmp_framebuffer = realloc(frame, max_frame_size+sizeof(struct s_sxz_video)+sizeof(struct s_com))) == NULL) {
                pthread_mutex_unlock(&pglobal->in[input_number].db);
                LOG("not enough memory\n");
                return NULL;
            }

            frame = tmp_framebuffer;
        }

        /* copy frame to our local buffer now */
        memcpy(frame+sizeof(struct s_sxz_video)+sizeof(struct s_com), pglobal->in[input_number].buf, frame_size);
        msg = (struct s_com *)frame;
        msg->code = XZ_CAMERA;
        msg->id = 0;
        msg->flags = 0;

        qianliyan = (struct s_sxz_video *)(frame + sizeof(struct s_com));
        qianliyan->size = frame_size;

        send(pglobal->out[0].fd_base, frame, frame_size, 0);

        /* allow others to access the global buffer again */
        pthread_mutex_unlock(&pglobal->in[input_number].db);

        /* prepare filename */
        memset(buffer1, 0, sizeof(buffer1));
        memset(buffer2, 0, sizeof(buffer2));

        /* get current time */
        t = time(NULL);
        now = localtime(&t);
        if(now == NULL) {
            perror("localtime");
            return NULL;
        }

        /* prepare string, add time and date values */
        if(strftime(buffer1, sizeof(buffer1), "%%s/%Y_%m_%d_%H_%M_%S_picture_%%09llu.jpg", now) == 0) {
            OPRINT("strftime returned 0\n");
            free(frame); frame = NULL;
            return NULL;
        }

        /* finish filename by adding the foldername and a counter value */
        snprintf(buffer2, sizeof(buffer2), buffer1, folder, counter);

        counter++;

        DBG("writing file: %s\n", buffer2);

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
Description.: this function is called first, in order to initialize
              this plugin and pass a parameter string
Input Value.: parameters
Return Value: 0 if everything is OK, non-zero otherwise
******************************************************************************/
int output_init(output_parameter *param)
{
	int i;
    delay = 0;

    param->argv[0] = OUTPUT_PLUGIN_NAME;

    /* show all parameters for DBG purposes */
    for(i = 0; i < param->argc; i++) {
        DBG("argv[%d]=%s\n", i, param->argv[i]);
    }

    reset_getopt();
    while(1) {
        int option_index = 0, c = 0;
        static struct option long_options[] = {
            {"h", no_argument, 0, 0
            },
            {"help", no_argument, 0, 0},
            {"f", required_argument, 0, 0},
            {"folder", required_argument, 0, 0},
            {"d", required_argument, 0, 0},
            {"delay", required_argument, 0, 0},
            {"m", required_argument, 0, 0},
            {"mjpeg", required_argument, 0, 0},
            {"i", required_argument, 0, 0},
            {"input", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        c = getopt_long_only(param->argc, param->argv, "", long_options, &option_index);

        /* no more options to parse */
        if(c == -1) break;

        /* unrecognized option */
        if(c == '?') {
            help();
            return 1;
        }

        switch(option_index) {
            /* h, help */
        case 0:
        case 1:
            DBG("case 0,1\n");
            help();
            return 1;
            break;

            /* f, folder */
        case 2:
        case 3:
            DBG("case 2,3\n");
            folder = malloc(strlen(optarg) + 1);
            strcpy(folder, optarg);
            if(folder[strlen(folder)-1] == '/')
                folder[strlen(folder)-1] = '\0';
            break;

            /* d, delay */
        case 4:
        case 5:
            DBG("case 4,5\n");
            delay = atoi(optarg);
            break;

            /* c, command */
        case 10:
        case 11:
            DBG("case 10,11\n");
            command = strdup(optarg);
            break;
        case 12:
        case 13:
            DBG("case 12,13\n");
            input_number = atoi(optarg);
            break;
        }
    }

    pglobal = param->global;
    if(!(input_number < pglobal->incnt)) {
        OPRINT("ERROR: the %d input_plugin number is too much only %d plugins loaded\n", input_number, pglobal->incnt);
        return 1;
    }

    OPRINT("output folder.....: %s\n", folder);
    OPRINT("input plugin.....: %d: %s\n", input_number, pglobal->in[input_number].plugin);
    OPRINT("delay after save..: %d\n", delay);
    OPRINT("command...........: %s\n", (command == NULL) ? "disabled" : command);
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

int output_cmd(int plugin, unsigned int control_id, unsigned int group, int value)
{
    DBG("command (%d, value: %d) for group %d triggered for plugin instance #%02d\n", control_id, value, group, plugin);
    return 0;
}
