#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "pub.h"
#include "common.h"

extern void BASE_LOG(char *buffer);

extern int g_hm_id;
extern struct s_hm g_hms[];

static char cmd_buffer[1500];

int send_gc_command(int mode)
{
    struct s_com *cmd;
    struct s_base_light *light_cmd;

    BASE_LOG("General Control Command is sending ...");

    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    cmd = (struct s_com *)cmd_buffer;
    cmd->code = BA_GC_SETTINGS;
    if(send(g_hms[g_hm_id-1].fd, cmd_buffer, BUFLEN, 0) >= 0)
    {
        BASE_LOG("OK!\n");
    }
    else
    {
        BASE_LOG("FAILED!!\n");
    }

    return 0;
}


int send_light_command(u_int32 cmds)
{
    struct s_com *common_cmd;
    struct s_base_light *light_cmd;

    printf("Light Command is sending ...");

    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    common_cmd = (struct s_com *)cmd_buffer;
    common_cmd->code = BA_LIGHT_CMD;
    light_cmd = (struct s_base_light *)(cmd_buffer + sizeof(struct s_com));
    light_cmd->on_off = cmds;
    if(send(g_hms[g_hm_id-1].fd, cmd_buffer, BUFLEN, 0) >= 0)
    {
        BASE_LOG("OK!\n");
    }
    else
    {
        BASE_LOG("FAILED!!\n");
    }

    return 0;
}

int send_motion_command(struct s_base_motion *motion)
{
    struct s_com *common_cmd;
    struct s_base_motion *motion_cmd;

    BASE_LOG("Motion command is sending ...");

    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    common_cmd = (struct s_com *)cmd_buffer;
    common_cmd->code = BA_MOTION_CMD;
    motion_cmd = (struct s_base_motion *)(cmd_buffer + sizeof(struct s_com));
    motion_cmd->left_action = motion->left_action;
    motion_cmd->left_data = motion->left_data;
    motion_cmd->right_action = motion->right_action;
    motion_cmd->right_data = motion->right_data;
    if(send(g_hms[g_hm_id-1].fd, cmd_buffer, BUFLEN, 0) >= 0)
    {
        BASE_LOG("OK!\n");
    }
    else
    {
        BASE_LOG("FAILED!!\n");
    }

    return 0;
}


int send_test_command()
{
    struct s_com *common_cmd;

    BASE_LOG("Test command is sending ...");

    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    common_cmd = (struct s_com *)cmd_buffer;
    common_cmd->code = BA_TEST_CMD;
    if(send(g_hms[g_hm_id-1].fd, cmd_buffer, BUFLEN, 0) >= 0)
    {
        BASE_LOG("OK!\n");
    }
    else
    {
        BASE_LOG("FAILED!!\n");
    }

    return 0;
}

int send_servo_command(int id, int angle)
{
    struct s_com *common_cmd;
    struct s_base_servo_cmd *servo_cmd;

    BASE_LOG("SERVO command is sending ...");

    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    common_cmd = (struct s_com *)cmd_buffer;
    common_cmd->code = BA_SERVO_CMD;
    servo_cmd = (struct s_base_servo_cmd *)(cmd_buffer + sizeof(struct s_com));
    servo_cmd->id = id;
    if(angle > 0) {
        servo_cmd->cmd = 1;
        servo_cmd->angle = angle;
    } else {
        servo_cmd->cmd = 0;
        servo_cmd->angle = -angle;
    }
    if(send(g_hms[g_hm_id-1].fd, cmd_buffer, BUFLEN, 0) >= 0)
    {
        BASE_LOG("OK!\n");
    }
    else
    {
        BASE_LOG("FAILED!!\n");
    }

    return 0;
}

int send_camera_command(int state)
{
    struct s_com *common_cmd;
    struct s_base_camera_cmd *camera_cmd;

    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    common_cmd = (struct s_com *)cmd_buffer;
    common_cmd->code = BA_CAMERA_CMD;
    camera_cmd = (struct s_base_camera_cmd *)(cmd_buffer + sizeof(struct s_com));
    camera_cmd->on_off = state;
    if(send(g_hms[g_hm_id-1].fd, cmd_buffer, BUFLEN, 0) >= 0)
    {
        BASE_LOG("OK!\n");
    }
    else
    {
        BASE_LOG("FAILED!!\n");
    }

    return 0;
}

