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

#ifndef MJPG_STREAMER_H
#define MJPG_STREAMER_H
#define SOURCE_VERSION "2.0"

/* FIXME take a look to the output_http clients thread marked with fixme if you want to set more then 10 plugins */
#define MAX_INPUT_PLUGINS 10
#define MAX_OUTPUT_PLUGINS 10
#define MAX_PLUGIN_ARGUMENTS 32
#include <linux/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

#ifdef DEBUG
#define DBG(...) fprintf(stderr, " DBG(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#else
#define DBG(...)
#endif

#define LOG(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stderr, "%s", _bf); syslog(LOG_INFO, "%s", _bf); }

/* global variables that are accessed by all plugins */
typedef struct _globals globals;

typedef struct _control control;

/* an enum to identify the commands destination*/
typedef enum {
    Dest_Input = 0,
    Dest_Output = 1,
    Dest_Program = 2,
} command_dest;

/* commands which can be send to the input plugin */
typedef enum _cmd_group cmd_group;
enum _cmd_group {
    IN_CMD_GENERIC = 0, // if you use non V4L2 input plugin you not need to deal the groups.
    IN_CMD_V4L2 = 1,
    IN_CMD_RESOLUTION = 2,
    IN_CMD_JPEG_QUALITY = 3,
};

#define ABS(a) (((a) < 0) ? -(a) : (a))
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#define LENGTH_OF(x) (sizeof(x)/sizeof(x[0]))

#define INPUT_PLUGIN_PREFIX " i: "
#define IPRINT(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stderr, "%s", INPUT_PLUGIN_PREFIX); fprintf(stderr, "%s", _bf); syslog(LOG_INFO, "%s", _bf); }

/* parameters for input plugin */
typedef struct _input_parameter input_parameter;
struct _input_parameter {
    int id;
    char *parameters;
    int argc;
    char *argv[MAX_PLUGIN_ARGUMENTS];
    struct _globals *global;
};

typedef struct _input_resolution input_resolution;
struct _input_resolution {
    unsigned int width;
    unsigned int height;
};

typedef struct _input_format input_format;
struct _input_format {
    struct v4l2_fmtdesc format;
    input_resolution *supportedResolutions;
    int resolutionCount;
    char currentResolution;
};

/* structure to store variables/functions for input plugin */
typedef struct _input input;
struct _input {
    char *plugin;
    void *handle;

    input_parameter param; // this holds the command line arguments

    // input plugin parameters
    struct _control *in_parameters;
    int parametercount;

    struct v4l2_jpegcompression jpegcomp;

    /* signal fresh frames */
    pthread_mutex_t db;
    pthread_cond_t  db_update;

    /* global JPG frame, this is more or less the "database" */
    unsigned char *buf;
    int size;

    /* v4l2_buffer timestamp */
    struct timeval timestamp;

    input_format *in_formats;
    int formatCount;
    int currentFormat; // holds the current format number

    int (*init)(input_parameter *, int id);
    int (*stop)(int);
    int (*run)(int);
    int (*cmd)(int plugin, unsigned int control_id, unsigned int group, int value);
};


#define OUTPUT_PLUGIN_PREFIX " o: "
#define OPRINT(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stderr, "%s", OUTPUT_PLUGIN_PREFIX); fprintf(stderr, "%s", _bf); syslog(LOG_INFO, "%s", _bf); }

/* parameters for output plugin */
typedef struct _output_parameter output_parameter;
struct _output_parameter {
    int id;
    char *parameters;
    int argc;
    char *argv[MAX_PLUGIN_ARGUMENTS];
    struct _globals *global;
};



/* structure to store variables/functions for output plugin */
typedef struct _output output;
struct _output {
    char *plugin;
    void *handle;
    output_parameter param;

    // input plugin parameters
    struct _control *out_parameters;
    int parametercount;

    int (*init)(output_parameter *param, int id);
    int (*stop)(int);
    int (*run)(int);
    int (*cmd)(int plugin, unsigned int control_id, unsigned int group, int value);
};

struct _control {
    struct v4l2_queryctrl ctrl;
    int value;
    struct v4l2_querymenu *menuitems;
    /*  In the case the control a V4L2 ctrl this variable will specify
        that the control is a V4L2_CTRL_CLASS_USER control or not.
        For non V4L2 control it is not acceptable, leave it 0.
    */
    int class_id;
    int group;
};

struct _globals {
    int stop;

    /* input plugin */
    input in[MAX_INPUT_PLUGINS];
    int incnt;

    /* output plugin */
    output out[MAX_OUTPUT_PLUGINS];
    int outcnt;

    /* pointer to control functions */
    //int (*control)(int command, char *details);
};

int input_init(input_parameter *param, int id);
int input_stop(int id);
int input_run(int id);

int video_init(int id);
int video_start(int id);
int video_stop(int id);

#endif
