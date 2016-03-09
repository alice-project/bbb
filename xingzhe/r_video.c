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

#include "pub.h"
#include "r_video.h"

static globals global;
extern struct s_base_info m_base;

static void video_cmd_handler(void *data)
{
    struct s_base_camera_cmd *cmd = (struct s_base_camera_cmd *)data;

    if(cmd->on_off == DEVICE_ON) {
        global.in[0].run(0);
        global.out[0].run(0);
    } else {
        global.in[0].stop(0);
        global.out[0].stop(global.out[0].param.id);
        pthread_cond_destroy(&global.in[0].db_update);
        pthread_mutex_destroy(&global.in[0].db);
    }
}

void signal_handler(int sig)
{
    int i;

    /* signal "stop" to threads */
    LOG("setting signal to stop\n");
    global.stop = 1;
    usleep(1000 * 1000);

    /* clean up threads */
    LOG("force cancellation of threads and cleanup resources\n");
    for(i = 0; i < global.incnt; i++) {
        global.in[i].stop(i);
    }

    for(i = 0; i < global.outcnt; i++) {
        global.out[i].stop(global.out[i].param.id);
        pthread_cond_destroy(&global.in[i].db_update);
        pthread_mutex_destroy(&global.in[i].db);
    }
    usleep(1000 * 1000);

    /* close handles of input plugins */
    for(i = 0; i < global.incnt; i++) {
        dlclose(global.in[i].handle);
    }

    for(i = 0; i < global.outcnt; i++) {
        dlclose(global.out[i].handle);
    }
    DBG("all plugin handles closed\n");

    LOG("done\n");

    closelog();
    exit(0);
    return;
}


int video_start(int id)
{
    //char *input  = "input_uvc.so --resolution 640x480 --fps 5 --device /dev/video0";
    int i;
    global.outcnt = 1;

    regist_cmd_cb(BA_CAMERA_CMD, video_cmd_handler);

    openlog("MJPG-streamer ", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "starting application");

    /* ignore SIGPIPE (send by OS if transmitting to closed TCP sockets) */
    signal(SIGPIPE, SIG_IGN);

    /* register signal handler for <CTRL>+C in order to clean up */
    if(signal(SIGINT, signal_handler) == SIG_ERR) {
        LOG("could not register signal handler\n");
        closelog();
        exit(EXIT_FAILURE);
    }

    global.outcnt = 1;
    global.incnt = 1;
    /* open input plugin */
    for(i = 0; i < global.incnt; i++) {
        /* this mutex and the conditional variable are used to synchronize access to the global picture buffer */
        if(pthread_mutex_init(&global.in[i].db, NULL) != 0) {
            LOG("could not initialize mutex variable\n");
            closelog();
            exit(EXIT_FAILURE);
        }
        if(pthread_cond_init(&global.in[i].db_update, NULL) != 0) {
            LOG("could not initialize condition variable\n");
            closelog();
            exit(EXIT_FAILURE);
        }

        global.in[i].stop      = 0;
        global.in[i].buf       = NULL;
        global.in[i].size      = 0;
        global.in[i].plugin = "./input_uvc.so";
        global.in[i].handle = dlopen(global.in[i].plugin, RTLD_LAZY);
        if(!global.in[i].handle) {
            LOG("ERROR: could not find input plugin\n");
            LOG("       Perhaps you want to adjust the search path with:\n");
            LOG("       # export LD_LIBRARY_PATH=/path/to/plugin/folder\n");
            LOG("       dlopen: %s\n", dlerror());
            closelog();
            exit(EXIT_FAILURE);
        }
        global.in[i].init = dlsym(global.in[i].handle, "input_init");
        if(global.in[i].init == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.in[i].stop = dlsym(global.in[i].handle, "input_stop");
        if(global.in[i].stop == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.in[i].run = dlsym(global.in[i].handle, "input_run");
        if(global.in[i].run == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        /* try to find optional command */
        global.in[i].cmd = dlsym(global.in[i].handle, "input_cmd");

        global.in[i].param.global = &global;
        global.in[i].param.id = i;

        if(global.in[i].init(&global.in[i].param, i)) {
            LOG("input_init() return value signals to exit\n");
            closelog();
            exit(0);
        }
    }

    /* open output plugin */
    for(i = 0; i < global.outcnt; i++) {
        global.out[i].plugin = "./output_udp.so";
        global.out[i].handle = dlopen(global.out[i].plugin, RTLD_LAZY);
        if(!global.out[i].handle) {
            LOG("ERROR: could not find output plugin %s\n", global.out[i].plugin);
            LOG("       Perhaps you want to adjust the search path with:\n");
            LOG("       # export LD_LIBRARY_PATH=/path/to/plugin/folder\n");
            LOG("       dlopen: %s\n", dlerror());
            closelog();
            exit(EXIT_FAILURE);
        }
        global.out[i].init = dlsym(global.out[i].handle, "output_init");
        if(global.out[i].init == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.out[i].stop = dlsym(global.out[i].handle, "output_stop");
        if(global.out[i].stop == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.out[i].run = dlsym(global.out[i].handle, "output_run");
        if(global.out[i].run == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }

        global.out[i].fd_base = m_base.fd;

        /* try to find optional command */
        global.out[i].cmd = dlsym(global.out[i].handle, "output_cmd");

        global.out[i].param.global = &global;
        global.out[i].param.id = i;
        if(global.out[i].init(&global.out[i].param, i)) {
            LOG("output_init() return value signals to exit\n");
            closelog();
            exit(0);
        }
    }

    /* start to read the input, push pictures into global buffer */
    DBG("starting %d input plugin\n", global.incnt);
    for(i = 0; i < global.incnt; i++) {
        syslog(LOG_INFO, "starting input plugin %s", global.in[i].plugin);
        if(global.in[i].run(i)) {
            LOG("can not run input plugin %d: %s\n", i, global.in[i].plugin);
            closelog();
            return 1;
        }
    }

    DBG("starting %d output plugin(s)\n", global.outcnt);
    for(i = 0; i < global.outcnt; i++) {
        syslog(LOG_INFO, "starting output plugin: %s (ID: %02d)", global.out[i].plugin, global.out[i].param.id);
        global.out[i].run(global.out[i].param.id);
    }

    return 0;
}


