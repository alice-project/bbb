#include <stdlib.h>
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

int send_gc_command()
{
    struct s_com *cmd;

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


int send_light_command(unsigned int cmds)
{
    struct s_com *common_cmd;
    struct s_base_light *light_cmd;

    BASE_LOG("Light Command is sending ...");

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
    motion_cmd->left_dir = motion->left_dir;
    motion_cmd->right_action = motion->right_action;
    motion_cmd->right_dir = motion->right_dir;
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

int send_servo_command()
{
    return 0;
}

int send_pwm_duty_command(int change)
{
    struct s_com *common_cmd;
    struct s_base_pwm_duty *duty_cmd;

    BASE_LOG("Test command is sending ...");

    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    common_cmd = (struct s_com *)cmd_buffer;
    common_cmd->code = BA_PWM_DUTY_CMD;
    duty_cmd = (struct s_base_pwm_duty *)(cmd_buffer + sizeof(struct s_com));
    duty_cmd->cmd = change;
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

int send_pwm_freq_command(int change)
{
    struct s_com *common_cmd;
    struct s_base_pwm_freq *freq_cmd;

    BASE_LOG("Test command is sending ...");

    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    common_cmd = (struct s_com *)cmd_buffer;
    common_cmd->code = BA_PWM_FREQ_CMD;
    freq_cmd = (struct s_base_pwm_freq *)(cmd_buffer + sizeof(struct s_com));
    freq_cmd->cmd = change;
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

