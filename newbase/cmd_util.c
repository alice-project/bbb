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

int format_gc_command()
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


int format_light_command(unsigned int cmds)
{
    struct s_com *common_cmd;
    struct s_base_light *light_cmd;

    BASE_LOG("light Command is sending ...");

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

